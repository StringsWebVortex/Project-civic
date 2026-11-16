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

# Optional USB-FS CDC image support. The default firmware image leaves these
# extensions empty and remains independent of the USB device stack.
EXTRA_SRCS ?=
EXTRA_FLAGS ?=
NET ?= 1
ifeq ($(NET),1)
NET_LINK_SRCS = Core/Src/net_usb.c Core/Src/web_server.c Core/Src/dash.c Core/Src/dash_store.c \
  Core/Src/flash_store.c Core/Src/gvret.c Core/Src/can_dbc.c build/net/dash_page.c build/can_dbc_table.c
NET_LINK_OBJS = $(NET_OBJS)
NET_FLAGS = -INet -IMiddlewares/Third_Party/tinyusb/src -IMiddlewares/Third_Party/tinyusb/lib/networking -IMiddlewares/Third_Party/lwip/src/include -IMiddlewares/Third_Party/jsmn -DNET_DASHBOARD=1
endif
ALL_FLAGS = $(ARM_FLAGS) $(NET_FLAGS) $(EXTRA_FLAGS)

USB_DIR = USB
USB_MW = Middlewares/ST/STM32_USB_Device_Library
USB_SRCS = \
  $(USB_DIR)/usbd_conf.c $(USB_DIR)/usbd_desc.c $(USB_DIR)/usbd_cdc_if.c \
  $(USB_DIR)/usb_gvret.c $(USB_DIR)/usb_tx_queue.c \
  Core/Src/gvret.c Core/Src/gvret_bridge.c \
  $(USB_MW)/Core/Src/usbd_core.c $(USB_MW)/Core/Src/usbd_ctlreq.c \
  $(USB_MW)/Core/Src/usbd_ioreq.c $(USB_MW)/Class/CDC/Src/usbd_cdc.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c \
  Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
