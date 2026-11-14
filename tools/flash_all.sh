#!/usr/bin/env bash
# flash_all.sh: gate, detect, back up, flash, verify and restore-on-fail for
# both boards of Project-Civic:
#   STM32F407VGT6 Discovery (MB997E, on-board ST-LINK/V2-A) via stlink-tools
#   ESP-32S NodeMCU v1.1 (USB-serial) via PlatformIO's esptool.py
#
# Usage:     bash /home/s752/Project-civic/tools/flash_all.sh
# Optional:  --esp-port /dev/ttyACM1   (otherwise auto-detected)
#
# Result: one tagged line per run in firmware/tests/results/flash_all.result
# ("100%" or the exact failure reason + last 3 log lines).
# Full log of the run: firmware/build/flash/flash_<time>.log
#
# Addresses and offsets come from the tools themselves, not assumptions:
#   STM32 flash base 0x08000000, 1 MiB: RM0090 memory map; checked live with
#     st-info --probe (chipid 0x413, flash 1048576).
#   ESP32 0x1000 bootloader, 0x8000 partitions, 0xe000 boot_app0, 0x10000 app:
#     ~/.platformio/packages/framework-arduinoespressif32/tools/platformio-build.py
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FW="$ROOT/firmware"
PIO_HOME="${PLATFORMIO_CORE_DIR:-$HOME/.platformio}"
PIO="$PIO_HOME/penv/bin/pio"
ESPTOOL=(python3 "$PIO_HOME/packages/tool-esptoolpy/esptool.py" --chip esp32)
BOOT_APP0="$PIO_HOME/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
ESPB="$FW/esp32-dashboard/.pio/build/esp32dev"
STM_BIN="$FW/build/project_civic.bin"
STM_BASE=0x08000000
STM_SIZE=1048576           # 1 MiB, STM32F407VGT6
ESP_SIZE=4194304           # 4 MiB, esp32dev board definition
ESP_BAUD=460800

TS="$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p "$FW/build/flash" "$ROOT/backups"
LOG="$FW/build/flash/flash_$TS.log"
STM_BACKUP="$ROOT/backups/stm32f407-before-flash-$TS.bin"
ESP_BACKUP="$ROOT/backups/esp32-before-flash-$TS.bin"
STM_READBACK="$FW/build/flash/stm32-readback-$TS.bin"
ESP_PORT=""
STM_TOUCHED=0
ESP_TOUCHED=0

while [ $# -gt 0 ]; do
  case "$1" in
    --esp-port) ESP_PORT="${2:-}"; shift 2 ;;
    *) echo "unknown option: $1"; exit 2 ;;
  esac
done

exec > >(exec tee -i -a "$LOG") 2>&1   # tee ignores Ctrl-C so fail() can restore

step() { printf '\n==== %s ====\n' "$*"; }

record() {
  (cd "$FW" && python3 -c 'import sys; sys.path.insert(0, "tests"); import testresult; testresult.record("flash_all", sys.argv[1])' "$1")
}

esp() { "${ESPTOOL[@]}" --port "$ESP_PORT" --baud "$ESP_BAUD" "$@"; }

restore_stm() {
  [ "$STM_TOUCHED" = 1 ] || return 0
  [ -s "$STM_BACKUP" ] || { echo "RESTORE STM32: no backup, cannot restore"; return 1; }
  step "RESTORE STM32 from $STM_BACKUP"
  st-flash write "$STM_BACKUP" "$STM_BASE" \
    && st-flash read "$STM_READBACK" "$STM_BASE" "$STM_SIZE" \
    && cmp -s "$STM_BACKUP" "$STM_READBACK" \
    && st-flash reset \
    && echo "RESTORE STM32: verified OK" \
    || { echo "RESTORE STM32: FAILED"; return 1; }
}

