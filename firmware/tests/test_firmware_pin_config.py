"""Source-level regression tests for the supplied Project Civic pinout."""
from pathlib import Path
import gzip
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
PROJECT_ROOT = ROOT.parent
MAIN = (ROOT / "Core/Src/main.c").read_text()
CAN_HW = (ROOT / "Core/Src/can_hw.c").read_text()
CAN_H = (ROOT / "Core/Inc/can_hw.h").read_text()
STARTUP = (ROOT / "startup_stm32f407xx.s").read_text()


class HardwarePinConfigTests(unittest.TestCase):
    # Pins verified against DS8626 (FT, AF9) and UM1472 (free on MB997).
    def test_can_pins_and_nominal_scan_rates(self):
        self.assertRegex(MAIN, r"\.can\s*=\s*CAN1,\s*\.bus_index\s*=\s*0u,\s*\.gpio\s*=\s*GPIOD")
        self.assertRegex(MAIN, r"\.rx_pin\s*=\s*0u,\s*\.tx_pin\s*=\s*1u,\s*\.s_gpio\s*=\s*GPIOD,\s*\.s_pin\s*=\s*2u,\s*\.af\s*=\s*9u")
        self.assertRegex(MAIN, r"\.preferred_bitrate\s*=\s*500000u")
        self.assertRegex(MAIN, r"\.can\s*=\s*CAN2,\s*\.bus_index\s*=\s*1u,\s*\.gpio\s*=\s*GPIOB")
        self.assertRegex(MAIN, r"\.rx_pin\s*=\s*12u,\s*\.tx_pin\s*=\s*13u,\s*\.s_gpio\s*=\s*GPIOD,\s*\.s_pin\s*=\s*3u,\s*\.af\s*=\s*9u")
        self.assertRegex(MAIN, r"\.preferred_bitrate\s*=\s*125000u")
        self.assertIn("c->cfg->preferred_bitrate", CAN_HW)
        for r in ("500000u", "125000u", "33333u"):
            self.assertIn(r, CAN_HW)

    def test_can_tx_and_s_are_open_drain_for_5v_tja1051(self):
        # TJA1051T VIO=VCC=5 V -> VIH 3.5 V: OD + 5 V pull-up on FT pins.
        self.assertIn("if (open_drain) g->OTYPER |= (1u << pin);", CAN_HW)
        self.assertIn("pin_af(cfg->gpio, cfg->tx_pin, cfg->af, true);", CAN_HW)
        self.assertIn("pin_af(cfg->gpio, cfg->rx_pin, cfg->af, false);", CAN_HW)
        self.assertIn("cfg->s_gpio->OTYPER |= (1u << cfg->s_pin);", CAN_HW)

    def test_tja1051_s_pin_silent_except_normal(self):
        self.assertIn("transceiver_follow_mode", CAN_HW)
        self.assertIn("(m == CAN_MODE_NORMAL) ? (1u << (cfg->s_pin + 16u))", CAN_HW)
        self.assertIn("S released = HIGH = silent", CAN_HW)

    def test_each_can_instance_has_its_own_rx_queue(self):
        self.assertIn("capture_queue[2]", MAIN)
        self.assertIn("&capture_queue[0]", MAIN)
        self.assertIn("&capture_queue[1]", MAIN)
        self.assertIn("capture_queue[c->cfg->bus_index].dropped", MAIN)
        self.assertGreaterEqual(MAIN.count("can_capture_queue_pop(&capture_queue[b]"), 1)

    def test_usart2_esp32_uart_link_is_configured(self):
        uart = (ROOT / "Core/Src/uart_esp.c").read_text()
        mk = (ROOT / "Makefile").read_text()
        self.assertIn("USART2", uart)
        self.assertIn("DMA1_Stream6", uart)
        self.assertIn("PROJECT_HOST_UART_BAUD", uart)
        self.assertIn("GPIOA->AFR[0]", uart)
        self.assertIn("(7u << (4u * 2u))", uart)
        self.assertIn("(7u << (4u * 3u))", uart)
        self.assertIn("uart_esp_rx_line()", MAIN)
        self.assertIn("Core/Src/uart_esp.c", mk)
        self.assertIn("Core/Src/uart_tx_queue.c", mk)
        firmware_sources = mk.split("FIRMWARE_SRCS =", 1)[1].split("$(IMG).elf", 1)[0]
        self.assertNotIn("Core/Src/uart_dma.c", firmware_sources)
        for stale in ("USART3", "host_rx_pop", "PB10", "GPIOB->AFR[1]"):
            self.assertNotIn(stale, MAIN + uart)

    def test_esp32_websocket_bridge_uses_stable_client_slots_and_one_debug_route(self):
        esp = (PROJECT_ROOT / "esp32_bridge/src/main.cpp").read_text()
        self.assertIn("static const long  UART_BAUD    = 2000000L;", esp)
        self.assertIn("static AsyncWebServer  ws_server(81);", esp)
        self.assertIn('static AsyncWebSocket  ws_proxy("/")', esp)
        self.assertIn('http_server.on("/token", HTTP_GET', esp)
        self.assertIn('http_server.on("/bridge/status", HTTP_GET', esp)
        self.assertEqual(esp.count('http_server.on("/debug", HTTP_GET'), 1)
        self.assertIn("ws_client_slot_allocate(client_id)", esp)
        self.assertIn("ws_client_slot(client_id)", esp)
        self.assertNotIn("client_id < WS_PROXY_CLIENTS", esp)
        self.assertIn('Serial2.println("WS,CONNECT")', esp)
        self.assertIn("uart_json_lines", esp)
        self.assertIn("log_flush()", esp)

    def test_browser_delivery_tracks_real_ws_clients_across_both_processors(self):
        esp = (PROJECT_ROOT / "esp32_bridge/src/main.cpp").read_text()
        main = MAIN
        dash = (ROOT / "Core/Src/dash.c").read_text()
        self.assertIn("if (ws_proxy_clients == 1u)", esp)
        self.assertIn('Serial2.println("WS,CONNECT")', esp)
        self.assertIn("if (ws_proxy_clients == 0u) Serial2.println(\"WS,DISCONNECT\")", esp)
        self.assertIn("ws_proxy_clients > 0u && millis() - ws_last_connect_heartbeat >= 5000u", esp)
        self.assertIn("if (ws_proxy_clients > 0u) ws_proxy.textAll(uart_line)", esp)
        self.assertIn('strcmp(esp_line, "WS,CONNECT") == 0', main)
        self.assertIn('strcmp(esp_line, "WS,DISCONNECT") == 0', main)
        self.assertIn("dash_bridge_connected()", main)
        self.assertIn("dash_bridge_disconnected()", main)
        self.assertIn("dash_bridge_text((char *)esp_line, strlen(esp_line))", main)
        self.assertIn("WS_BRIDGE_CLIENT", dash)

    def test_pipeline_diagnostics_cross_the_uart_and_reach_debug_api(self):
        esp = (PROJECT_ROOT / "esp32_bridge/src/main.cpp").read_text()
        dash = (ROOT / "Core/Src/dash.c").read_text()
        uart = (ROOT / "Core/Src/uart_esp.c").read_text()
        for counter in ("uart_esp_tx_dropped", "uart_esp_tx_dma_errors", "uart_esp_rx_dropped"):
            self.assertIn(counter, dash)
        for field in ("txd", "dma", "te", "dme", "fe", "flg", "rxd", "sent", "drop", "dbc"):
            self.assertIn(field, dash)
        self.assertIn('"type\\":\\"pipeline_diag', dash)
        self.assertIn("drop", dash)
        self.assertIn("can_dbc_msg_count", dash)
        self.assertIn("update_pipeline_diag(uart_line)", esp)
        for field in ("[PIPE] seen=%u", "records=%lu", "parse_fail=%lu",
                      "tx_drop=%lu", "dma=%lu", "[PIPE-DMA] TE=%lu",
                      "DME=%lu", "FE=%lu", "flags=0x%08lx", "rx_drop=%lu"):
            self.assertIn(field, esp)
        for field in ("stm32_uart_tx_dropped", "stm32_uart_tx_dma_errors",
                      "stm32_uart_dma_te", "stm32_uart_dma_dme", "stm32_uart_dma_fe",
                      "stm32_uart_dma_last_flags", "stm32_uart_rx_dropped",
                      "gvret_frames_dropped", "stm32_diag_age_s",
                      "stm32_diag_records", "stm32_diag_parse_failures"):
            self.assertIn(field, esp)
        self.assertIn("tx_dma_errors++", uart)
        self.assertIn("rx_dropped++", uart)

    def test_dashboard_pipeline_diagnostics_and_scope_are_visible(self):
        page = (ROOT / "web/index.html").read_text()
        self.assertIn('id="pipelineIntegrity"', page)
        self.assertIn("m.type==='pipeline_diag'", page)
        self.assertIn("TCP backpressure drops", page)
        self.assertIn("standard F-CAN definitions", page)
        self.assertIn("192-entry table", page)
        self.assertNotIn("still receives every frame", page)

    def test_dbc_decoder_explicitly_limits_bus_and_frame_format(self):
        dash = (ROOT / "Core/Src/dash.c").read_text()
        dbc = (ROOT / "Core/Inc/can_dbc.h").read_text()
        self.assertIn("e->bus != 0u || e->ext", dash)
        self.assertIn("standard IDs", dbc)
        tests = (ROOT / "tests/test_dash_bridge.c").read_text()
        self.assertIn("B-CAN, extended frames and undefined standard IDs remain raw-only", tests)

    def test_esp32_spiffs_dashboard_matches_current_web_source(self):
        dashboard = (ROOT / "web/index.html").read_bytes()
        compressed = (PROJECT_ROOT / "esp32_bridge/data/index.html.gz").read_bytes()
        self.assertEqual(gzip.decompress(compressed), dashboard)

    def test_no_usart3_or_esp32_uart_link(self):
        # Historical test name retained so its earlier failed record is superseded
        # by the current USART2/ESP32 wiring assertions.
        self.test_usart2_esp32_uart_link_is_configured()

    def test_tja1051_transmit_floor_is_not_bypassed(self):
        timing = (ROOT / "Core/Inc/periph_timing.h").read_text()
        self.assertIn("#define TJA1051_MIN_TX_BITRATE 20000u", timing)
        self.assertIn("can_tx_bitrate_allowed(c->current_bitrate)", CAN_HW)
        self.assertIn("BITRATE_BELOW_TRANSCEIVER_MIN", CAN_HW)

    def test_armed_bus_cannot_reconfigure_or_bypass_bitrate_floor(self):
        set_bitrate = CAN_HW.split("bool can_hw_set_bitrate(", 1)[1].split(
            "/* ── filters", 1)[0]
        self.assertIn("c->tx_armed) return false", set_bitrate)
        self.assertIn("c->tx_armed) return", CAN_HW.split(
            "void can_hw_start_passive(", 1)[1].split("uint32_t can_hw_compute", 1)[0])
        mode = CAN_HW.split("bool can_hw_set_controller_mode(", 1)[1].split(
            "void can_hw_abort_mailboxes", 1)[0]
        self.assertIn("mode == CAN_MODE_NORMAL", mode)
        self.assertIn("!c->tx_armed", mode)
        self.assertIn("!c->can_locked", mode)
        self.assertIn("can_tx_bitrate_allowed(c->current_bitrate)", mode)
        arm = CAN_HW.split("const char *can_hw_tx_arm(", 1)[1].split(
            "void can_hw_tx_disarm_hw", 1)[0]
        self.assertIn("seconds > TX_ARM_MAX_SECONDS", arm)
        self.assertIn("if (!c || !c->cfg)", arm)
        window = CAN_HW.split("bool can_hw_tx_window_open(", 1)[1].split(
            "bool can_hw_rate_ok", 1)[0]
        self.assertIn("c->mode != CAN_MODE_NORMAL", window)
        self.assertIn("can_tx_bitrate_allowed(c->current_bitrate)", window)
        load = CAN_HW.split("bool can_hw_load_mailbox(", 1)[1].split(
            "bool can_hw_tx_service", 1)[0]
        self.assertIn("!c->tx_armed", load)
        self.assertIn("c->mode != CAN_MODE_NORMAL", load)
        self.assertIn("dlc > 8u", load)
        bitrate_command = MAIN.split('if (strcmp(cmd, "BITRATE") == 0)', 1)[1].split(
            'if (strcmp(cmd, "TX") == 0)', 1)[0]
        self.assertIn("if (c->tx_armed)", bitrate_command)

    def test_arm_confirm_and_ignition_safety_are_preserved(self):
        timing = (ROOT / "Core/Src/uds_client.c").read_text()
        arm_command = MAIN.split('if (strcmp(cmd, "ARM") == 0)', 1)[1].split(
            'if (strcmp(cmd, "DISARM") == 0)', 1)[0]
        self.assertIn("if (!veh_key_on())", arm_command)
        self.assertIn("can_hw_tx_arm(c, secs", arm_command)
        uds_command = MAIN.split("static void uds_command(", 1)[1].split(
            "static void udspad_command", 1)[0]
        self.assertIn("if (!c->tx_armed)", uds_command)
        self.assertIn("conf_t != NULL", uds_command)
        self.assertIn("UDS_START_CONFIRM_REQUIRED", uds_command)
        self.assertIn("uds_request_needs_confirm(req, len) && !confirm", timing)
        ecu_command = MAIN.split("static void ecuscan_command(", 1)[1].split(
            "static bool ecuscan_skip", 1)[0]
        self.assertIn("if (!c->tx_armed)", ecu_command)
        self.assertIn("uds_request_needs_confirm(req, len)", ecu_command)
        parked = MAIN.split("if (old != VEH_OFF && veh == VEH_OFF)", 1)[1].split(
            "if (change ||", 1)[0]
        self.assertIn('disarm(&s_can[i], "KEY_OFF")', parked)
        # Owner requirement (2026-11-15): capture from the first frame until the
        # car is locked, so key OFF ends every TX-capable activity but leaves the
        # controllers in listen-only (SILM) capture, started at power-up.
        self.assertNotIn("can_hw_stop_passive", parked)
        self.assertIn('uds_stop(&s_can[i], "KEY_OFF")', parked)
        self.assertIn('ecuscan_stop(&s_can[i], "KEY_OFF")', parked)
        self.assertIn("remote_start_cancel", parked)
        boot = MAIN.split("can_hw_filters_init(&s_can[0], &s_can[1], true);", 1)[1][:400]
        self.assertIn("can_hw_start_passive(&s_can[i], system_millis)", boot)
        self.assertIn("periodic[i].active = false", parked)
        self.assertIn("gateway_on = false", parked)


if __name__ == "__main__":
    unittest.main()
