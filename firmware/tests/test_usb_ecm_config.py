"""USB network adapter uses RNDIS class (Android/Linux/Windows compatible)
and fits the OTG_FS hardware limits (RM0090 34.11)."""
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MK = (ROOT / "Makefile").read_text()
NET = (ROOT / "Core/Src/net_usb.c").read_text()
CONF = (ROOT / "USB/rndis/usbd_conf_rndis.c").read_text()
RNDIS_C = ROOT / "USB/rndis/usbd_rndis.c"
RNDIS_H = ROOT / "USB/rndis/usbd_rndis.h"


class UsbEcmConfigTests(unittest.TestCase):
    def test_st_cdc_ecm_class_is_built_and_tinyusb_is_gone(self):
        """RNDIS class is built instead of CDC-ECM; tinyusb is still gone."""
        self.assertIn("USB/rndis/usbd_rndis.c", MK)
        self.assertNotIn("tinyusb", MK.lower())
        self.assertTrue(RNDIS_C.exists())
        self.assertTrue(RNDIS_H.exists())
        self.assertIn("USBD_RegisterClass(&USBD_Device, USBD_RNDIS_CLASS)", NET)

    def test_fifo_budget_fits_1_25_kb(self):
        rx = [int(x, 16) for x in re.findall(r"HAL_PCDEx_SetRxFiFo\(&hpcd_USB_OTG_FS, (0x[0-9A-Fa-f]+)\)", CONF)]
        tx = [int(x, 16) for x in re.findall(r"HAL_PCDEx_SetTxFiFo\(&hpcd_USB_OTG_FS, \d, (0x[0-9A-Fa-f]+)\)", CONF)]
        self.assertEqual(len(rx), 1)
        self.assertGreaterEqual(len(tx), 3)                                # EP0, notify IN, bulk IN
        self.assertLessEqual(sum(rx) + sum(tx), 320)                       # 320 words of FIFO RAM
        self.assertGreaterEqual(min(tx), 16)                               # >= one 64-byte packet

    def test_mac_strings_agree_and_board_mac_differs(self):
        """Host and board MACs are locally-administered and differ by one bit."""
        rndis_src = RNDIS_C.read_text()
        self.assertIn("0x02, 0x43, 0x49, 0x56, 0x49, 0x43", NET)
        self.assertIn("0x02, 0x43, 0x49, 0x56, 0x49, 0x43", rndis_src)
        self.assertIn("hwaddr[5] ^= 0x01", NET)

    def test_rx_is_flow_controlled_and_tx_bounded(self):
        """OUT endpoint is re-armed only after lwIP got the frame; TX wait is bounded."""
        body = NET.split("void net_usb_poll(void)", 1)[1]
        self.assertLess(body.index("netif_data.input"), body.index("usbd_rndis_rx_done"))
        self.assertIn("> 20u) return ERR_MEM", NET)
        # TX buf lives in CCM RAM (CPU-only, no DMA)
        rndis_src = RNDIS_C.read_text()
        self.assertIn('__attribute__((section(".ccmram")))', rndis_src)
        self.assertIn("dma_enable = DISABLE", CONF)


if __name__ == "__main__":
    unittest.main()
