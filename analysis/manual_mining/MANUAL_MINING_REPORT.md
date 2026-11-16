LEMON manual mining — 2013 Civic R18Z1
Corpus and method
Source: offline LEMON manual folder supplied in the project root.
HTML article pages scanned: 33,358 (all files under pages/).
Distinct title strings: 9,535.
Search index: page_catalog.tsv (one row per HTML page, title and topic hits).
2013 R18Z1-labelled engine/fuel DTC index: 140 rows, in r18z1_2013_dtc_index.csv.
High-value areas across the corpus
Area Unique titles / pages
DTC / fault descriptions 1,625 / 4,882
DTC troubleshooting indexes 32 / 126
Wiring and circuit diagrams 243 / 728
Network / CAN communications 101 / 312
Sensor and component locations / tests 299 / 876
System descriptions 270 / 1,168
Troubleshooting procedures 439 / 1,542
Snapshot / freeze / scan-data topics 18 / 256
Titles explicitly mentioning R18Z1 408 / 2,134

Full-corpus term-hit counts: troubleshooting 14,812; sensor 8,976; HDS 8,208; CAN 8,156; DTC 8,068; diagnostic 7,477; snapshot 2,940; DLC 2,680; data list 1,600; F-CAN 733; B-CAN 726; VTEC 530; MAF 475.

Confirmed technical findings
OBD/DLC and network
pages/4802.html — PGM-FI System Description - OBD (12–13): DLC 6/14 are F-CAN H/L for PGM-FI; pin 7 is K-Line for communication other than PGM-FI; pin 9 SCS; pins 4/5 ground; pin 16 HDS +B. It also documents pending DTC after a first failed drive cycle and confirmed DTC/MIL after the fault persists into a second drive cycle; MIL indication is driven through the gauge-control module over F-CAN.
pages/32913.html — 1.8L Engine Performance Circuit, W/O CNG, five sheets (Figs 6–10; SVGs VA396929–VA396933). Text-label extraction is saved in R18Z1_ENGINE_WIRING_TEXT.md. It identifies ECM/PCM, MAF/IAT, MAP, ECT1/2, APP A/B, throttle actuator/body, CKP, CMP, knock, A/F sensor 1, secondary HO2S 2, injectors, ignition coils, EGR, EVAP purge/vent, FTP, fuel-tank unit, VTEC oil-control solenoid and pressure switch.
pages/32907.html — Computer Data Lines, part of the selected 2013 non-hybrid System Wiring Diagrams. Its Fig. 1 is explicitly “B-CAN Circuit & S-NET Circuit, Except Hybrid”; Fig. 3/4 show the non-hybrid DLC circuit. The corresponding images/VA396521.svg diagram includes the MICU, PCM, gauge control module/tachometer, junction boxes and optional audio/navigation nodes. This is the correct non-hybrid topology reference. The drawing uses pink/blue paired conductors at several connector locations.
pages/1684.html and pages/1690.html — 2013 Navigation System diagnostic mode identifies F-CAN at 500 kbps and B-CAN at 125 kbps.
pages/775.html — F-CAN circuit troubleshooting: documented nodes include ECM/PCM, gauge control module, VSA modulator, steering-angle sensor, EPS, SRS, TPMS and optional audio-navigation unit. This is topology evidence.
pages/771.html — DLC troubleshooting also references K-Line and other HDS module communications; B-CAN is shown in the non-hybrid network drawing at page 32907.
pages/19339.html — Network Communication DTC Troubleshooting Index (2013); pages/4199.html — B-CAN troubleshooting information.
Engine and OEM data list
pages/4795.html — ECM/PCM controls fuel injection, injection timing, idle and ETCS; fail-safe may substitute a preprogrammed value and store a confirmed DTC/MIL. Important shutdown nuance: ECM/PCM self-shutdown mode may keep it awake for up to one hour after ignition LOCK.
pages/4520.html — R18Z1 PGM-FI component index: MAF/IAT, ECT1/2, MAP and ECM/PCM.
pages/4522.html — VTEC component index: rocker-arm oil-control valve and related components.
pages/4807.html — VTEC control uses vehicle speed, engine speed, engine load, throttle position and coolant temperature.
pages/4806.html and pages/4810.html — VTEC construction/operation. Page 4807 identifies control inputs as vehicle speed, engine speed, engine load, throttle position and coolant temperature; page 4810 describes solenoid ON/OFF operation.
pages/18526/18528.html — P2646/P2647 diagnostics explicitly use HDS DATA LIST item VTEC PRES SW and HDS VTEC TEST.
pages/18530/18532.html — P2648/P2649 cover rocker-arm oil-control solenoid circuit faults.
pages/16643.html — R18Z1 misfire diagnostics use CYL1 MISFIRE through CYL4 MISFIRE, compare idle and static load, and correlate ECT, RPM, speed, MAP and TP with an onboard snapshot.
Factory snapshots
pages/4804.html — Honda PGM-FI onboard snapshot is explicitly for engine stalls and hard-start/no-start. Stall snapshot is 10 seconds: 9 seconds before and 1 second after, subject to stated preconditions.
pages/16131.html — R18Z1 stall troubleshooting: HDS can sample 30 seconds; if fault cannot be reproduced, use Engine Stall 2 snapshot and compare MAF/RPM patterns.
pages/16134.html — R18Z1 hard-start/no-start procedure checks Hard to Start Engine and Engine Stall 2 snapshots.
pages/16643.html — misfire diagnostic asks to record freeze data and onboard snapshot before analysis.
pages/4795.html — ECM/PCM may remain awake for up to one hour after LOCK.
Detailed R18Z1 diagnostic extraction
The 140 DTC index rows resolve to 129 unique full troubleshooting articles in R18Z1_DTC_DETAILED_EXTRACTION.md. These include preconditions, freeze-data preservation warnings, HDS Data List items, inspection sequences, connector/test-point references and repair-verification steps.
HONDA_HDS_DATA_LIST_ITEMS.md separates explicit Honda HDS parameter names from generic SAE PIDs and records source pages.
2013 R18Z1-labelled engine DTC categories
Electrical / power: 11 index entries
Emissions systems: 10 index entries
Fuel / combustion / emissions: 22 index entries
Network / communication: 6 index entries
Other: 16 index entries
Sensors / air / engine position: 58 index entries
Throttle / idle: 13 index entries
VTEC: 4 index entries

