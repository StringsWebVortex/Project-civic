HOST_CC      ?= cc
HOST_CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -pedantic -O2
ARM_GCC      ?= /home/s752/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-gcc
ARM_SIZE     ?= /home/s752/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-size
ARM_OBJCOPY  ?= /home/s752/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-objcopy
CMSIS_INC     = Drivers/CMSIS
HAL_INC       = Drivers/STM32F4xx_HAL_Driver/Inc

# ── Transceiver: TJA1051 (NXP) — ONLY supported CAN transceiver. ───────
# No substitutes, variants, or successors are permitted.
# TJA1051 pins (NXP TJA1051 Rev 9 Table 3): 1 TXD, 2 GND, 3 VCC (5 V),
# 4 RXD, 5 n.c., 6 CANL, 7 CANH, 8 S (silent mode). There is no STB pin.
# `make firmware`    : full image = the deliverable, build/project_civic.bin.
#                      Boots in bxCAN silent mode; CAN TX, DTC read/clear, ECU
#                      reset and live writes run only inside an ARM,<s> window,
#                      and every non-read service also needs CONFIRM.
# `make firmware-tx` : same as `make firmware` (kept as an alias).
# `make firmware-rx` : receive-only bench image, no TX code compiled,
#                      build/rx/project_civic_rx.bin (never the deliverable).
TRANSCEIVER_SILENT_CONFIRMED ?= 0
CAN_TX_CONFIRMED             ?= 0
HOST_UART_BAUD               ?= 2000000

ARM_FLAGS = \
  -mcpu=cortex-m4 -mthumb \
  -DSTM32F407xx \
  -DHSE_VALUE=8000000U \
  -DPROJECT_TRANSCEIVER_SILENT_CONFIRMED=$(TRANSCEIVER_SILENT_CONFIRMED)u \
  -DPROJECT_TRANSCEIVER_TJA1051=1u \
  -DPROJECT_CAN_TX_CONFIRMED=$(CAN_TX_CONFIRMED)u \
  -DPROJECT_HOST_UART_BAUD=$(HOST_UART_BAUD)u \
  -ffunction-sections -fdata-sections \
  -Wall -Wextra -Werror -Os \
  -ICore/Inc \
  -I$(CMSIS_INC) \
  -I$(HAL_INC)

# All firmware source files
FIRMWARE_SRCS = \
  Core/Src/main.c \
  Core/Src/can_frame_queue.c \
  Core/Src/can_frame_format.c \
  Core/Src/can_signal_profiler.c \
  Core/Src/obd2_decoder.c \
  Core/Src/system_stm32f4xx.c \
  Core/Src/uart_dma.c \
  Core/Src/periph_timing.c \
  Core/Src/isotp.c \
  Core/Src/uds_client.c \
  Core/Src/ecu_scan.c \
  startup_stm32f407xx.s

.PHONY: lint test-host test-sanitize test-build test-esp32 results result firmware firmware-tx firmware-rx image clean FORCE

# Each image has its own output path, so building one never overwrites the
# other (the old single build/project_civic.bin could silently become the
# receive-only image, which has no TX/DTC/reset code).
IMG ?= build/project_civic

# ── Full image (deliverable): boots silent; TX only inside ARM window ──
firmware:
	@$(MAKE) --no-print-directory image IMG=build/project_civic \
	  TRANSCEIVER_SILENT_CONFIRMED=1 CAN_TX_CONFIRMED=1

firmware-tx: firmware

# ── Receive-only bench image: TX code is not compiled in ───────────────
firmware-rx:
	@$(MAKE) --no-print-directory image IMG=build/rx/project_civic_rx \
	  TRANSCEIVER_SILENT_CONFIRMED=1 CAN_TX_CONFIRMED=0

image: $(IMG).elf $(IMG).bin
	$(ARM_SIZE) $(IMG).elf

# Rebuild whenever the effective flags change (prevents stale images)
$(IMG).flags: FORCE
	@mkdir -p $(dir $@)
	@echo '$(ARM_FLAGS)' | cmp -s - $@ || echo '$(ARM_FLAGS)' > $@

FORCE:

