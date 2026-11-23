"""Source-level safety/retention contract for the ESP32 DTC event journal."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
ESP = (ROOT / "esp32-dashboard/src/main.cpp").read_text()


class EventJournalContractTests(unittest.TestCase):
    def test_bounded_pre_post_capture_and_metadata_csv(self):
        self.assertIn("EVENT_PRE_SAMPLES = 60", ESP)
        self.assertIn("EVENT_POST_SAMPLES = 10", ESP)
        self.assertIn("EVENT_SNAPSHOT_MAX = 8", ESP)
        self.assertIn("event_id,dtc,ecu,bus,event_utc,uptime_ms", ESP)
        self.assertIn("writeEventCsvRow(f, id, sample.csv, n)", ESP)
        self.assertIn("writeEventCsvRow(f, pending.id, csv, n)", ESP)
        self.assertIn("/event.csv", ESP)

    def test_journal_is_persistent_and_mount_failure_never_formats(self):
        self.assertIn("LittleFS.begin(false)", ESP)
        self.assertNotIn("LittleFS.begin(true)", ESP)
        self.assertIn("LittleFS.rename(EVENT_TMP, EVENT_FILE)", ESP)
        self.assertIn("loadEvents()", ESP)
        self.assertIn("saveEvents()", ESP)

    def test_unseen_alarms_are_never_evicted_for_capacity(self):
        self.assertIn("EVENT_MAX = 40", ESP)
        self.assertIn("if (eventStore[i].seen)", ESP)
        self.assertIn("all 40 events are still unacknowledged, preserving every alarm", ESP)
        self.assertIn("markEventSeen", ESP)

    def test_uds_dtc_response_is_reassembled_before_event_trigger(self):
        self.assertIn("offset != r.nextOffset", ESP)
        self.assertIn("strcmp(r.ecu, ecuField.c_str()) != 0", ESP)
        self.assertIn("completeDtcResponse(busField, csvField(raw, 3))", ESP)
        complete = ESP.split("static void completeDtcResponse", 1)[1].split("static bool markEventSeen", 1)[0]
        self.assertIn("captureDtcReply(busField, ecuField, String(r.hex))", complete)

    def test_auto_obd_and_dtc_requests_are_key_and_arm_gated(self):
        self.assertIn("armedBus==='F' && armedLeft>0 && (engineState==='ON'||engineState==='RUN')", ESP)
        self.assertIn("requests=['03','07','0A','1902FF']", ESP)
        self.assertIn("idNext,j.autoPid?200:20", ESP)
        self.assertIn("if(!pidOn || armedBus!=='F' || armedLeft<=0", ESP)
        self.assertIn("wsSend({op:'uds_abort',bus:'F'})", ESP)

    def test_acknowledged_faults_do_not_reopen_on_each_scan(self):
        self.assertIn("lastReportedMs != 0u", ESP)
        self.assertIn("< 120000u", ESP)
        self.assertIn("latest->lastReportedMs = millis()", ESP)


if __name__ == "__main__":
    unittest.main()