restore_esp() {
  [ "$ESP_TOUCHED" = 1 ] || return 0
  [ -s "$ESP_BACKUP" ] || { echo "RESTORE ESP32: no backup, cannot restore"; return 1; }
  step "RESTORE ESP32 from $ESP_BACKUP"
  esp --after no_reset write_flash --flash_mode keep --flash_freq keep --flash_size keep 0x0 "$ESP_BACKUP" \
    && esp --after hard_reset verify_flash 0x0 "$ESP_BACKUP" \
    && echo "RESTORE ESP32: verified OK" \
    || { echo "RESTORE ESP32: FAILED"; return 1; }
}

fail() {
  trap - INT TERM
  local reason="$1" rs="restore not needed"
  echo "FAILED: $reason"
  if [ "$STM_TOUCHED" = 1 ] || [ "$ESP_TOUCHED" = 1 ]; then
    # Restore both boards so the STM32/ESP32 pair stays a matching set.
    rs="restored"
    restore_stm || rs="STM32 RESTORE FAILED"
    restore_esp || rs="$rs; ESP32 RESTORE FAILED"
  fi
  sleep 1
  record "FAILED: $reason ($rs) | $(tail -n 3 "$LOG" | tr '\n' ' ' | tr -s ' ')"
  echo "Result recorded in firmware/tests/results/flash_all.result; full log: $LOG"
  exit 1
}
trap 'fail "interrupted by user/signal"' INT TERM

need() { command -v "$1" >/dev/null 2>&1 || fail "required tool not found: $1"; }

# ── 1. Tools ────────────────────────────────────────────────────────────
step "1/7 Tools"
need st-info; need st-flash; need python3; need make; need cmp
[ -x "$PIO" ] || fail "PlatformIO not found at $PIO"
[ -f "${ESPTOOL[1]}" ] || fail "esptool.py not found at ${ESPTOOL[1]}"
[ -s "$BOOT_APP0" ] || fail "boot_app0.bin not found at $BOOT_APP0"
echo "tools OK"

# ── 2. Gate: build + every recorded test must be 100% in THIS run ──────
step "2/7 Gate: lint, host tests, sanitizer tests, STM32 + ESP32 builds"
GATE_START="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
(cd "$FW" && make -s lint test-host test-sanitize test-build && make -s test-esp32 PIO="$PIO") >/dev/null 2>&1
REQUIRED="lint_firmware build_firmware build_firmware_rx image_has_tx_code build_esp32_dashboard
 test_periph_timing test_can_frame_queue test_obd2_decoder test_can_signal_profiler test_isotp_uds test_ecu_scan
 test_can_frame_queue_san test_obd2_decoder_san test_can_signal_profiler_san test_isotp_uds_san test_ecu_scan_san"
for t in $REQUIRED; do
  line="$(cd "$FW" && make -s result T="$t")"
  ts="$(printf '%s' "$line" | awk '{print $2}')"
  [[ ! "$ts" < "$GATE_START" ]] || fail "gate: $t did not record a result in this run"
  case "$line" in *"result=100%"*) ;; *) fail "gate: $t not 100% (make result T=$t)";; esac
done
while IFS= read -r line; do
  ts="$(printf '%s' "$line" | awk '{print $2}')"
  [[ "$ts" < "$GATE_START" ]] && continue
  case "$line" in *"result=100%"*) ;; *) fail "gate: ${line%%:*} not 100%";; esac
done < <(cd "$FW" && make -s results)
echo "gate OK: every test of this run is 100%"

# ── 3. Images ───────────────────────────────────────────────────────────
step "3/7 Images (path, size, sha256)"
for f in "$STM_BIN" "$ESPB/bootloader.bin" "$ESPB/partitions.bin" "$BOOT_APP0" "$ESPB/firmware.bin"; do
  [ -s "$f" ] || fail "image missing: $f"
  printf '%9s  %s  %s\n' "$(stat -c %s "$f")" "$(sha256sum "$f" | cut -d' ' -f1)" "$f"
done
[ "$(stat -c %s "$STM_BIN")" -lt "$STM_SIZE" ] || fail "STM32 image larger than 1 MiB flash"