$(IMG).elf: $(FIRMWARE_SRCS) $(wildcard Core/Inc/*.h) $(IMG).flags
	mkdir -p $(dir $@)
	$(ARM_GCC) $(ARM_FLAGS) -g --specs=nosys.specs \
	  -TSTM32F407VGTX_FLASH.ld \
	  -Wl,--gc-sections \
	  -Wl,-Map=$(IMG).map \
	  $(FIRMWARE_SRCS) -o $@

$(IMG).bin: $(IMG).elf
	$(ARM_OBJCOPY) -O binary $< $@
	@echo "Flat binary: $@"

# ── Host tests ──────────────────────────────────────────────────────────
TEST_SRCS_C = \
  Core/Src/can_frame_queue.c \
  Core/Src/can_frame_format.c \
  Core/Src/can_signal_profiler.c \
  Core/Src/obd2_decoder.c

build/test_can_frame_queue: tests/test_can_frame_queue.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc \
	  tests/test_can_frame_queue.c \
	  Core/Src/can_frame_queue.c Core/Src/can_frame_format.c -o $@

build/test_obd2_decoder: tests/test_obd2_decoder.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc \
	  tests/test_obd2_decoder.c \
	  Core/Src/obd2_decoder.c Core/Src/can_frame_queue.c -o $@

build/test_can_signal_profiler: tests/test_can_signal_profiler.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc \
	  tests/test_can_signal_profiler.c \
	  Core/Src/can_signal_profiler.c Core/Src/can_frame_queue.c -o $@

build/test_can_frame_queue_san: tests/test_can_frame_queue.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc \
	  tests/test_can_frame_queue.c \
	  Core/Src/can_frame_queue.c Core/Src/can_frame_format.c -o $@

build/test_obd2_decoder_san: tests/test_obd2_decoder.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc \
	  tests/test_obd2_decoder.c \
	  Core/Src/obd2_decoder.c Core/Src/can_frame_queue.c -o $@

build/test_can_signal_profiler_san: tests/test_can_signal_profiler.c $(TEST_SRCS_C)
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc \
	  tests/test_can_signal_profiler.c \
	  Core/Src/can_signal_profiler.c Core/Src/can_frame_queue.c -o $@

build/test_periph_timing: tests/test_periph_timing.c Core/Src/periph_timing.c Core/Inc/periph_timing.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_periph_timing.c Core/Src/periph_timing.c -o $@

ISOTP_SRCS = Core/Src/isotp.c Core/Src/uds_client.c
ISOTP_HDRS = Core/Inc/isotp.h Core/Inc/uds_client.h

build/test_isotp_uds: tests/test_isotp_uds.c $(ISOTP_SRCS) $(ISOTP_HDRS)
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_isotp_uds.c $(ISOTP_SRCS) -o $@

build/test_isotp_uds_san: tests/test_isotp_uds.c $(ISOTP_SRCS) $(ISOTP_HDRS)
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_isotp_uds.c $(ISOTP_SRCS) -o $@

build/test_ecu_scan: tests/test_ecu_scan.c Core/Src/ecu_scan.c Core/Inc/ecu_scan.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_ecu_scan.c Core/Src/ecu_scan.c -o $@

build/test_ecu_scan_san: tests/test_ecu_scan.c Core/Src/ecu_scan.c Core/Inc/ecu_scan.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_ecu_scan.c Core/Src/ecu_scan.c -o $@

# Tests: every test writes ONE line per run to tests/results/<name>.result
# ("100%" or the exact failure reason). No test output is printed.
# Read results with `make results` (newest line per test) or
# `make result T=<name>`. Never read the whole result file.
RECORD = python3 tests/testresult.py run
C_TESTS     = test_periph_timing test_can_frame_queue test_obd2_decoder test_can_signal_profiler test_isotp_uds test_ecu_scan
SAN_TESTS   = test_can_frame_queue_san test_obd2_decoder_san test_can_signal_profiler_san test_isotp_uds_san test_ecu_scan_san

test-host:
	@for t in $(C_TESTS); do \
	  $(RECORD) $$t --timeout 30 --build "$(MAKE) -s build/$$t" -- ./build/$$t || true; \
	done
	@python3 tests/testresult.py pytests --timeout 60 || true
	@echo "test-host finished: run 'make results'"

test-sanitize:
	@for t in $(SAN_TESTS); do \
	  $(RECORD) $$t --timeout 120 --build "$(MAKE) -s build/$$t" -- ./build/$$t || true; \
	done
	@echo "test-sanitize finished: run 'make results'"

# Firmware images built as recorded tests (build_firmware = full deliverable
# image, build_firmware_rx = receive-only bench image), plus a check that the
# deliverable really contains the TX/diagnostic code (image_has_tx_code).
test-build:
	@$(RECORD) build_firmware --timeout 300 -- $(MAKE) -s firmware ARM_GCC=$(ARM_GCC) ARM_SIZE=$(ARM_SIZE) ARM_OBJCOPY=$(ARM_OBJCOPY) || true
	@$(RECORD) build_firmware_rx --timeout 300 -- $(MAKE) -s firmware-rx ARM_GCC=$(ARM_GCC) ARM_SIZE=$(ARM_SIZE) ARM_OBJCOPY=$(ARM_OBJCOPY) || true
	@$(RECORD) image_has_tx_code --timeout 60 -- sh -c 'grep -q -- "-DPROJECT_CAN_TX_CONFIRMED=1u" build/project_civic.flags && grep -q "UDS,ERR,NOT_ARMED" build/project_civic.bin && ! grep -q "TX_NOT_BUILT" build/project_civic.bin || { echo "build/project_civic.bin is not the full TX/diagnostic image"; exit 1; }' || true
	@echo "test-build finished: run 'make results'"

# ESP32 dashboard firmware built as a recorded test (needs PlatformIO).
PIO ?= pio
test-esp32:
	@$(RECORD) build_esp32_dashboard --timeout 600 -- $(PIO) run -d esp32-dashboard || true
	@echo "test-esp32 finished: run 'make results'"

# Static analysis (GCC -fanalyzer + extra warnings) over project sources,
# recorded as lint_firmware. Any warning fails the test (-Werror).
LINT_SRCS = $(filter Core/Src/%.c,$(filter-out Core/Src/system_stm32f4xx.c,$(FIRMWARE_SRCS)))
LINT_FLAGS = $(ARM_FLAGS) -fanalyzer -Wshadow -Wconversion -Wno-sign-conversion
lint: TRANSCEIVER_SILENT_CONFIRMED = 1
lint: CAN_TX_CONFIRMED = 1
lint:
	@$(RECORD) lint_firmware --timeout 600 -- sh -c 'for f in $(LINT_SRCS); do $(ARM_GCC) $(LINT_FLAGS) -c $$f -o /dev/null || exit 1; done' || true
	@echo "lint finished: run 'make result T=lint_firmware'"

results:
	@python3 tests/testresult.py latest

result:
	@python3 tests/testresult.py last $(T)

clean:
	rm -rf build
