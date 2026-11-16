"""USB network adapter uses ST's official CDC-ECM class (STM32CubeF4 USB
Device Library) and fits the OTG_FS hardware limits (RM0090 34.11)."""
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MK = (ROOT / "Makefile").read_text()
NET = (ROOT / "Core/Src/net_usb.c").read_text()
CONF = (ROOT / "USB/ecm/usbd_conf_ecm.c").read_text()
IFH = (ROOT / "USB/ecm/usbd_cdc_ecm_if_template.h").read_text()
ECM_C = ROOT / "Middlewares/ST/STM32_USB_Device_Library/Class/CDC_ECM/Src/usbd_cdc_ecm.c"


class UsbEcmConfigTests(unittest.TestCase):
    def test_st_cdc_ecm_class_is_built_and_tinyusb_is_gone(self):
        self.assertIn("$(USB_MW)/Class/CDC_ECM/Src/usbd_cdc_ecm.c", MK)
        self.assertNotIn("tinyusb", MK.lower())
        self.assertTrue(ECM_C.exists())
        self.assertIn("COPYRIGHT", ECM_C.read_text().upper())            # vendor file, unmodified header
        self.assertIn("USBD_RegisterClass(&USBD_Device, USBD_CDC_ECM_CLASS)", NET)

    def test_fifo_budget_fits_1_25_kb(self):
        rx = [int(x, 16) for x in re.findall(r"HAL_PCDEx_SetRxFiFo\(&hpcd_USB_OTG_FS, (0x[0-9A-Fa-f]+)\)", CONF)]
        tx = [int(x, 16) for x in re.findall(r"HAL_PCDEx_SetTxFiFo\(&hpcd_USB_OTG_FS, \d, (0x[0-9A-Fa-f]+)\)", CONF)]
        self.assertEqual(len(rx), 1)
        self.assertEqual(len(tx), 3)                                       # EP0, ECM bulk IN, ECM notify
        self.assertLessEqual(sum(rx) + sum(tx), 320)                       # 320 words of FIFO RAM
        self.assertGreaterEqual(min(tx), 16)                               # >= one 64-byte packet

    def test_mac_strings_agree_and_board_mac_differs(self):
        self.assertIn('"024349564943"', NET)
        self.assertIn('"024349564943"', IFH)
        self.assertIn("netif->hwaddr[5] ^= 0x01", NET)

    def test_rx_is_flow_controlled_and_tx_bounded(self):
        # OUT endpoint is re-armed only after lwIP got the frame; TX wait is bounded.
        body = NET.split("void net_usb_poll(void)", 1)[1]
        self.assertLess(body.index("netif_data.input"), body.index("USBD_CDC_ECM_ReceivePacket"))
        self.assertIn("> 20u) return ERR_MEM", NET)
        self.assertIn('__attribute__((section(".ccmram")))', NET)          # CPU-only buffer, no DMA
        self.assertIn("dma_enable = DISABLE", CONF)


if __name__ == "__main__":
    unittest.main()
