"""Source-level regression tests for the supplied Project Civic pinout."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
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

    def test_can_tx_is_push_pull(self):
        self.assertNotIn("OTYPER |=", CAN_HW)
        self.assertIn("g->OTYPER  &= ~(1u << pin);", CAN_HW)

    def test_tja1051_s_pin_silent_except_normal(self):
        self.assertIn("transceiver_follow_mode", CAN_HW)
        self.assertIn("(m == CAN_MODE_NORMAL) ? (1u << (cfg->s_pin + 16u))", CAN_HW)
        self.assertIn("S HIGH = silent first", CAN_HW)

    def test_each_can_instance_has_its_own_rx_queue(self):
        self.assertIn("capture_queue[2]", MAIN)
        self.assertIn("&capture_queue[0]", MAIN)
        self.assertIn("&capture_queue[1]", MAIN)
        self.assertIn("capture_queue[c->cfg->bus_index].dropped", MAIN)
        self.assertGreaterEqual(MAIN.count("can_capture_queue_pop(&capture_queue[b]"), 1)

    def test_no_usart3_or_esp32_uart_link(self):
        for stale in ("USART3", "uart_dma", "host_rx_pop", "PB10", "GPIOB->AFR[1]"):
            self.assertNotIn(stale, MAIN)
        self.assertFalse((ROOT / "Core/Src/uart_dma.c").exists())

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
