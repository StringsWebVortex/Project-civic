"""Source-level regression tests for the supplied Project Civic pinout."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "Core/Src/main.c").read_text()
CAN_HW = (ROOT / "Core/Src/can_hw.c").read_text()
CAN_H = (ROOT / "Core/Inc/can_hw.h").read_text()
UART_DMA = (ROOT / "Core/Src/uart_dma.c").read_text()
UART_H = (ROOT / "Core/Inc/uart_dma.h").read_text()
STARTUP = (ROOT / "startup_stm32f407xx.s").read_text()
ESP = (ROOT / "esp32-dashboard/src/main.cpp").read_text()


class HardwarePinConfigTests(unittest.TestCase):
    def test_can_pins_and_nominal_scan_rates(self):
        self.assertRegex(MAIN, r"\.can\s*=\s*CAN1,\s*\.bus_index\s*=\s*0u,\s*\.gpio\s*=\s*GPIOB")
        self.assertRegex(MAIN, r"\.rx_pin\s*=\s*8u,\s*\.tx_pin\s*=\s*9u,\s*\.af\s*=\s*9u")
        self.assertRegex(MAIN, r"\.preferred_bitrate\s*=\s*500000u")
        self.assertRegex(MAIN, r"\.can\s*=\s*CAN2,\s*\.bus_index\s*=\s*1u,\s*\.gpio\s*=\s*GPIOB")
        self.assertRegex(MAIN, r"\.rx_pin\s*=\s*5u,\s*\.tx_pin\s*=\s*6u,\s*\.af\s*=\s*9u")
        self.assertRegex(MAIN, r"\.preferred_bitrate\s*=\s*125000u")
        self.assertIn("c->cfg->preferred_bitrate", CAN_HW)
        self.assertIn("500000u", CAN_HW)
        self.assertIn("125000u", CAN_HW)
        self.assertIn("33333u", CAN_HW)

    def test_each_can_instance_has_its_own_rx_queue(self):
        self.assertIn("capture_queue[2]", MAIN)
        self.assertIn("&capture_queue[0]", MAIN)
        self.assertIn("&capture_queue[1]", MAIN)
        self.assertIn("capture_queue[c->cfg->bus_index].dropped", MAIN)
        self.assertGreaterEqual(MAIN.count("can_capture_queue_pop(&capture_queue[b]"), 1)

    def test_usart3_rx_tx_and_dma1_stream3_channel4(self):
        self.assertIn("GPIOB->AFR[1]", MAIN)
        self.assertIn("RCC_AHB1ENR_GPIOBEN", MAIN)
        self.assertIn("RCC_APB1ENR_USART3EN", MAIN)
        self.assertIn("GPIOB->MODER", MAIN)
        self.assertIn("void USART3_IRQHandler(void)", MAIN)
        self.assertIn("USART3->SR", MAIN)
        self.assertIn("USART3->DR", MAIN)
        self.assertIn("DMA1_Stream3", UART_DMA)
        self.assertIn("(4u << DMA_SxCR_CHSEL_Pos)", UART_DMA)
        self.assertIn("&USART3->DR", UART_DMA)
        self.assertIn("DMA1->LISR", UART_DMA)
        self.assertIn("DMA1->LIFCR", UART_DMA)
        self.assertIn("DMA1_Stream3_IRQn", UART_DMA)
        self.assertIn("void DMA1_Stream3_IRQHandler(void)", UART_DMA)
        self.assertIn("DMA1 Stream3 Channel4 for USART3 TX", UART_H)
        self.assertIn("DMA1_Stream3_IRQHandler", STARTUP)
        self.assertIn("USART3_IRQHandler", STARTUP)

    def test_esp32_application_and_rom_use_same_uart_wires(self):
        self.assertRegex(ESP, r"STM32_RX\s*=\s*16")
        self.assertRegex(ESP, r"STM32_TX\s*=\s*17")
        self.assertRegex(ESP, r"STM32_BOOT0\s*=\s*26")
        self.assertRegex(ESP, r"STM32_NRST\s*=\s*25")
        self.assertIn("Stm32.begin(STM32_BAUD, SERIAL_8N1, STM32_RX, STM32_TX)", ESP)
        self.assertIn("Stm32.begin(57600, SERIAL_8E1, STM32_RX, STM32_TX)", ESP)
        self.assertIn("SB19 ON", ESP)
        self.assertIn("no erase was sent", ESP)
        for stale in ("STM32_BOOT1", "STM32_BL_RX", "STM32_BL_TX", "GPIO27", "GPIO4 → BOOT0", "GPIO5 → NRST"):
            self.assertNotIn(stale, ESP)

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