# ── 4. Detect boards ────────────────────────────────────────────────────
step "4/7 Detect STM32 (ST-LINK)"
PROBE="$(st-info --probe 2>&1)"; echo "$PROBE"
[ "$(printf '%s\n' "$PROBE" | grep -c 'serial:')" = 1 ] || fail "need exactly one ST-LINK connected"
printf '%s' "$PROBE" | grep -Eq 'chipid:[[:space:]]+0x0?413' || fail "target is not STM32F40x/41x (chipid 0x413)"
printf '%s' "$PROBE" | grep -Eq 'flash:[[:space:]]+1048576' || fail "target flash is not 1 MiB (STM32F407VGT6)"

step "4/7 Detect ESP32 (USB serial)"
if [ -z "$ESP_PORT" ]; then
  for c in /dev/serial/by-id/*; do
    [ -e "$c" ] || continue
    case "$c" in *STLink*|*STLINK*|*ST-LINK*) continue ;; esac
    p="$(readlink -f "$c")"
    if out="$("${ESPTOOL[@]}" --port "$p" --baud 115200 flash_id 2>&1)" && printf '%s' "$out" | grep -q 'Chip is ESP32'; then
      ESP_PORT="$p"; break
    fi
  done
fi
[ -n "$ESP_PORT" ] || fail "no ESP32 found on any USB serial port (use --esp-port)"
ID="$(esp flash_id 2>&1)"; echo "$ID" | grep -E 'Chip is|MAC:|flash size'
printf '%s' "$ID" | grep -q 'Chip is ESP32' || fail "device on $ESP_PORT is not an ESP32"
printf '%s' "$ID" | grep -q 'Detected flash size: 4MB' || fail "ESP32 flash is not 4MB"
echo "ESP32 on $ESP_PORT"

# ── 5. Read-back backups (whole flash of both boards) ──────────────────
step "5/7 Backup STM32 1 MiB -> $STM_BACKUP"
st-flash read "$STM_BACKUP" "$STM_BASE" "$STM_SIZE" || fail "STM32 backup read failed"
[ "$(stat -c %s "$STM_BACKUP")" = "$STM_SIZE" ] || fail "STM32 backup size wrong"
step "5/7 Backup ESP32 4 MiB -> $ESP_BACKUP (about 2 minutes)"
esp --after no_reset read_flash 0x0 "$ESP_SIZE" "$ESP_BACKUP" || fail "ESP32 backup read failed"
[ "$(stat -c %s "$ESP_BACKUP")" = "$ESP_SIZE" ] || fail "ESP32 backup size wrong"
sha256sum "$STM_BACKUP" "$ESP_BACKUP"

# ── 6. Flash + verify STM32 ────────────────────────────────────────────
step "6/7 Flash STM32 $STM_BIN @ $STM_BASE"
STM_TOUCHED=1
st-flash write "$STM_BIN" "$STM_BASE" || fail "STM32 write failed"
st-flash read "$STM_READBACK" "$STM_BASE" "$(stat -c %s "$STM_BIN")" || fail "STM32 read-back failed"
cmp "$STM_BIN" "$STM_READBACK" || fail "STM32 verify mismatch (read-back differs from image)"
st-flash reset || fail "STM32 reset failed"
echo "STM32 verified: read-back equals image"

# ── 7. Flash + verify ESP32 ────────────────────────────────────────────
step "7/7 Flash ESP32 on $ESP_PORT"
ESP_IMAGES=(0x1000 "$ESPB/bootloader.bin" 0x8000 "$ESPB/partitions.bin" 0xe000 "$BOOT_APP0" 0x10000 "$ESPB/firmware.bin")
ESP_TOUCHED=1
esp --after no_reset write_flash --flash_mode keep --flash_freq keep --flash_size keep "${ESP_IMAGES[@]}" \
  || fail "ESP32 write failed"
esp --after hard_reset verify_flash "${ESP_IMAGES[@]}" || fail "ESP32 verify mismatch"
echo "ESP32 verified: flash equals images"

trap - INT TERM
record "100%"
step "DONE: both boards flashed and verified"
echo "Backups: $STM_BACKUP"
echo "         $ESP_BACKUP"
echo "Result:  $(cd "$FW" && make -s result T=flash_all)"
echo "Log:     $LOG"