The CSV maps each DTC-index row to the detailed troubleshooting article. Some labels include North American model codes.

Proposed vehicle-monitor architecture
Acquisition, snapshot/event recorder, dashboard lifecycle, detection layers and implementation order: Firmware implementation: see [FIRMWARE_FEATURES.md](../../FIRMWARE_FEATURES.md) §6–§9.
ST Edge AI compatibility check
STM32Cube AI Studio 1.4.0 and ST Edge AI Core 4.1.0-20732 are now installed successfully. The stedgeai --version and stedgeai --tools commands both execute successfully; setup details are recorded in ST_EDGE_AI_SETUP.md.
The installed ST Edge AI STM32 target configuration explicitly lists the STM32F4 family and selects the ARM Cortex-M4 GCC runtime/library path with Cortex-M4 hard-float compiler flags. The installed STM32 MCU library contains the ARMCortexM4 GCC runtime. This confirms a supported compile/runtime path for the STM32F407.
Official ST Edge AI Core 4.1.0 documentation lists STM32F4xx as a Cortex-M4/FPU target. The runtime can still be compiled/linked.
For the anomaly-detection goal, ST’s separate NanoEdge AI Studio is the more directly relevant AutoML product: it generates anomaly/outlier/classification/regression libraries and supports STM32. The current ST product page routes software through a request/approval process (up to 48 hours).
it just take snapshot when it found mis happening in its observations.
Primary source pages

4802, 4795, 4804, 4806, 4807, 4810, 4520, 4522, 1684, 1690, 32907, 32913, 775, 771, 19339, 4199, 18526, 18528, 18530, 18532, 16643, 17956, 17959, 5643, 16131, 16134, 4474.

Official ST AI references checked
ST Edge AI Core 4.1.0 STM32 target support: <https://dev.st.com/edge-ai-docs/st-edge-ai-core/4.1.0/en/docs/user-manual/core-stm32-cli.html>
ST Edge AI Core 4.1.0 release notes (Cortex-M4 validation limitation): <https://dev.st.com/edge-ai-docs/st-edge-ai-core/4.1.0/en/docs/user-manual/core-release-notes.html>
NanoEdge AI Studio product page (anomaly detection and software request): <https://www.st.com/en/development-tools/nanoedgeaistudio.html>