USB_HEADERS = \
  $(wildcard $(USB_DIR)/*.h $(USB_MW)/Core/Inc/*.h \
    $(USB_MW)/Class/CDC/Inc/*.h) \
  Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h \
  Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_usb.h
USB_FLAGS = -DUSB_CDC_ENABLED -DUSE_HAL_DRIVER -Wno-error \
  -I$(USB_DIR) -I$(USB_MW)/Core/Inc -I$(USB_MW)/Class/CDC/Inc

# ── Single-board dashboard (ESP32 duties moved to the STM32) ──────────────
# OTG-FS CDC-NCM network adapter (TinyUSB) + lwIP raw TCP: HTTP 80, WS 81,
# GVRET TCP 23, observer TCP 2323, DHCP/DNS for the USB host.
TUSB = Middlewares/Third_Party/tinyusb
LWIP = Middlewares/Third_Party/lwip
NET_SRCS = \
  Core/Src/net_usb.c Net/usb_descriptors.c \
  $(TUSB)/src/tusb.c $(TUSB)/src/common/tusb_fifo.c \
  $(TUSB)/src/device/usbd.c $(TUSB)/src/device/usbd_control.c \
  $(TUSB)/src/class/net/ncm_device.c \
  $(TUSB)/src/portable/synopsys/dwc2/dcd_dwc2.c \
  $(wildcard $(TUSB)/src/portable/synopsys/dwc2/dwc2_common.c) \
  $(TUSB)/lib/networking/dhserver.c $(TUSB)/lib/networking/dnserver.c \
  $(LWIP)/src/core/def.c $(LWIP)/src/core/inet_chksum.c $(LWIP)/src/core/init.c \
  $(LWIP)/src/core/ip.c $(LWIP)/src/core/mem.c $(LWIP)/src/core/memp.c \
  $(LWIP)/src/core/netif.c $(LWIP)/src/core/pbuf.c $(LWIP)/src/core/tcp.c \
  $(LWIP)/src/core/tcp_in.c $(LWIP)/src/core/tcp_out.c $(LWIP)/src/core/timeouts.c \
  $(LWIP)/src/core/udp.c $(LWIP)/src/core/sys.c $(LWIP)/src/core/stats.c \
  $(LWIP)/src/core/ipv4/etharp.c $(LWIP)/src/core/ipv4/icmp.c \
  $(LWIP)/src/core/ipv4/ip4.c $(LWIP)/src/core/ipv4/ip4_addr.c \
  $(LWIP)/src/core/ipv4/ip4_frag.c $(LWIP)/src/core/ipv4/acd.c \
  $(LWIP)/src/netif/ethernet.c
NET_INC = -INet -I$(TUSB)/src -I$(TUSB)/lib/networking -I$(LWIP)/src/include
# Third-party code is compiled without -Werror (warnings are theirs).
NET_CFLAGS = $(filter-out -Werror -Wextra,$(ARM_FLAGS)) $(NET_INC) -DNET_DASHBOARD=1
NET_OBJS = $(patsubst %.c,build/net/%.o,$(filter-out Core/Src/net_usb.c,$(NET_SRCS)))

build/net/dash_page.c: web/index.html tools/gen_dash_page.py
	@mkdir -p $(dir $@)
	$(PYTHON) tools/gen_dash_page.py $< $@

build/net/%.o: %.c Net/tusb_config.h Net/lwipopts.h
	@mkdir -p $(dir $@)
	$(ARM_GCC) $(NET_CFLAGS) -w -c $< -o $@

# All firmware source files
FIRMWARE_SRCS = \
  Core/Src/main.c \
  Core/Src/can_frame_queue.c \
  Core/Src/can_frame_format.c \
  Core/Src/can_signal_profiler.c \
  Core/Src/obd2_decoder.c \
  Core/Src/system_stm32f4xx.c \
  Core/Src/periph_timing.c \
  Core/Src/isotp.c \
  Core/Src/uds_client.c \
  Core/Src/ecu_scan.c \
  Core/Src/can_hw.c \
  Core/Src/can_filter.c \
  Core/Src/can_opts.c \
  Core/Src/device_id.c \
  Core/Src/remote_start.c \
  Core/Src/gear_led.c \
  startup_stm32f407xx.s

.PHONY: lint test-host test-sanitize test-build test-esp32 results result firmware firmware-tx firmware-rx firmware-usb image clean FORCE

# Each image has its own output path, so building one never overwrites the
# other (the old single build/project_civic.bin could silently become the
# receive-only image, which has no TX/DTC/reset code).
IMG ?= build/project_civic

# ── Full image (deliverable): boots silent; TX only inside ARM window ──
firmware:
	@$(MAKE) --no-print-directory image IMG=build/project_civic \
	  TRANSCEIVER_SILENT_CONFIRMED=1 CAN_TX_CONFIRMED=1

firmware-tx: firmware
firmware-rx: firmware   # alias kept; the single full image is the only deliverable now

firmware-usb:
	@$(MAKE) --no-print-directory image IMG=build/usb/project_civic_usb \
	  TRANSCEIVER_SILENT_CONFIRMED=1 CAN_TX_CONFIRMED=1 NET=0 \
	  EXTRA_SRCS="$(USB_SRCS)" EXTRA_FLAGS="$(USB_FLAGS)"

image: $(IMG).elf $(IMG).bin
	$(ARM_SIZE) $(IMG).elf

# Rebuild whenever the effective flags change (prevents stale images)
$(IMG).flags: FORCE
	@mkdir -p $(dir $@)
	@echo '$(ALL_FLAGS)' | cmp -s - $@ || echo '$(ALL_FLAGS)' > $@

FORCE:

$(IMG).elf: STM32F407VGTX_FLASH.ld $(FIRMWARE_SRCS) $(EXTRA_SRCS) $(NET_LINK_SRCS) $(NET_LINK_OBJS) $(wildcard Core/Inc/*.h) $(USB_HEADERS) $(IMG).flags
	mkdir -p $(dir $@)
	$(ARM_GCC) $(ALL_FLAGS) -g --specs=nosys.specs \
	  -TSTM32F407VGTX_FLASH.ld \
	  -Wl,--gc-sections \
	  -Wl,-Map=$(IMG).map \
	  $(FIRMWARE_SRCS) $(EXTRA_SRCS) $(NET_LINK_SRCS) $(NET_LINK_OBJS) -o $@

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

build/test_gvret: tests/test_gvret.c Core/Src/gvret.c Core/Inc/gvret.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_gvret.c Core/Src/gvret.c -o $@

build/test_gvret_bridge: tests/test_gvret_bridge.c Core/Src/gvret_bridge.c Core/Src/gvret.c Core/Inc/gvret_bridge.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_gvret_bridge.c Core/Src/gvret_bridge.c Core/Src/gvret.c -o $@

build/test_gvret_bridge_san: tests/test_gvret_bridge.c Core/Src/gvret_bridge.c Core/Src/gvret.c Core/Inc/gvret_bridge.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_gvret_bridge.c Core/Src/gvret_bridge.c Core/Src/gvret.c -o $@

build/test_usb_tx_queue: tests/test_usb_tx_queue.c USB/usb_tx_queue.c USB/usb_tx_queue.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -IUSB tests/test_usb_tx_queue.c USB/usb_tx_queue.c -o $@

build/test_usb_tx_queue_san: tests/test_usb_tx_queue.c USB/usb_tx_queue.c USB/usb_tx_queue.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -IUSB tests/test_usb_tx_queue.c USB/usb_tx_queue.c -o $@

build/test_remote_start: tests/test_remote_start.c Core/Src/remote_start.c Core/Inc/remote_start.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_remote_start.c Core/Src/remote_start.c -o $@

build/test_dash_store: tests/test_dash_store.c tests/fake_flash.c Core/Src/dash_store.c Core/Inc/dash_store.h Core/Inc/flash_store.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -DFS_HOST_TEST -ICore/Inc tests/test_dash_store.c tests/fake_flash.c Core/Src/dash_store.c Core/Src/can_frame_queue.c -o $@

build/test_dash_store_san: tests/test_dash_store.c tests/fake_flash.c Core/Src/dash_store.c Core/Inc/dash_store.h Core/Inc/flash_store.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g -DFS_HOST_TEST \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_dash_store.c tests/fake_flash.c Core/Src/dash_store.c Core/Src/can_frame_queue.c -o $@

build/test_dash_bridge: tests/test_dash_bridge.c tests/fake_flash.c Core/Src/dash.c Core/Src/dash_store.c Core/Src/gvret.c Core/Inc/dash.h Core/Inc/web_server.h Core/Src/can_dbc.c build/can_dbc_table.c
	mkdir -p build
	$(HOST_CC) -std=gnu11 -Wall -Wextra -Werror -O2 -DFS_HOST_TEST -Itests/host_shim -ICore/Inc -IMiddlewares/Third_Party/jsmn tests/test_dash_bridge.c tests/fake_flash.c Core/Src/dash.c Core/Src/dash_store.c Core/Src/device_id.c Core/Src/gvret.c Core/Src/can_frame_queue.c Core/Src/can_dbc.c build/can_dbc_table.c -o $@

build/test_dash_bridge_san: tests/test_dash_bridge.c tests/fake_flash.c Core/Src/dash.c Core/Src/dash_store.c Core/Src/gvret.c Core/Inc/dash.h Core/Inc/web_server.h Core/Src/can_dbc.c build/can_dbc_table.c
	mkdir -p build
	$(HOST_CC) -std=gnu11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -DFS_HOST_TEST -Itests/host_shim -ICore/Inc -IMiddlewares/Third_Party/jsmn tests/test_dash_bridge.c tests/fake_flash.c Core/Src/dash.c Core/Src/dash_store.c Core/Src/device_id.c Core/Src/gvret.c Core/Src/can_frame_queue.c Core/Src/can_dbc.c build/can_dbc_table.c -o $@

build/test_web_server: tests/test_web_server.c Core/Src/web_server.c Core/Inc/web_server.h
	mkdir -p build
	$(HOST_CC) -std=gnu11 -Wall -Wextra -Werror -O2 -Itests/host_shim -ICore/Inc tests/test_web_server.c -o $@

build/test_web_server_san: tests/test_web_server.c Core/Src/web_server.c Core/Inc/web_server.h
	mkdir -p build
	$(HOST_CC) -std=gnu11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -Itests/host_shim -ICore/Inc tests/test_web_server.c -o $@

build/can_dbc_table.c: dbc/acura_ilx_2016_can.dbc $(wildcard dbc/_*.dbc) tools/dbc_to_c.py
	@mkdir -p build
	$(PYTHON) tools/dbc_to_c.py dbc/acura_ilx_2016_can.dbc $@

build/test_can_dbc: tests/test_can_dbc.c Core/Src/can_dbc.c Core/Inc/can_dbc.h build/can_dbc_table.c
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_can_dbc.c Core/Src/can_dbc.c build/can_dbc_table.c -lm -o $@

build/test_can_dbc_san: tests/test_can_dbc.c Core/Src/can_dbc.c Core/Inc/can_dbc.h build/can_dbc_table.c
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_can_dbc.c Core/Src/can_dbc.c build/can_dbc_table.c -lm -o $@

build/test_gear_led: tests/test_gear_led.c Core/Src/gear_led.c Core/Inc/gear_led.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_gear_led.c Core/Src/gear_led.c -o $@

build/test_gear_led_san: tests/test_gear_led.c Core/Src/gear_led.c Core/Inc/gear_led.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_gear_led.c Core/Src/gear_led.c -o $@

build/test_can_filter: tests/test_can_filter.c Core/Src/can_filter.c Core/Inc/can_filter.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_can_filter.c Core/Src/can_filter.c -o $@

build/test_can_opts: tests/test_can_opts.c Core/Src/can_opts.c Core/Inc/can_opts.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_can_opts.c Core/Src/can_opts.c -o $@

build/test_can_opts_san: tests/test_can_opts.c Core/Src/can_opts.c Core/Inc/can_opts.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_can_opts.c Core/Src/can_opts.c -o $@

build/test_device_id: tests/test_device_id.c Core/Src/device_id.c Core/Inc/device_id.h
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -ICore/Inc tests/test_device_id.c Core/Src/device_id.c -o $@

build/test_device_id_san: tests/test_device_id.c Core/Src/device_id.c Core/Inc/device_id.h
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -pedantic -O1 -g \
	  -fsanitize=address,undefined -fno-omit-frame-pointer \
	  -ICore/Inc tests/test_device_id.c Core/Src/device_id.c -o $@

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
PYTHON ?= python3
RECORD = $(PYTHON) tests/testresult.py run
C_TESTS     = test_periph_timing test_can_frame_queue test_obd2_decoder test_can_signal_profiler test_isotp_uds test_ecu_scan test_gvret test_gvret_bridge test_usb_tx_queue test_remote_start test_can_filter test_can_opts test_device_id test_gear_led test_dash_store test_dash_bridge test_web_server test_can_dbc
SAN_TESTS   = test_can_opts_san test_device_id_san test_can_frame_queue_san test_obd2_decoder_san test_can_signal_profiler_san test_isotp_uds_san test_ecu_scan_san test_gvret_bridge_san test_usb_tx_queue_san test_gear_led_san test_dash_store_san test_dash_bridge_san test_web_server_san test_can_dbc_san

test-host:
	@for t in $(C_TESTS); do \
	  $(RECORD) $$t --timeout 30 --build "$(MAKE) -s build/$$t" -- ./build/$$t || true; \
	done
	@$(PYTHON) tests/testresult.py pytests --timeout 60 || true
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
	@$(RECORD) image_has_full_code --timeout 60 -- sh -c 'grep -q "NOT_ARMED" build/project_civic.bin && grep -q "START,CRANK" build/project_civic.bin && grep -q "STATUS,SYS,GATEWAY" build/project_civic.bin && grep -q "GEAR,ACK" build/project_civic.bin || { echo "build/project_civic.bin is missing full TX/diagnostic/remote-start/gear-LED code"; exit 1; }' || true
	@$(RECORD) image_fits_app_sectors --timeout 60 -- sh -c 's=$$(stat -c %s build/project_civic.bin); test $$s -gt 4096 -a $$s -le 262144 || { echo "build/project_civic.bin is $$s bytes; must be 4 KB..256 KB (sectors 0-5)"; exit 1; }' || true
	@$(RECORD) image_has_dashboard --timeout 60 -- sh -c 'grep -a -q "Civic CAN Monitor" build/project_civic.bin && grep -a -q "event snapshot not ready" build/project_civic.bin && grep -a -q "Sec-WebSocket-Accept" build/project_civic.bin || { echo "build/project_civic.bin is missing the single-board dashboard (USB network, HTTP/WS, journal)"; exit 1; }' || true
	@$(RECORD) build_usb_firmware --timeout 300 -- $(MAKE) -s firmware-usb ARM_GCC=$(ARM_GCC) ARM_SIZE=$(ARM_SIZE) ARM_OBJCOPY=$(ARM_OBJCOPY) || true
	@$(RECORD) image_has_usb_cdc --timeout 60 -- sh -c 'test -s build/usb/project_civic_usb.bin && grep -a -q "Civic CAN GVRET (CDC)" build/usb/project_civic_usb.bin || { echo "USB firmware image is missing CDC descriptor code"; exit 1; }' || true
	@echo "test-build finished: run 'make results'"

# ESP32 dashboard firmware built as a recorded test (needs PlatformIO).
PIO ?= $(shell command -v pio 2>/dev/null || printf '%s/.platformio/penv/bin/pio' "$(HOME)")
test-esp32:
	@$(RECORD) build_esp32_dashboard --timeout 600 -- $(PIO) run -d esp32-dashboard || true
	@echo "test-esp32 finished: run 'make results'"

# Static analysis (GCC -fanalyzer + extra warnings) over project sources,
# recorded as lint_firmware. Any warning fails the test (-Werror).
LINT_SRCS = Core/Src/can_dbc.c $(filter Core/Src/%.c,$(filter-out Core/Src/system_stm32f4xx.c,$(FIRMWARE_SRCS))) Core/Src/gvret_bridge.c USB/usb_tx_queue.c \
  Core/Src/net_usb.c Core/Src/web_server.c Core/Src/dash.c Core/Src/dash_store.c Core/Src/flash_store.c
LINT_FLAGS = $(ARM_FLAGS) -fanalyzer -Wshadow -Wconversion -Wno-sign-conversion
LINT_FLAGS += -IUSB $(NET_FLAGS)
lint: TRANSCEIVER_SILENT_CONFIRMED = 1
lint: CAN_TX_CONFIRMED = 1
lint:
	@$(RECORD) lint_firmware --timeout 600 -- sh -c 'for f in $(LINT_SRCS); do $(ARM_GCC) $(LINT_FLAGS) -c $$f -o /dev/null || exit 1; done' || true
	@echo "lint finished: run 'make result T=lint_firmware'"

results:
	@$(PYTHON) tests/testresult.py latest

result:
	@$(PYTHON) tests/testresult.py last $(T)

clean:
	rm -rf build
