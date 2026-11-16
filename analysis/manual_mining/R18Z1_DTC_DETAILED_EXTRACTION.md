# 2013 R18Z1 DTC diagnostic evidence extraction

- DTC index rows: 140
- Unique detailed troubleshooting pages: 129
- Source: selected 2013 Honda Civic manual archive. The DTC index includes variant qualifiers (KA/KC/KL etc.).
- This is a reference extraction, not an instruction to perform resets, actuator tests, or repairs. Preserve freeze-frame and snapshot data before any clearing/reset.

## P0201, P0202, P0203, P0204 — page 16634

- Source file: pages/16634.html

DTC P0201: No. 1 Cylinder Injector Circuit Malfunction, DTC P0202: No. 2 Cylinder Injector Circuit Malfunction, DTC P0203: No. 3 Cylinder Injector Circuit Malfunction, DTC P0204: No. 4 Cylinder Injector Circuit Malfunction ('12-'13: R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 20 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0201, P0202, P0203, or P0204 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the injectors and the ECM/PCM.

2. Injector internal Ω check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector (s).
| Injector 2P connector (problem cylinder)
- 1. At the injector side, measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Injector 2P connector (problem cylinder): disconnected
| Test circuit | IGP, INJ
| Test point 1 | Injector 2P connector No. 1 (injector side)
| Test point 2 | Injector 2P connector No. 2 (injector side)
| Courtesy of HONDA, U.S.A., INC.
Is there 10-13 --
YES
The injector is OK. Go to step 3.
NO
Replace the problem injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , then do the repair verification.

3. Open wire check (IGP line):

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Injector 2P connector (problem cylinder): disconnected
| Test circuit | IGP
| Test point 1 | Injector 2P connector No. 2 (YEL/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IGP wire is OK. Go to step 4.
NO
Repair an open in the IGP wire between the injector and PGM-FI main relay 1, then go to step 6.

4. Determine possible failure area (INJ line short, INJ line open):

- 1. Turn the ignition switch to LOCK (0).
- 1. Check for continuity between body ground and the appropriate injector connector terminal (see table).
| Test condition | IG LOCK (0)
| | Injector 2P connector (problem cylinder): disconnected
| PROBLEM CYLINDER | DTC | WIRE COLOR
| No. 1 | P0201 | BRN
| No. 2 | P0202 | RED
| No. 3 | P0203 | BLU
| No. 4 | P0204 | YEL
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 5.
NO
Go to step 6.

5. Shorted wire check (INJ lines):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between body ground and the appropriate injector connector terminal of the problem cylinder (see table).
| Test condition | IG LOCK (0)
| | Injector 2P connector (problem cylinder): disconnected
| | ECM/PCM connector C (49P): disconnected
| PROBLEM CYLINDER | DTC | WIRE COLOR
| No. 1 | P0201 | BRN
| No. 2 | P0202 | RED
| No. 3 | P0203 | BLU
| No. 4 | P0204 | YEL
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the INJ wire between the ECM/PCM and the injector, then do the repair verification.
NO
The INJ wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM- Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

6. Open wire check (INJ lines):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between the appropriate injector 2P connector terminal No. 1 and the ECM/PCM connector terminal of the problem cylinder (see table).
| Test condition | IG LOCK (0)
| | Injector 2P connector (problem cylinder): disconnected
| | ECM/PCM connector C (49P): disconnected
| PROBLEM CYLINDER | DTC | ECM/PCM TERMINAL | WIRE COLOR
| No. 1 | P0201 | C5 | BRN
| No. 2 | P0202 | C6 | RED
| No. 3 | P0203 | C7 | BLU
| No. 4 | P0204 | C8 | YEL
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The INJ wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM- Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the INJ wire between the ECM/PCM and the injector, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0201, P0202, P0203, or P0204 indicated?
YES
Check for poor connections or loose terminals at the injector and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0222 — page 16637

- Source file: pages/16637.html

DTC P0222: TP Sensor B Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check TP SENSOR B in the DATA LIST with the HDS.
Is there about 0. 3 V or less?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Determine possible failure area (TH_2 line, others):

- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P0122 and P0222 indicated at the same time?
YES
Go to step 4.
NO
Go to step 3.

3. Shorted wire check (TH_2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | TH_2
| Test point 1 | Throttle body 6P connector No. 4 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TH_2 wire between the ECM/PCM (C31) and the throttle body, then do the repair verification.
NO
The TH_2 wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Determine possible failure area (Throttle body, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Throttle body 6P connector: disconnected
| Test circuit | VCC3
| Test point 1 | Throttle body 6P connector No. 5 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5. 0 V?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Cleaning (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Cleaning (Except K24Z7) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Go to step 5.

5. Open wire check (VCC3 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Throttle body 6P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VCC3
| Test point 1 | ECM/PCM connector C (49P) No. 22 (GRN)
| Test point 2 | Throttle body 6P connector No. 5 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC3 wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC3 wire between the ECM/PCM (C22) and the throttle body, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0222 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0223 — page 16640

- Source file: pages/16640.html

DTC P0223: TP Sensor B Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check TP SENSOR B in the DATA LIST with the HDS.
Is there about 4. 8 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Determine possible failure area (SG3 line, TH_2 line):

- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P0123 and P0223 indicated at the same time?
YES
Go to step 5.
NO
Go to step 3.

3. Determine possible failure area (Throttle body, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON(II)
| | Throttle body 6P connector: disconnected
| Test circuit | TH_2
| Test point 1 | Throttle body 6P connector No. 4 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5. 0 V?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Cleaning (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Cleaning (Except K24Z7) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. Open wire check (TH_2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Throttle body 6P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | TH_2
| Test point 1 | ECM/PCM connector C (49P) No. 31 (YEL)
| Test point 2 | Throttle body 6P connector No. 4 (YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TH_2 wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TH_2 wire between the ECM/PCM (C31) and the throttle body, then do the repair verification.

5. Open wire check (SG3 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | SG3
| Test point 1 | ECM/PCM connector C (49P) No. 14 (BLU)
| Test point 2 | Throttle body 6P connector No. 3 (BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG3 wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG3 wire between the ECM/PCM (C14) and the throttle body, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0223 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0300, P0301, P0302, P0303, P0304 — page 16643

- Source file: pages/16643.html

DTC P0300: Random Misfire Detected DTC P0301: No. 1 Cylinder Misfire Detected DTC P0302: No. 2 Cylinder Misfire Detected DTC P0303: No. 3 Cylinder Misfire Detected DTC P0304: No. 4 Cylinder Misfire Detected ('12-'13: R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) How to Troubleshoot the Fuel and Emissions Systems (R18Z1) How to Troubleshoot the A/T System (A/T) How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
If the misfire is frequent enough to trigger detection of increased emissions during two consecutive driving cycles, the MIL will come on, and DTC P0300 (and some combination of P0301 through P0304) will be stored.
If the misfire is frequent enough to damage the catalyst, the MIL will flash whenever the misfire occurs, and DTC P0300 (and some combination of P0301 through P0304) will be stored. When the misfire stops, the MIL will remain on.
Troubleshoot the following DTCs first, if any of them were stored along with the random misfire DTC (s) (Because parts can sometimes fail without setting DTC's you should also do a physical inspection of the systems listed below):
P0102, P0103: MAF sensor
P0107, P0108: MAP sensor
P0171, P0172: Fuel system
P0201, P0202, P0203, P0204: No. 1?No. 4 cylinder injector
P0335, P0339: CKP sensor
P0365, P0369: CMP sensor
P0351, P0352, P0353, P0354: No. 1 - No. 4 cylinder ignition coil (s)
P2646, P2647, P2648, P2649: VTEC system
P0404, P0406, P2413: EGR system

1. Failure Confirmation - Idle

- 1. Record the FREEZE DATA and the ONBOARD SNAPSHOT with the HDS.
- 1. Start the engine, and let it idle.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Are the misfire counts increasing on any cylinder at idle?
YES
The engine is misfiring at idle, go to step 2.
NO
The engine does not misfire at idle, go to step 3.

2. Determine Affected Cylinders

- 1. Continue to monitor the misfire parameters in the DATA LIST.
Are misfire counts generally isolated to a single cylinder?
YES
The misfire is isolated to a single cylinder.
If the misfire counts are increasing every engine revolution (completely inactive cylinder), go to Single Cylinder Continuous Misfire at Idle troubleshooting on step 6.
If the misfire counts are increasing every few engine revolutions (partially inactive cylinder), go to Single Cylinder Non-Continuous Misfire at Idle troubleshooting on step 21.
NO
The misfire affects multiple cylinders.
If the misfire counts are increasing every engine revolution (completely inactive cylinder), go to Multiple Cylinder Continuous Misfire at Idle troubleshooting on step 30.
If the misfire counts are increasing every few engine revolutions (partially inactive cylinder), start the Multiple Cylinder Non-Continuous Misfire at Idle troubleshooting on step 48.

3. Failure Confirmation - Static Load

- 1. Increase the engine speed to 3, 500 RPM.
- 1. Hold the engine speed steady, and monitor the misfire parameters in the DATA LIST.
Are the misfire counts increasing on any cylinder?
YES
The engine is misfiring under load. Go to step 4.
NO
The engine does not misfire under a static load. Go to step 5.

4. Determine Affected Cylinders

- 1. Continue to monitor the misfire parameters in the DATA LIST.
Are misfire counts generally isolated to a single cylinder?
YES
The misfire is isolated to a single cylinder, start the Single Cylinder Misfire. Under Load troubleshooting on step 57.
NO
The misfire affects multiple cylinders, start the Multiple Cylinder Misfire. Under Load troubleshooting on step 64.

5. Failure Confirmation - OBD Snapshot

- 1. Open the ONBOARD SNAPSHOT with the HDS.
- 1. Scroll through the snapshot, and find when the MISFIRE value begins to increase.
- 1. Record the value of these parameters as the MISFIRE value increases:
| Signal | Value | Units
| ECT SENSOR 1 | | °F
| ENGINE SPEED | | RPM
| VEHICLE SPEED | | MPH
| MAP SENSOR | | V
| TP SENSOR | | V
- 1. Drive the vehicle according to the conditions you identified above while monitoring the misfire counts.
Are the misfire counts increasing on any cylinder?
YES
The engine is misfiring under the specific operating conditions above.
If the misfire occurred when the engine was cold, start the Misfire When Cold troubleshooting on step 71.
If the misfire occurred when the engine was loaded, go to step 4.
NO
Intermittent failure; the engine is not misfiring at this time. NOTE: If the misfire can not be reproduced, there is a possibility that the problem may have been caused by poor quality fuel. The problem may have disappeared after refueling with fresh fuel. The MIL DIST (MIL Distance) value in the Data List in combination with the fuel level may help you make this determination.

6. Check Ignition Coil Spark

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the ignition coil from the problem cylinder.
- 1. Connect a commercially available spark tester between the ignition coil and engine ground.
- 1. Start the engine with the ignition coil connected.
Does the ignition coil produce a spark?
YES
The ignition coil wiring is OK. Go to step 14.
NO
There is a problem with the ignition coil, or the Ignition coil circuit. Go to step 7.

7. Ignition Coil Check

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Connect a commercially available spark tester between the ignition coil and engine ground.
- 1. Start the engine with the ignition coil connected.
Does the ignition coil produce a spark?
YES
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
There is a problem with the ignition coil circuit. Go to step 8.

8. Check Ignition Coil Power and Ground

- 1. Disconnect the ignition coil 3P connector from the problem cylinder.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 1 (power) and No. 2 (ground).
NOTE: After the ignition switch is turned to LOCK (0), this circuit may still show battery voltage for a period of time. This is normal.
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The ignition coil power and ground circuits are OK. Go to step 9.
NO
There is a problem with the ignition coil power or ground circuit. Go to step 13.

9. Check Ignition Coil Control Short to Power

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 3 (Ignition Control) and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there about 4 V or more?
YES
The ignition coil control circuit is shorted to power. Go to step 10.
NO
The ignition coil control circuit is not shorted to power. Go to step 11.
 1. Isolate Ignition Coil Control Short to Power

- 1. Jump the SCS line with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the ECM/PCM connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 3 (Ignition Control) and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there about 4 V or more?
YES
There is a short to power on the ignition coil control circuit. Find and repair the failure, and then perform the Repair Confirmation.
NO
The ECM/PCM is malfunctioning. Replace the ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
 1. Check Ignition Coil Control Open Circuit

- 1. Jump the SCS line with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the ECM/PCM connectors.
- 1. Check for continuity between the ignition coil control terminal at the ECM/PCM connector and at the ignition coil 3P connector terminal No. 3.
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The ignition coil control circuit is not open. Go to step 12.
NO
Repair the open circuit between the ignition coil and the ECM/PCM, and then perform the Repair Confirmation.
 1. Check Ignition Coil Control Short to Ground

- 1. Check for continuity between the ignition coil control terminal at the ignition coil 3P connector terminal No. 3 and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair the short to ground on the ignition coil control wire, and then perform the Repair Confirmation.
NO
The ignition coil control wiring is OK. Substitute a known good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
 1. Check Ignition Coil Power Circuit

- 1. Measure the voltage between ignition coil 3P connector terminal No. 1 (power) and a known-good engine ground.
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage between the ignition coil power terminal and engine ground?
YES
The power circuit is OK. Repair the open circuit between the ignition coil (3P connector terminal No. 2) and G101, and then perform the Repair Confirmation.
NO
Repair the open circuit between the ignition coil and the ignition coil relay, and then perform the Repair Confirmation.
 1. Inspect Ignition Coil

- 1. Inspect the ignition coil and spark plug tube for engine oil contamination.
Is the ignition coil covered in engine oil?
YES
Clean the oil from the ignition coil and spark plug tube. Replace the spark plug tube seals in the cylinder head cover - Refer to: Cylinder Head Cover Removal and Installation (K24Z7) , Cylinder Head Cover Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step15.
 1. Exchange Spark Plug and Ignition Coil

- 1. Exchange the spark plug and the ignition coil with one from a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The ignition coil and spark plug are OK. Go to step 17.
NO
There is a failure in the ignition coil or the spark plug. Go to step 16.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The spark plug has a failure. Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Check Cylinder Leakage

- 1. Perform a cylinder leakage test.
- 1. Record your test results in the chart shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Leakage % | Leakage Area
| No. 1 | |
| No. 2 | |
| No. 3 | |
| No. 4 | |
Does any cylinder have leakage greater than 10%?
YES
If the majority of the air is escaping from exhaust pipe or throttle body, check for tight valve clearance and adjust each cylinder as needed, then retest. Determine the cause of the cylinder leakage and repair the engine as needed, and then perform the Repair Confirmation.
NO
The cylinders are sealing properly. Go to step 18.
 1. Check Cylinder Compression

- 1. Perform a cylinder compression test.
- 1. Record your test results in a chart similar to the one shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Compression
| No. 1 |
| No. 2 |
| No. 3 |
| No. 4 |
| Highest Reading | Lowest Reading | Variation %
| | |
Does the compression vary more than 10% between cylinders, or does any cylinder read below 150 PSI?
YES
The cylinder leakage is normal, but the engine compression is low. Check for each of the conditions below and repair as needed, and then perform the Repair Confirmation.
Incorrect camshaft timing.
A bent (short) connecting rod.
Worn or damaged camshaft lobes.
NO
The engine compression and cylinder leakage is normal. Go to step 19.
 1. Exchange Fuel Injector

- 1. Exchange the fuel injector from the problem cylinder with one from another cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do the misfire counts stay on the problem cylinder after the fuel injector is swapped?
YES
The fuel injector is OK. Go to step 20.
NO
The fuel injector is faulty. Replace the faulty fuel injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
 1. CKP Pattern Clear/Pattern Learn

- 1. Select CRANK PATTERN in the ADJUSTMENT MENU with the HDS.
- 1. Select CRANK PATTERN CLEAR, and clear the CKP pattern.
- 1. Select CRANK PATTERN LEARNING with the HDS, and follow the screen prompts.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
NO
Perform the Repair Confirmation.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step22.
 1. Inspect Ignition Coil

- 1. Inspect the ignition coil and spark plug tube for engine oil contamination.
Is the ignition coil covered in engine oil?
YES
Clean the oil from the ignition coil and spark plug tube. Replace the spark plug tube seals in the cylinder head - Refer to: Cylinder Head Cover Removal and Installation (K24Z7) , Cylinder Head Cover Removal and Installation (Except K24Z7) cover, and then perform the Repair Confirmation.
NO
Go to step23.
 1. Exchange Spark Plug and Ignition Coil

- 1. Exchange the spark plug and the ignition coil with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The ignition coil and spark plug are OK. Go to step 25.
NO
There is a failure in the ignition coil or the spark plug. Go to step 24.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The spark plug has a failure. Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Check MAP Sensor Fluctuation

- 1. Start the engine, and let it idle until the cooling fans cycle twice.
- 1. Turn off all engine and electrical loads (AC, radio, lights, etc.).
- 1. Configure a HIGH-SPEED SNAPSHOT, and select only the MAP SENSOR.
- 1. Begin displaying live vehicle data on a line graph.
- 1. Configure the MAP SENSOR line graph range for 1. 2 V MAX and 0. 8 V MIN.
- 1. Watch the MAP SENSOR fluctuation. See examples below.
| Courtesy of HONDA, U.S.A., INC.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
The intake manifold pressure fluctuation indicates a mechanical engine failure. Go to step 26.
NO
The intake manifold vacuum fluctuation is OK. Go to step 29.
 1. Adjust Valve Clearance

- 1. Adjust the valve clearances - Refer to: Valve, Spring, and Valve Seal Removal and Installation (K24Z7) , Valve Clearance Adjustment (K24Z7) , Valve, Spring, and Valve Seal Removal and Installation (R18Z1) , Valve, Spring, and Valve Seal Removal and Installation (R18A9) , Valve Clearance Adjustment (Except K24Z7) , and inspect the camshaft for excessive wear or grooves.
NOTE: ECT Sensor 1 must read less than 100 ° F (38 ° C) before adjusting the valve clearance. Shortcutting this step may cause incorrect valve clearance and engine misfire.
- 1. Recheck the MAP Sensor fluctuation at idle.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
Adjusting the valve clearance did not correct the excessive MAP sensor fluctuation. Go to step 27.
NO
The excessive MAP sensor fluctuation was caused by incorrect valve clearance. Perform the Repair Confirmation.
 1. Check Cylinder Leakage

- 1. Perform a cylinder leakage test.
- 1. Record your test results in the chart shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Leakage % | Leakage Area
| No. 1 | |
| No. 2 | |
| No. 3 | |
| No. 4 | |
Does any cylinder have leakage greater than 10 %?
YES
If the majority of the air is escaping from exhaust pipe or throttle body, check for tight valve clearance and adjust each cylinder as needed, then retest. Determine the cause of the cylinder leakage and repair the engine as needed, and then perform the Repair Confirmation.
NO
The cylinders are sealing properly. Go to step 28.
 1. Check Cylinder Compression

- 1. Perform a cylinder compression test.
- 1. Record your test results in a chart similar to the one shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Compression
| No. 1 |
| No. 2 |
| No. 3 |
| No. 4 |
| Highest Reading | Lowest Reading | Variation %
| | |
Does the compression vary more than 10 % between cylinders, or does any cylinder read below 150 PSI?
YES
The cylinder leakage is normal, but the engine compression is low. Check for each of the conditions below and repair as needed, and then perform the Repair Confirmation.
Incorrect camshaft timing.
A bent (short) connecting rod.
Worn or damaged camshaft lobes.
NO
The engine compression and cylinder leakage is normal. Go to step 29.
 1. Exchange Fuel Injector

- 1. Exchange the fuel injector from the problem cylinder with one from another cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do the misfire counts stay on the problem cylinder after the fuel injector is swapped?
YES
The fuel injector is OK. All of the failures have been checked for this type of misfire. Go to step 1 in the Failure Confirmation procedure and ensure you selected the correct troubleshooting procedure.
NO
The fuel injector is faulty. Replace the faulty fuel injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step31.
 1. Check Ignition Coil Spark

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the ignition coil from the problem cylinder.
- 1. Connect a commercially available spark tester between the ignition coil and engine ground.
- 1. Start the engine with the ignition coil connected.
Does the ignition coil produce a spark?
YES
The ignition coil wiring is OK. Go to step 39.
NO
There is a problem with the ignition coil wiring. Go to step 32.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Connect a commercially available spark tester between the ignition coil and engine ground.
- 1. Start the engine with the ignition coil connected.
Does the ignition coil produce a spark?
YES
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
There is a problem with the ignition coil circuit. Go to step 33.
 1. Check Ignition Coil Power and Ground

- 1. Disconnect the ignition coil 3P connector from the problem cylinder.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 1 (power) and No. 2 (ground).
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The ignition coil power and ground circuits are OK. Go to step 34.
NO
There is a problem with the ignition coil power or ground circuit. Go to step 38.
 1. Check Ignition Coil Control Short to Power

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 3 (Ignition Control) and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there about 4 V or more?
YES
The ignition coil control circuit is shorted to power. Go to step 35.
NO
The ignition coil control circuit is not shorted to power. Go to step 36.
 1. Isolate Ignition Coil Control Short to Power

- 1. Jump the SCS line with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the ECM/PCM connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between ignition coil 3P connector terminals No. 3 (Ignition Control) and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there about 4 V or more?
YES
There is a short to power on the ignition coil control circuit. Find and repair the failure, and then perform the Repair Confirmation.
NO
The ECM/PCM is malfunctioning. Replace the ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
 1. Check Ignition Coil Control Open Circuit

- 1. Jump the SCS line with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the ECM/PCM connectors.
- 1. Check for continuity between the ignition coil control terminal at the ECM/PCM connector and at the ignition coil 3P connector terminal No. 3.
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The ignition coil control circuit is not open. Go to step 37.
NO
Repair the open circuit between the ignition coil and the ECM/PCM, and then perform the Repair Confirmation.
 1. Check Ignition Coil Control Short to Ground

- 1. Check for continuity between the ignition coil control terminal at the ignition coil 3P connector terminal No. 3 and body ground.
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair the short to ground on the ignition coil control wire, and then perform the Repair Confirmation.
NO
The ignition coil control wiring is OK. Substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
 1. Check Ignition Coil Power Circuit

- 1. Measure the voltage between ignition coil 3P connector terminal No. 1 (power) and a known-good engine ground.
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage between the ignition coil power terminal and engine ground?
YES
The power circuit is OK. Repair the open circuit between the ignition coil (3P connector terminal No. 2) and G101, and then perform the Repair Confirmation.
NO
Repair the open circuit between the ignition coil and the ignition coil relay, and then perform the Repair Confirmation.
 1. Inspect Ignition Coil

- 1. Inspect the ignition coil and spark plug tube for engine oil contamination.
Is the ignition coil covered in engine oil?
YES
Clean the oil from the ignition coil and spark plug tube. Replace the spark plug tube seals in the cylinder head cover - Refer to: Cylinder Head Cover Removal and Installation (K24Z7) , Cylinder Head Cover Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step40.
 1. Exchange Spark Plug and Ignition Coil

- 1. Exchange the spark plug and the ignition coil with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The ignition coil and spark plug are OK. Go to step 42.
NO
There is a failure in the ignition coil or the spark plug. Go to step 41.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The spark plug has a failure. Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Check MAP Sensor Fluctuation

- 1. Start the engine, and let it idle until the cooling fans cycle twice.
- 1. Turn off all engine and electrical loads (AC, radio, lights, etc.).
- 1. Configure a HIGH-SPEED SNAPSHOT, and select only the MAP SENSOR.
- 1. Begin displaying live vehicle data on a line graph.
- 1. Configure the MAP SENSOR line graph range for 1. 2 V MAX and 0. 8 V MIN.
- 1. Watch the MAP SENSOR fluctuation. See examples below.
| Courtesy of HONDA, U.S.A., INC.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
The intake manifold pressure fluctuation indicates a mechanical engine failure. Go to step 43.
NO
The intake manifold vacuum fluctuation is OK. Go to step 46.
 1. Adjust Valve Clearance

- 1. Adjust the valve clearances - Refer to: Valve, Spring, and Valve Seal Removal and Installation (K24Z7) , Valve Clearance Adjustment (K24Z7) . Valve, Spring, and Valve Seal Removal and Installation (R18Z1) . Valve, Spring, and Valve Seal Removal and Installation (R18A9) . Valve Clearance Adjustment (Except K24Z7) , and inspect the camshaft for excessive wear or grooves.
NOTE: ECT Sensor 1 must read less than 100 °F (38 ° C) before adjusting the valve clearance. Shortcutting this step may cause incorrect valve clearance and engine misfire.
- 1. Recheck the MAP Sensor fluctuation at idle.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
Adjusting the valve clearance did not correct the excessive MAP sensor fluctuation. Go to step 44.
NO
The excessive MAP sensor fluctuation was caused by incorrect valve clearance. Perform the Repair Confirmation.
 1. Check Cylinder Leakage

- 1. Perform a cylinder leakage test.
- 1. Record your test results in the chart shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Leakage % | Leakage Area
| No. 1 | |
| No. 2 | |
| No. 3 | |
| No. 4 | |
Does any cylinder have leakage greater than 10 %?
YES
If the majority of the air is escaping from exhaust pipe or throttle body, check for tight valve clearance and adjust each cylinder as needed, then retest. Determine the cause of the cylinder leakage and repair the engine as needed, and then perform the Repair Confirmation.
NO
The cylinders are sealing properly. Go to step 45.
 1. Check Cylinder Compression

- 1. Perform a cylinder compression test.
- 1. Record your test results in a chart similar to the one shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Compression
| No. 1 |
| No. 2 |
| No. 3 |
| No. 4 |
| Highest Reading | Lowest Reading | Variation %
| | |
Does the compression vary more than 10 % between cylinders, or does any cylinder read below 150 PSI?
YES
The cylinder leakage is normal, but the engine compression is low. Check for each of the conditions below and repair as needed, and then perform the Repair Confirmation.
Incorrect camshaft timing.
A bent (short) connecting rod.
Worn or damaged camshaft lobes.
NO
The engine compression and cylinder leakage is normal. Go to step 46.
 1. Exchange Fuel Injector

- 1. Exchange the fuel injector from the problem cylinder with one from another cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do the misfire counts stay on the problem cylinder after the fuel injector is swapped?
YES
The fuel injector is OK. Go to step 47.
NO
The fuel injector is faulty. Replace the faulty fuel injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
 1. CKP Pattern Clear/Pattern Learn

- 1. Select CRANK PATTERN in the ADJUSTMENT MENU with the HDS.
- 1. Select CRANK PATTERN CLEAR, and clear the CKP pattern.
- 1. Select CRANK PATTERN LEARNING with the HDS, and follow the screen prompts.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
NO
Perform the Repair Confirmation.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step49.
 1. Check MAP Sensor Fluctuation

- 1. Start the engine, and let it idle until the cooling fans cycle twice.
- 1. Turn off all engine and electrical loads (AC, radio, lights, etc.).
- 1. Configure a HIGH-SPEED SNAPSHOT, and select only the MAP SENSOR.
- 1. Begin displaying live vehicle data on a line graph.
- 1. Configure the MAP SENSOR line graph range for 1. 2 V MAX and 0. 8 V MIN.
- 1. Watch the MAP SENSOR fluctuation. See examples below.
| Courtesy of HONDA, U.S.A., INC.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
The intake manifold pressure fluctuation indicates a mechanical engine failure. Go to step 50.
NO
The intake manifold vacuum fluctuation is OK. Go to step 53.
 1. Adjust Valve Clearance

- 1. Adjust the valve clearances - Refer to: Valve, Spring, and Valve Seal Removal and Installation (K24Z7) , Valve Clearance Adjustment (K24Z7) , Valve, Spring, and Valve Seal Removal and Installation (R18Z1) , Valve, Spring, and Valve Seal Removal and Installation (R18A9) , Valve Clearance Adjustment (Except K24Z7) , and inspect the camshaft for excessive wear or grooves.
NOTE: ECT Sensor 1 must read less than 100 ° F (38 ° C) before adjusting the valve clearance. Shortcutting this step may cause incorrect valve clearance and engine misfire.
- 1. Recheck the MAP Sensor fluctuation at idle.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine hot?
YES
Adjusting the valve clearance did not correct the excessive MAP sensor fluctuation. Go to step 51.
NO
The excessive MAP sensor fluctuation was caused by incorrect valve clearance. Perform the Repair Confirmation.
 1. Check Cylinder Leakage

- 1. Perform a cylinder leakage test.
- 1. Record your test results in the chart shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Leakage % | Leakage Area
| No. 1 | |
| No. 2 | |
| No. 3 | |
| No. 4 | |
Does any cylinder have leakage greater than 10 %?
YES
If the majority of the air is escaping from exhaust pipe or throttle body, check for tight valve clearance and adjust each cylinder as needed, then retest. Determine the cause of the cylinder leakage and repair the engine as needed, and then perform the Repair Confirmation.
NO
The cylinders are sealing properly. Go to step 52.
 1. Check Cylinder Compression

- 1. Perform a cylinder compression test.
- 1. Record your test results in a chart similar to the one shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Compression
| No. 1 |
| No. 2 |
| No. 3 |
| No. 4 |
| Highest Reading | Lowest Reading | Variation %
| | |
Does the compression vary more than 10 % between cylinders, or does any cylinder read below 150 PSI?
YES
The cylinder leakage is normal, but the engine compression is low. Check for each of the conditions below and repair as needed, and then perform the Repair Confirmation.
Incorrect camshaft timing.
A bent (short) connecting rod.
Worn or damaged camshaft lobes.
NO
The engine compression and cylinder leakage is normal. Go to step 53.
 1. Block-Off EGR Valve

- 1. Loosen the EGR valve nuts enough to lift the valve about 3 mm. Do not remove the nuts.
- 1. Slide a suitable block-off plate the size of a business card between the valve and the exhaust passages.
| Courtesy of HONDA, U.S.A., INC.
- 1. Tighten the EGR valve nuts.
- 1. Operate the vehicle under the conditions where the misfire was duplicated.
Did the misfire counts stop or significantly reduce?
YES
A leaking EGR valve is causing the misfire. Replace the EGR valve , and then perform the Repair Confirmation.
NO
The misfire is not caused by a leaking EGR valve. Go to step 54.
 1. Check Fuel Pressure

- 1. Relieve the fuel pressure. - Refer to: Fuel Pressure Relieving (K24Z7) , Fuel Pressure Relieving (R18Z1) , Fuel Pressure Relieving (R18A9)
- 1. Attach the fuel pressure gauge set and the fuel pressure gauge. - Refer to: Fuel Pressure Test (K24Z7) , Fuel Pressure Test (R18Z1) , Fuel Supply System Leak Inspection (R18A9)
- 1. Locate and record the fuel pressure specification for your vehicle.
| Minimum Pressure | Maximum Pressure
| |
- 1. Start the engine and record the fuel pressure under the conditions the misfire was duplicated.
| Fuel Pressure
|
Is the fuel pressure within specifications?
YES
The fuel pressure is OK. Go to step 55.
NO
Check for pinched or damaged fuel lines. If OK, replace the fuel tank unit - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1) , and then perform the Repair Confirmation.
 1. Check Fuel Contamination

- 1. Remove the fuel tank unit. - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1)
- 1. Inspect for evidence of fuel contamination such as diesel fuel, water, rust, or debris.
NOTE: If the misfire occurred shortly after refueling, the likely cause is poor fuel quality. The MIL DIST (MIL distance) value in the DATA LIST, along with the fuel level, can help you make this determination. Incorrect or contaminated fuel typically affects multiple cylinders. But a problem such as water in the fuel can cause a single cylinder misfire that occurs randomly.
Is there evidence of fuel contamination?
YES
Replace the fuel filter and then drain/refill the fuel tank with a known-good fuel. Perform the Repair Confirmation.
NO
The fuel system has no signs of fuel contamination. Go to step 56.
 1. Check MAF Sensor g/s

- 1. Locate and record the MAF SENSOR g/s specification for your vehicle. This can be found in troubleshooting procedure for DTC P0171.
| Minimum g/s | Maximum g/s | Specified RPM
| | |
- 1. Ensure the vehicle meets these conditions:
Engine coolant temperature (ECT SENSOR 1) above 176 °F (80 °C)
A/T in P or N, M/T in neutral
All electrical loads off
- 1. Monitor the ENGINE SPEED in the DATA LIST with the HDS. Raise and hold the engine speed steady at the specified RPM.
- 1. While holding the RPM steady, check the MAF SENSOR in the DATA LIST.
Is the MAF Sensor g/s value within specifications?
YES
The MAF sensor is reading correctly. Replace the fuel injectors - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
NO
The MAF sensor is not reading correctly. Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step58.
 1. Inspect Ignition Coil

- 1. Inspect the ignition coil and spark plug tube for engine oil contamination.
Is the ignition coil covered in engine oil?
YES
Clean the oil from the ignition coil and spark plug tube. Replace the spark plug tube seals in the cylinder head cover - Refer to: Cylinder Head Cover Removal and Installation (K24Z7) , Cylinder Head Cover Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step59.
 1. Exchange Spark Plug and Ignition Coil

- 1. Exchange the spark plug and the ignition coil with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The ignition coil and spark plug are OK. Go to step 61.
NO
There is a failure in the ignition coil or the spark plug. Go to step 60.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The spark plug has a failure. Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Exchange Fuel Injector

- 1. Exchange the fuel injector from the problem cylinder with one from another cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do the misfire counts stay on the problem cylinder after the fuel injector is swapped?
YES
The fuel injector is OK. Go to step 62.
NO
The fuel injector is faulty. Replace the faulty fuel injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
 1. Inspect VTEC System

- 1. Perform the VTEC rocker arm test. - Refer to: VTEC Rocker Arm Test (K24Z7) , VTEC Rocker Arm Test (Except K24Z7)
Does the engine pass the VTEC rocker arm test?
YES
Go to step63.
NO
Repair the VTEC system, and then perform the Repair Confirmation.
 1. CKP Pattern Clear/Pattern Learn

- 1. Select CRANK PATTERN in the ADJUSTMENT MENU with the HDS.
- 1. Select CRANK PATTERN CLEAR, and clear the CKP pattern.
- 1. Select CRANK PATTERN LEARNING with the HDS, and follow the screen prompts.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
Update the ECM/PCM with the latest software - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) or substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
NO
Perform the Repair Confirmation.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step65.
 1. Check Fuel Pressure

- 1. Relieve the fuel pressure. - Refer to: Fuel Pressure Relieving (K24Z7) , Fuel Pressure Relieving (R18Z1) , Fuel Pressure Relieving (R18A9)
- 1. Attach the fuel pressure gauge set and the fuel pressure gauge. - Refer to: Fuel Pressure Test (K24Z7) , Fuel Pressure Test (R18Z1) , Fuel Supply System Leak Inspection (R18A9)
- 1. Locate and record the fuel pressure specification for your vehicle.
| Minimum Pressure | Maximum Pressure
| |
- 1. Start the engine and record the fuel pressure under the conditions the misfire was duplicated.
| Fuel Pressure
|
Is the fuel pressure within specifications?
YES
The fuel pressure is OK. Go to step 66.
NO
Check for pinched or damaged fuel lines. If OK, replace the fuel tank unit - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1) , and then perform the Repair Confirmation.
 1. Check Fuel Contamination

- 1. Remove the fuel tank unit. - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1)
- 1. Inspect for evidence of fuel contamination such as diesel fuel, water, rust, or debris.
NOTE: If the misfire occurred shortly after refueling, the likely cause is poor fuel quality. The MIL DIST (MIL distance) value in the DATA LIST, along with the fuel level, can help you make this determination. Incorrect or contaminated fuel typically affects multiple cylinders. But a problem such as water in the fuel can cause a single cylinder misfire that occurs randomly.
Is there evidence of fuel contamination?
YES
Replace the fuel filter and then drain/refill the fuel tank with a known-good fuel. Perform the Repair Confirmation.
NO
The fuel system has no signs of fuel contamination. Go to step 67.
 1. Exchange Fuel Injector

- 1. Exchange the fuel injector from the problem cylinder with one from another cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do the misfire counts stay on the problem cylinder after the fuel injector is swapped?
YES
The fuel injector is OK. Go to step 68.
NO
The fuel injector is faulty. Replace the faulty fuel injector - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
 1. Inspect VTEC System

- 1. Perform the VTEC rocker arm test. - Refer to: VTEC Rocker Arm Test (K24Z7) , VTEC Rocker Arm Test (Except K24Z7)
Does the engine pass the VTEC rocker arm test?
YES
Go to step69.
NO
Repair the VTEC system, and then perform the Repair Confirmation.
 1. Check for Exhaust Restrictions

- 1. Reinstall all removed parts, and reconnect all connectors.
- 1. Record the MAP SENSOR value while the engine is off.
- 1. Start the engine and monitor the ENGINE SPEED and MAP SENSOR in the DATA LIST with the HDS.
- 1. Raise the engine speed to 3500 RPM and hold the throttle steady.
Does the MAP SENSOR read near atmospheric pressure (engine off value)?
YES
There is a restriction in the exhaust system. Check for catalyst damage and repair as necessary. Perform the Repair Confirmation.
NO
Go to step70.
 1. CKP Pattern Clear/Pattern Learn

- 1. Select CRANK PATTERN in the ADJUSTMENT MENU with the HDS.
- 1. Select CRANK PATTERN CLEAR, and clear the CKP pattern.
- 1. Select CRANK PATTERN LEARNING with the HDS, and follow the screen prompts.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
Update the ECM/PCM, substitute a known-good ECM/PCM, ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , and then perform the Repair Confirmation.
NO
Perform the Repair Confirmation.
 1. Check MAP Sensor Fluctuation

- 1. Start the engine, and let it idle until the cooling fans cycle twice.
- 1. Turn off all engine and electrical loads (AC, radio, lights, etc.).
- 1. Configure a HIGH-SPEED SNAPSHOT, and select only the MAP SENSOR.
- 1. Begin displaying live vehicle data on a line graph.
- 1. Configure the MAP SENSOR line graph range for 1. 2 V MAX and 0. 8 V MIN.
- 1. Watch the MAP SENSOR fluctuation. See examples below.
| Courtesy of HONDA, U.S.A., INC.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine cold?
YES
The intake manifold pressure fluctuation indicates a mechanical engine failure. Go to step 72.
NO
The intake manifold vacuum fluctuation is OK. Go to step 75.
 1. Adjust Valve Clearance

- 1. Adjust the valve clearances - Refer to: Valve, Spring, and Valve Seal Removal and Installation (K24Z7) , Valve Clearance Adjustment (K24Z7) , Valve, Spring, and Valve Seal Removal and Installation (R18Z1) , Valve, Spring, and Valve Seal Removal and Installation (R18A9) , Valve Clearance Adjustment (Except K24Z7) , and inspect the camshaft for excessive wear or grooves.
NOTE: ECT Sensor 1 must read less than 100 °F (38 °C) before adjusting the valve clearance. Shortcutting this step may cause incorrect valve clearance and engine misfire.
- 1. Recheck the MAP Sensor fluctuation at idle.
Does the MAP SENSOR continuously fluctuate 0. 04 V or more at idle, no loads, engine cold?
YES
Adjusting the valve clearance did not correct the excessive MAP sensor fluctuation. Go to step 73.
NO
The excessive MAP sensor fluctuation was caused by incorrect valve clearance. Perform the Repair Confirmation.
 1. Check Cylinder Leakage

- 1. Perform a cylinder leakage test.
- 1. Record your test results in the chart shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Leakage % | Leakage Area
| No. 1 | |
| No. 2 | |
| No. 3 | |
| No. 4 | |
Does any cylinder have leakage greater than 10 %?
YES
If the majority of the air is escaping from exhaust pipe or throttle body, check for tight valve clearance and adjust each cylinder as needed, then retest. Determine the cause of the cylinder leakage and repair the engine as needed, and then perform the Repair Confirmation.
NO
The cylinders are sealing properly. Go to step 74.
 1. Check Cylinder Compression

- 1. Perform a cylinder compression test.
- 1. Record your test results in a chart similar to the one shown below.
| Courtesy of HONDA, U.S.A., INC.
| | Compression
| No. 1 |
| No. 2 |
| No. 3 |
| No. 4 |
| Highest Reading | Lowest Reading | Variation %
| | |
Does the compression vary more than 10 % between cylinders, or does any cylinder read below 150 PSI?
YES
The cylinder leakage is normal, but the engine compression is low. Check for each of the conditions below and repair as needed, and then perform the Repair Confirmation.
Incorrect camshaft timing.
A bent (short) connecting rod.
Worn or damaged camshaft lobes.
NO
The engine compression and cylinder leakage is normal. Go to step 75.
 1. Check Fuel Injector Leak Down

- 1. Connect the fuel pressure gauge set. - Refer to: Fuel Pressure Test (K24Z7) , Fuel Pressure Test (R18Z1) , Fuel Supply System Leak Inspection (R18A9)
- 1. Ensure that the fuel shut off valve is on the fuel pump side of the pressure gauge.
| Courtesy of HONDA, U.S.A., INC.
- 1. Start the engine and then shut it off.
- 1. Turn the shut off valve 90° to isolate the pressure between the fuel injectors and the fuel pressure gauge.
- 1. Record the initial fuel pressure reading.
- 1. Let the vehicle sit for 10 minutes.
- 1. Check the fuel pressure.
Does the pressure drop more than 10 PSI within 10 minutes?
YES
The fuel injectors are leaking. Replace the fuel injectors - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
NO
The fuel injectors are not leaking. Go to step 76.
 1. Inspect Spark Plug

- 1. Remove the spark plug from the problem cylinder.
- 1. Inspect the spark plug for signs of failure. Also make sure the spark plugs are the correct part number for the vehicle.
Does the spark plug show any signs of failure or is it not the correct part number?
YES
Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
Go to step77.
 1. Exchange Spark Plug and Ignition Coil

- 1. Exchange the spark plug and the ignition coil with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The ignition coil and spark plug are OK. Go to step 79.
NO
There is a failure in the ignition coil or the spark plug. Go to step 78.
 1. Exchange Ignition Coil

- 1. Exchange the ignition coil on the problem cylinder with a normally operating cylinder.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay on the problem cylinder?
YES
The spark plug has a failure. Replace the spark plug - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
NO
The ignition coil has a failure. Replace the ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , and then perform the Repair Confirmation.
 1. Block-Off EGR Valve

- 1. Loosen the EGR valve nuts enough to lift the valve about 3 mm. Do not remove the nuts.
- 1. Slide a suitable block-off plate the size of a business card between the valve and the exhaust passages.
| Courtesy of HONDA, U.S.A., INC.
- 1. Tighten the EGR valve nuts.
- 1. Operate the vehicle under the conditions where the misfire was duplicated.
Did the misfire counts stop or significantly reduce?
YES
A leaking EGR valve is causing the misfire. Replace the EGR valve , and then perform the Repair Confirmation.
NO
The misfire is not caused by a leaking EGR valve. Go to step 80.
 1. Engine Carbon Removal

- 1. Perform a top engine cleaning following the product manufacturer's instructions.
- 1. Drive the vehicle under the conditions when the misfire was duplicated.
- 1. Monitor these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Did the misfire counts stay?
YES
The carbon removal procedure did not correct the misfire. Replace the fuel injectors - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , and then perform the Repair Confirmation.
NO
The carbon removal procedure corrected the misfire. Perform the Repair Confirmation.
Repair Confirmation
- 1. Reinstall all removed parts, and reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Clear the CKP pattern with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Do the CKP pattern learn procedure. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Drive the vehicle under the conditions that you noticed the misfire while monitoring these parameters in the DATA LIST:
CYL1 MISFIRE
CYL2 MISFIRE
CYL3 MISFIRE
CYL4 MISFIRE
Do any of the cylinders display misfire counts?
YES
The engine is still misfiring. Go to step 1 in the Failure Confirmation procedure and ensure that you selected the correct troubleshooting procedure.
NO
The misfire problem has been fixed. Clear the DTC.

## P0326 — page 16648

- Source file: pages/16648.html

DTC P0326: Knock Sensor Malfunction (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Start the engine. Hold the engine speed at 2, 000 RPM or more without load (A/T in P or N, M/T in neutral) for at least 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0326 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the knock sensor and the ECM/PCM.

2. Knock sensor installation check:

- 1. Remove the intake manifold - Refer to: Intake Manifold Removal and Installation (K24Z7) , Intake Manifold Removal and Installation (R18Z1) , Intake Manifold Removal and Installation (R18A9) .
- 1. Check the knock sensor installation.
Is the knock sensor installed correctly?
YES
Replace the knock sensor - Refer to: Knock Sensor Removal and Installation (K24Z7) , Knock Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Install the knock sensor correctly - Refer to: Knock Sensor Removal and Installation (K24Z7) , Knock Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158 °F (70 °C)
A/T in D, M/T in 4th
Engine speed at 1, 500 RPM or more
Drive 5 seconds or more
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0326 indicated?
YES
Check for poor connections or loose terminals at the knock sensor and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0327 — page 16649

- Source file: pages/16649.html

DTC P0327: Knock Sensor Circuit Low Voltage ('12-'13: R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0327 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the knock sensor and the ECM/PCM.

2. Determine possible failure area (knock sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Knock sensor 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0328 indicated?
YES
Replace the knock sensor - Refer to: Knock Sensor Removal and Installation (K24Z7) , Knock Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Shorted wire check (KNOCK line to KSGND line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | KNOCK, KSGND
| Test point 1 | Knock sensor 2P connector No. 1 (BLK)
| Test point 2 | Knock sensor 2P connector No. 2 (WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the KNOCK wire to KSGND wire between the ECM/PCM (C17) and the ECM/PCM (C48), then do the repair verification.
NO
Go to step 4.

4. Shorted wire check (KNOCK line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | KNOCK
| Test point 1 | Knock sensor 2P connector No. 2 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the KNOCK wire between the ECM/PCM (C17) and the knock sensor, then do the repair verification.
NO
The KNOCK wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0327 indicated?
YES
Check for poor connections or loose terminals at the knock sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0328 — page 16652

- Source file: pages/16652.html

DTC P0328: Knock Sensor Circuit High Voltage ('12-'13: R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0328 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the knock sensor and the ECM/PCM.

2. Determine possible failure area (knock sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| Knock sensor 2P connector
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| Test circuit | KNOCK, KSGND
| Test point 1 | Knock sensor 2P connector No. 1 (BLK)
| Test point 2 | Knock sensor 2P connector No. 2 (WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5. 0 V?
YES
Replace the knock sensor - Refer to: Knock Sensor Removal and Installation (K24Z7) , Knock Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Determine possible failure area (KNOCK line, KSGND line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| Test circuit | KNOCK
| Test point 1 | Knock sensor 2P connector No. 2 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5. 0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (KSGND line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Knock sensor 2P connector No. 1 (BLK)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| | Knock sensor 2P connector No. 1: jumped to body ground
| | ECM/PCM connector C (49P): disconnected
| Test circuit | KSGND
| Test point 1 | ECM/PCM connector C (49P) No. 48 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The KSGND wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, Refer to ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the KSGND wire between the ECM/PCM (C48) and the knock sensor, then do the repair verification.

5. Open wire check (KNOCK line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Knock sensor 2P connector No. 2 (WHT)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Knock sensor 2P connector: disconnected
| | Knock sensor 2P connector No. 2: jumped to body ground
| | ECM/PCM connector C (49P): disconnected
| Test circuit | KNOCK
| Test point 1 | ECM/PCM connector C (49P) No. 17 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The KNOCK wire is OK. Update the ECM/PCM, substitute a known-good ECM/PCM, Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the KNOCK wire between the ECM/PCM (C17) and the knock sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0328 indicated?
YES
Check for poor connections or loose terminals at the knock sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0335 — page 16655

- Source file: pages/16655.html

DTC P0335: CKP Sensor No Signal ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0335 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the CKP sensor and the ECM/PCM.

2. Open wire check (IGP (CRANK), IGP (CRK) lines):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| CKP sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CKP sensor 3P connector: disconnected
| Test circuit | IGP(CRANK)
| Test point 1 | CKP sensor 3P connector No. 3 (YEL/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IGP (CRANK)/IGP (CRK) wires are OK. Go to step 3.
NO
Repair an open in the IGP (CRANK)/IGP (CRK) wire between the CKP sensor and PGM-FI main relay 1, then do the repair verification.

3. Determine possible failure area (CRANK P line or CRANK line, LG line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CKP sensor 3P connector: disconnected
| Test circuit | CRANK P
| Test point 1 | CKP sensor 3P connector No. 1 (BLU/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5. 0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (LG line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CKP sensor 3P connector: disconnected
| Test circuit | LG, IGP(CRANK)
| Test point 1 | CKP sensor 3P connector No. 3 (YEL/BLK)
| Test point 2 | CKP sensor 3P connector No. 2 (BRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The LG wire is OK. Replace the CKP sensor - Refer to: CKP Sensor Removal and Installation (K24Z7) , CKP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Repair an open in the LG wire between the CKP sensor and G101, then do the repair verification.

5. Shorted wire check (CRANK P, CRANK lines):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | CKP sensor 3P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | CRANK
| Test point 1 | ECM/PCM connector C (49P) No. 42 (BLU/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the CRANK P/CRANK wire between the ECM/PCM (C42) and the CKP sensor, then do the repair verification.
NO
The CRANK P/CRANK wires are not shorted. Go to step 6.

6. Open wire check (CRANK P, CRANK lines):

- 1. Connect terminals A and B with a jumper wire.
| Terminal A | CKP sensor 3P connector No. 1 (BLU/WHT)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | CKP sensor 3P connector: disconnected
| | CKP sensor 3P connector No. 1: jumped to body ground
| | ECM/PCM connector C (49P): disconnected
| Test circuit | CRANK
| Test point 1 | ECM/PCM connector C (49P) No. 42 (BLU/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The CRANK/CRANK P wires are OK. Update the ECM/PCM, substitute a known-good ECM/PCM, Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the CRANK/CRANK P wire between the ECM/PCM (C42) and the CKP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the CKP pattern clear/CKP pattern learn procedure .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0335 indicated?
YES
Check for poor connections or loose terminals at the CKP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0339 — page 16659

- Source file: pages/16659.html

DTC P0339: CKP Sensor Circuit Intermittent Interruption (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 10 seconds.
- 1. Check the CKP NOISE in the DATA LIST with the HDS.
Are 0 counts indicated?
YES
Go to step 3.
NO
Go to step 2.

2. Problem verification:

- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
- 1. Check the CKP NOISE in the DATA LIST with the HDS.
Are 0 counts indicated?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the CKP sensor and the ECM/PCM.

3. Connectors and terminals condition check:

- 1. Check for poor connections or loose terminals at these locations:
CKP sensor
ECM/PCM
Engine ground
Body ground
Are the connections and terminals OK?
YES
Go to step 4.
NO
Repair the connections or terminals, then do the repair verification.

4. CKP pulse plate visual check:

- 1. Remove the CKP sensor - Refer to: CKP Sensor Removal and Installation (K24Z7) , CKP Sensor Removal and Installation (Except K24Z7) , and check for damage to the CKP pulse plate.
Is the pulse plate damaged?
YES
Replace the CKP pulse plate - Refer to: Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (K24Z7) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18Z1) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18A9) , then do the repair verification.
NO
Replace the CKP sensor - Refer to: CKP Sensor Removal and Installation (K24Z7) , CKP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the CKP pattern clear/CKP pattern learn procedure .
- 1. Start the engine, and let it idle for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0339 indicated?
YES
Check for poor connections or loose terminals at the CKP sensor, and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0351, P0352, P0353, P0354 — page 16663

- Source file: pages/16663.html

DTC P0351: No. 1 Cylinder Ignition Coil Circuit Malfunction, DTC P0352: No. 2 Cylinder Ignition Coil Circuit Malfunction, DTC P0353: No. 3 Cylinder Ignition Coil Circuit Malfunction, DTC P0354: No. 4 Cylinder Ignition Coil Circuit Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0351, P0352, P0353, and/or P0354 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ignition coil and the ECM/PCM.

2. Ignition coil check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Exchange the ignition coil from the problem cylinder with one from another cylinder.
- 1. Start the engine.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0351, P0352, P0353, or P0354 indicated at the exchanged position?
YES
Replace the faulty ignition coil - Refer to: Ignition Coil and Spark Plug Removal and Installation (K24Z7) , Ignition Coil and Spark Plug Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Shorted wire check (IG1 (COIL) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. A16 (15 A)
| Location | Under-hood fuse/relay box
Is the fuses OK?
YES
Go to step 4.
NO
Repair a short in the IG1 (COIL) wire between the ignition coil and the ignition coil relay. Replace the No. A16 (15 A) fuse, then do the repair verification.

4. Relay check:

- 1. Remove the ignition coil relay (A) from the under-hood fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Test the ignition coil relay - Refer to: Power Relay Test ('12-'13: except Natural Gas models) (2012 2013) , Power Relay Test ('12-'13: Natural Gas models) (2012 2013) .
Is the relay OK?
YES
Go to step 5.
NO
Replace the ignition coil relay, then do the repair verification.

5. Open wire check (IG1 (COIL) lines):

- 1. Reinstall the ignition coil relay.
- 1. Disconnect the following connector.
| Ignition coil 3P connector (problem cylinder)
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Ignition coil 3P connector (problem cylinder): disconnected
| Test circuit | IG1(COIL)
| Test point 1 | Ignition coil 3P connector No. 3 (BLK/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IG1 (COIL) wires are OK. Go to step 6.
NO
Repair an open in the IG1 (COIL) wire between the ignition coil and the No. 16 IG COIL (15 A) fuse, then do the repair verification.

6. Open wire check (PG line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG ON (II)
| | Ignition coil 3P connector (problem cylinder): disconnected
| Test circuit | PG
| Test point 1 | Ignition coil 3P connector No. 2 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PG wire is OK. Go to step 7.
NO
Repair an open in the PG wire between the ignition coil and G101, then do the repair verification.

7. Shorted wire check (IGNO lines):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
- 1. Check for continuity between body ground and the appropriate ECM/PCM connector terminal of the problem ignition coil (see table).
| Test condition | IG LOCK (0)
| | Ignition coil 3P connector (problem cylinder): disconnected
| | ECM/PCM connector C (49P): disconnected
| PROBLEM CYLINDER | DTC | ECM/PCM TERMINAL | WIRE COLOR
| No. 1 | P0351 | C25 | YEL/GRN
| No. 2 | P0352 | C26 | BLU/RED
| No. 3 | P0353 | C27 | WHT/BLU
| No. 4 | P0354 | C28 | BRN
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the IGNO wire between the ECM/PCM and the ignition coil, then do the repair verification.
NO
The IGNO wires are not shorted. Go to step 8.

8. Open wire check (IGNO lines):

- 1. Check for continuity between appropriate ignition coil 3P connector terminal No. 1 and the appropriate ECM/PCM connector terminal of the problem ignition coil (see table).
| Test condition | IG LOCK (0)
| | Ignition coil 3P connector (problem cylinder): disconnected
| | ECM/PCM connector C (49P): disconnected
| PROBLEM CYLINDER | DTC | ECM/PCM TERMINAL | WIRE COLOR
| No. 1 | P0351 | C25 | YEL/GRN
| No. 2 | P0352 | C26 | BLU/RED
| No. 3 | P0353 | C27 | WHT/BLU
| No. 4 | P0354 | C28 | BRN
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The IGNO wires are OK. Update the ECM/PCM, substitute a known-good ECM/PCM, Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the IGNO wire between the ECM/PCM and the ignition coil, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0351, P0352, P0353, or P0354 indicated?
YES
Check for poor connections or loose terminals at the ignition coil and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0365 — page 16682

- Source file: pages/16682.html

DTC P0365: CMP Sensor Circuit No Signal ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0365 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the CMP sensor and the ECM/PCM.

2. Open wire check (IGP(TDC) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| CMP sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CMP sensor 3P connector: disconnected
| Test circuit | IGP(TDC)
| Test point 1 | CMP sensor 3P connector No. 1 (YEL/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IGP(TDC) wire is OK. Go to step 3.
NO
Repair an open in the IGP(TDC) wire between the CMP sensor and PGM-FI main relay 1, then do the repair verification.

3. Determine possible failure area (TDC line, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CMP sensor 3P connector: disconnected
| Test circuit | TDC
| Test point 1 | CMP sensor 3P connector No. 3 (WHT/GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (LG line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | CMP sensor 3P connector: disconnected
| Test circuit | IGP(TDC), LG
| Test point 1 | CMP sensor 3P connector No. 1 (YEL/BLK)
| Test point 2 | CMP sensor 3P connector No. 2 (BRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The LG wire is OK. Replace the CMP sensor - Refer to: CMP Sensor B Removal and Installation (K24Z7) , CMP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Repair an open in the LG wire between the CMP sensor and G101, then do the repair verification.

5. Shorted wire check (TDC line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | CMP sensor 3P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | TDC
| Test point 1 | ECM/PCM connector C (49P) No. 41 (WHT/GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TDC wire between the ECM/PCM (C41) and the CMP sensor, then do the repair verification.
NO
The TDC wire is not shorted. Go to step 6.

6. Open wire check (TDC line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | CMP sensor 3P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | TDC
| Test point 1 | ECM/PCM connector C (49P) No. 41 (WHT/GRN)
| Test point 2 | CMP sensor 3P connector No. 3 (WHT/GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TDC wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TDC wire between the ECM/PCM (C41) and the CMP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0365 indicated?
YES
Check for poor connections or loose terminals at the CMP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0369 — page 16686

- Source file: pages/16686.html

DTC P0369: CMP Sensor Circuit Intermittent Interruption (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 10 seconds.
- 1. Check the CMP B NOISE in the DATA LIST with the HDS.
Are 0 counts indicated?
YES
Go to step 3 .
NO
Go to step 2.

2. Problem verification:

- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
- 1. Check the CMP B NOISE in the DATA LIST with the HDS.
Are 0 counts indicated?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the CMP sensor and the ECM/PCM.

3. Connectors and terminals condition check:

- 1. Check for poor connections or loose terminals at these locations:
CMP sensor
ECM/PCM
Engine ground
Body ground
Are the connections and terminals OK?
YES
Go to step 4.
NO
Repair the connections or terminals, then do the repair verification.

4. CMP pulse plate visual check:

- 1. Remove the cylinder head cover, and check for damage to the CMP pulse plate.
Is the pulse plate damaged?
YES
Replace the CMP pulse plate - Refer to: Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (K24Z7) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18Z1) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18A9) , then do the repair verification.
NO
Replace the CMP sensor - Refer to: CMP Sensor B Removal and Installation (K24Z7) , CMP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0369 indicated?
YES
Check for poor connections or loose terminals at the CMP sensor and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0400 — page 16687

- Source file: pages/16687.html

DTC P0400: EGR System Leak Detected (KA KC KL models)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. EGR pipe visual check:

- 1. Check for a loose or damaged EGR pipe.
Is the EGR pipe OK?
YES
Go to step 2.
NO
Reconnect or replace the EGR pipe , then do the repair verification.

2. Problem verification:

- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive the vehicle at 25 mph (40 km/h) or less for 5 minutes
Drive at a steady speed of 26 mph (41 km/h) or more (engine speed between 1, 600-4, 000 rpm) for 11 seconds or more.
- 1. Monitor the OBD STATUS for DTC P0400 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Replace the EGR valve , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for an exhaust gas leak between the EGR pipe and the EGR valve. If the HDS indicates NOT COMPLETED, go to step 2-3 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive the vehicle at 25 mph (40 km/h) or less for 5 minutes
Drive at a steady speed of 26 mph (41 km/h) or more (engine speed between 1, 600-4, 000 rpm) for 11 seconds or more.
- 1. Monitor the OBD STATUS for DTC P0400 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0401 — page 16688

- Source file: pages/16688.html

DTC P0401: EGR Insufficient Flow (KA KC KL models)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. EGR valve check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Do the EGR TEST with the HDS.
Is the result OK?
YES
Go to step 2.
NO
Replace the EGR valve , then do the repair verification.

2. Problem verification:

- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive the vehicle at a steady speed between 15-75 mph (24-120 km/h) for at least 5 seconds
Drive the vehicle at a steady speed between 55-75 mph (90-120 km/h) for at least 10 seconds
During the drive, decelerate (with the throttle fully closed) for 5 seconds
- 1. Monitor the OBD STATUS for DTC P0401 with the HDS.
Does the HDS indicate FAILED?
YES
Clean the intake manifold EGR port with throttle plate cleaner. Also, clean the passage inside the EGR valve with throttle plate cleaner, then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EGR valve and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 2-1 on this step and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive the vehicle at a steady speed between 15-75 mph (24-120 km/h) for at least 5 seconds
Drive the vehicle at a steady speed between 55-75 mph (90-120 km/h) for at least 10 seconds
During the drive, decelerate (with the throttle fully closed) for 5 seconds
- 1. Monitor the OBD STATUS for DTC P0401 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0404 — page 16689

- Source file: pages/16689.html

DTC P0404: EGR Valve Circuit Range/Performance Problem ('12-'13, '14-'15: except R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Do the EGR TEST with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Clean any carbon build-up on the EGR valve with throttle plate cleaner.
NO
Failure is duplicated. Go to step 2.

2. EGR valve position sensor internal circuit check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EGR 5P connector
- 1. At the EGR valve side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| Test circuit | EGR_L, SG2
| Test point 1 | EGR valve 5P connector No. 3 (EGR valve side)
| Test point 2 | EGR valve 5P connector No. 4 (EGR valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there 100 kΩ or more?
YES
Replace the EGR valve , then do the repair verification.
NO
Go to step 3.

3. EGR valve internal circuit check:

- 1. At the EGR valve side, measure the resistance between test point 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| Test circuit | EGR_L, VCC2
| Test point 1 | EGR valve 5P connector No. 2 (EGR valve side)
| Test point 2 | EGR valve 5P connector No. 4 (EGR valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there 100 kΩ or more?
YES
Replace the EGR valve , then do the repair verification.
NO
Go to step 4.

4. Open wire check (PG line):

- 1. Check for continuity between test point 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| Test circuit | PG
| Test point 1 | EGR valve 5P connector No. 1 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PG wire is OK. Go to step 5.
NO
Repair an open in the PG wire between the EGR valve and G101, then do the repair verification.

5. Shorted wire check (E_EGR line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | E_EGR
| Test point 1 | ECM/PCM connector B (49P) No. 2 (BLU/RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the E_EGR wire between the ECM/PCM (B2) and the EGR valve, then do the repair verification.
NO
The E_EGR wire is not shorted. Go to step 6.

6. Open wire check (E_EGR line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | E_EGR
| Test point 1 | ECM/PCM connector B (49P) No. 2 (BLU/RED)
| Test point 2 | EGR valve 5P connector No. 5 (BLU/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The E_EGR wire is OK. Go to step 7.
NO
Repair an open in the E_EGR wire between the ECM/PCM (B2) and the EGR valve, then do the repair verification.

7. EGR port and passage cleaning:

- 1. Remove the EGR valve.
- 1. Clean the intake manifold EGR port with throttle plate cleaner. Also, clean the passage inside the EGR valve with throttle plate cleaner.
- 1. Install the EGR valve.
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Do the EGR TEST with the HDS.
Is the result OK?
YES
The failure is not duplicated. Do the repair verification.
NO
Go to step 8.

8. EGR valve check (substitution):

- 1. Substitute a known-good EGR valve.
- 1. Reconnect the EGR valve 5P connector.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Do the EGR TEST with the HDS.
Is the result OK?
YES
Replace the original EGR valve , then do the repair verification.
NO
The EGR valve is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EGR TEST with the HDS.
- 1. Monitor the OBD STATUS for DTC P0404 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0406 — page 16691

- Source file: pages/16691.html

DTC P0406: EGR Valve Position Sensor Circuit High Voltage ('12-'13, '14-'15: except R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the EGR VLS in the DATA LIST with the HDS.
Is 4.88 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EGR valve and the ECM/PCM.

2. Determine possible failure area (EGR valve position sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EGR 5P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | EGR 5P connector: disconnected
| Test circuit | VCC2, SG2
| Test point 1 | EGR valve 5P connector No. 2 (YEL/BLU)
| Test point 2 | EGR valve 5P connector No. 3 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the EGR valve , then do the repair verification.
NO
Go to step 3.

3. Open wire check (SG2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG ON (II)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SG2
| Test point 1 | ECM/PCM connector B (49P) No. 44 (GRN/YEL)
| Test point 2 | EGR valve 5P connector No. 3 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG2 wire between the ECM/PCM (B44) and the EGR valve, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0406 indicated?
YES
Check for poor connections or loose terminals at the EGR valve and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0420 — page 16694

- Source file: pages/16694.html

DTC P0420: Catalyst System Efficiency Below Threshold (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If some of the DTCs listed below are stored at the same time as DTC P0420, troubleshoot them first, then recheck for DTC P0420.
P0137, P0138: Secondary HO2S (Sensor 2)
P0141: Secondary HO2S (Sensor 2) heater
P0300: Random misfire
P0301, P0302, P0303, P0304: No. 1, No. 2, No. 3, No. 4 cylinder misfire detected
Poor quality fuel may cause this DTC.

1. PGM-FI system check 1:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 5th
Vehicle speed between 45-75 mph (70-120 km/h) for 5 minutes or more with cruise control set
Vehicle speed between 55-75 mph (90-120 km/h) for 10 seconds, then decelerate (with the throttle fully closed). Repeat this three or more times
Maintain the vehicle speed at 55 mph (88 km/h) for 5 minutes or more with cruise control set
- 1. Monitor the OBD STATUS for DTC P0420 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the catalytic converter - Refer to: Warm Up TWC Removal and Installation (K24Z7) , Under-Floor TWC Removal and Installation (K24Z7) , Catalytic Converter Removal and Installation (R18Z1) , Catalytic Converter Removal and Installation (R18A9) , Under-Floor TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , Warm Up TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates OUT OF CONDITION, go to step 2 on this step and recheck. If the HDS indicates PASSED, intermittent failure the system is OK at this time. If the HDS indicates EXECUTING, replace the catalytic converter - Refer to: Warm Up TWC Removal and Installation (K24Z7) , Under-Floor TWC Removal and Installation (K24Z7) , Catalytic Converter Removal and Installation (R18Z1) , Catalytic Converter Removal and Installation (R18A9) , Under-Floor TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , Warm Up TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , then go to step 2.

2. PGM-FI system check 2:

- 1. Continue test-drive until a result comes on.
- 1. Monitor the OBD STATUS for DTC P0420 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the catalytic converter - Refer to: Warm Up TWC Removal and Installation (K24Z7) , Under-Floor TWC Removal and Installation (K24Z7) , Catalytic Converter Removal and Installation (R18Z1) , Catalytic Converter Removal and Installation (R18A9) , Under-Floor TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , Warm Up TWC Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-4 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Drive the vehicle until the result of CAT MONITOR CONDITION in the DATA LIST on HDS indicates OK.
NOTE: Test-drive may to 10 minutes or more.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 5th
Vehicle speed between 45-75 mph (70-120 km/h) for 5 minutes or more with cruise control set
Vehicle speed between 55-75 mph (90-120 km/h) for 10 seconds, then decelerate (with the throttle fully closed). Repeat this three or more times
Maintain the vehicle speed at 55 mph (88 km/h) for 5 minutes or more with cruise control set
- 1. Monitor the OBD STATUS for DTC P0420 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0443 — page 16698

- Source file: pages/16698.html

DTC P0443: EVAP Canister Purge Valve Circuit Malfunction ('13: R18Z1)
Special Tools Required
Vacuum Pump/Gauge, 0-30 inHg Snap-on YA4000A or equivalent, commercially available
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0443 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EVAP canister purge valve and the ECM/PCM.

2. Determine possible failure area (EVAP canister purge valve stuck open failure, EVAP canister purge valve stuck closed failure):

- 1. Turn the ignition switch to LOCK (0), and allow the engine to cool below 140°F (60°C).
- 1. Disconnect the vacuum hose (A) from the EVAP canister purge valve (B) in the engine compartment, then connect a vacuum pump/gauge, 0-30 inHg, to the hose.
| Courtesy of HONDA, U.S.A., INC.
- 1. Start the engine, and let it idle.
Is there vacuum?
YES
The EVAP canister purge valve is stuck open. Go to step 3.
NO
The EVAP canister purge valve is stuck closed. Go to step 5.

3. Determine possible failure area (PC_SOL line, EVAP canister purge valve):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EVAP canister purge valve 2P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister purge valve 2P connector: disconnected
| Test circuit | PC_SOL
| Test point 1 | EVAP canister purge valve 2P connector No. 1 (YEL/BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
There is a problem in the PC_SOL line. Go to step 4.
NO
Replace the EVAP canister purge valve - Refer to: EVAP Canister Purge Valve Removal and Installation (K24Z7) , EVAP Canister Purge Valve Removal and Installation (R18Z1) , then do the repair verification.

4. Shorted wire check (PC_SOL line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister purge valve 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | PC_SOL
| Test point 1 | EVAP canister purge valve 2P connector No. 1 (YEL/BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the PC_SOL wire between the ECM/PCM (B3) and the EVAP canister purge valve, then do the repair verification.
NO
The PC_SOL wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

5. Open wire check (IG1_ACG/IG1(PCS) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EVAP canister purge valve 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | EVAP canister purge valve 2P connector: disconnected
| Test circuit | IG1(PCS)
| Test point 1 | EVAP canister purge valve 2P connector No. 2 (BLK/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IG1_ACG/IG1(PCS) wire is OK. Go to step 6.
NO
Repair an open in the IG1_ACG/IG1(PCS) wire between the EVAP canister purge valve and the No. B2 (10 A) fuse in the under-dash fuse/relay box, then do the repair verification.

6. Open wire check (PC_SOL line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister purge valve 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | PC_SOL
| Test point 1 | ECM/PCM connector B (49P) No. 3 (YEL/BLU)
| Test point 2 | EVAP canister purge valve 2P connector No. 1 (YEL/BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PC_SOL wire is OK. Go to step 7.
NO
Repair an open in the PC_SOL wire between the ECM/PCM (B3) and the EVAP canister purge valve, then do the repair verification.

7. EVAP canister purge valve internal resistance check:

- 1. At the purge valve side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister purge valve 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | IG1(PCS), PC_SOL
| Test point 1 | EVAP canister purge valve 2P connector No. 1 (EVAP canister purge valve side)
| Test point 2 | EVAP canister purge valve 2P connector No. 2 (EVAP canister purge valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 23-26 - at room temperature-
YES
The EVAP canister purge valve is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the EVAP canister purge valve - Refer to: EVAP Canister Purge Valve Removal and Installation (K24Z7) , EVAP Canister Purge Valve Removal and Installation (R18Z1) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Monitor the OBD STATUS for DTC P0443 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
EXECUTING
Keep idling until a result comes on.
OUT OF CONDITION
Keep idling until a result comes on.

## P0451 — page 16701

- Source file: pages/16701.html

DTC P0451: FTP Sensor Circuit Range/Performance Problem (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If DTC P2422 is stored at the same time as DTC P0451, troubleshoot DTC P2422 first, then recheck for DTC P0451.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 1 minute.
- 1. Monitor the OBD STATUS for DTC P0451 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, keep idling until a result comes on.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle for 1 minute.
- 1. Monitor the OBD STATUS for DTC P0451 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0452 — page 16702

- Source file: pages/16702.html

DTC P0452: FTP Sensor Circuit Low Voltage ('12-'13: KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. EVAP system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the fuel fill cap.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is about - 7.3 kPa (- 2.16 inHg, - 55 mmHg), or 0.3 V or less indicated?
YES
Go to step 3.
NO
Go to step 2.

2. Problem verification:

- 1. Install the fuel fill cap.
- 1. Start the engine, and let it idle.
- 1. Monitor the OBD STATUS for DTC P0452 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 3.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, keep idling until a result comes on.

3. Determine possible failure area (PTANK line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| FTP sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is about - 7.3 kPa (- 2.16 inHg, - 55 mmHg), or 0.3 V or less indicated?
YES
Go to step 4.
NO
Go to step 5.

4. Shorted wire check (PTANK line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | FTP sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | PTANK
| Test point 1 | FTP sensor 3P connector No. 2 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the PTANK wire between the ECM/PCM (A37) and the FTP sensor, then do the repair verification.
NO
The PTANK wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

5. Determine possible failure area (FTP sensor, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | FTP sensor 3P connector: disconnected
| Test circuit | VCC6
| Test point 1 | FTP sensor 3P connector No. 3 (RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.
NO
There is a problem in the VCC6 line. Go to step 6.

6. Open wire check (VCC6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | FTP sensor 3P connector No. 3 (RED)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | FTP sensor 3P connector: disconnected
| | FTP sensor 3P connector No. 3: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VCC6
| Test point 1 | ECM/PCM connector A (49P) No. 30 (RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC6 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC6 wire between the ECM/PCM (A30) and the FTP sensor, then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle.
- 1. Monitor the OBD STATUS for DTC P0452 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep idling until a result comes on.

## P0453 — page 16705

- Source file: pages/16705.html

DTC P0453: FTP Sensor Circuit High Voltage ('12-'13: KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. EVAP system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the fuel fill cap.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is about 7.3 kPa (2.16 inHg, 55 mmHg), or 4.7 V or more indicated?
YES
Go to step 3.
NO
Go to step 2.

2. Problem verification:

- 1. Install the fuel fill cap.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P0453 with the HDS.
Does the HDS indicate FAILED?
YES
Failure is not duplicated. Go to step 3.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, keep idling until a result comes on.

3. Determine possible failure area (FTP sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| FTP sensor 3P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | FTP sensor 3P connector No. 2 (WHT)
| Terminal B | FTP sensor 3P connector No. 3 (BLK)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is about 7.3 kPa (2.16 inHg, 55 mmHg), or 4.7 V or more indicated?
YES
The FTP sensor is OK. Go to step 4.
NO
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.

4. Determine possible failure area (PTANK line, SG6 line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | FTP sensor 3P connector: disconnected
| Test circuit | VCC6, SG6
| Test point 1 | FTP sensor 3P connector No. 1 (RED)
| Test point 2 | FTP sensor 3P connector No. 3 (BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 5.
NO
Go to step 6.

5. Open wire check (PTANK line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | FTP sensor 3P connector No. 2 (WHT)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | FTP sensor 3P connector: disconnected
| | FTP sensor 3P connector No. 2: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | PTANK
| Test point 1 | ECM/PCM connector A (49P) No. 37 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PTANK wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the PTANK wire between the ECM/PCM (A37) and the FTP sensor, then do the repair verification.

6. Open wire check (SG6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | FTP sensor 3P connector No. 3 (BLK)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | FTP sensor 3P connector: disconnected
| | FTP sensor 3P connector No. 3: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | SG6
| Test point 1 | ECM/PCM connector A (49P) No. 10 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG6 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG6 wire between the ECM/PCM (A10) and the FTP sensor, then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle.
- 1. Monitor the OBD STATUS for DTC P0453 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep idling until a result comes on.

## P0455, P0456 — page 16709

- Source file: pages/16709.html

DTC P0455: EVAP System Large Leak Detected; DTC P0456: EVAP System Very Small Leak Detected (KA KC KL models R18Z1)
Special Tools Required
Vacuum Pump/Gauge, 0-30 inHg Snap-on YA4000A or equivalent, commercially available
Vacuum/Pressure Gauge, 0-4 inHg 07JAZ-001000B
| Courtesy of HONDA, U.S.A., INC.
The fuel system is designed to allow specified maximum vacuum and pressure conditions. Do not deviate from the vacuum and pressure tests as indicated in these procedures. Excessive pressure/vacuum would damage the EVAP components or cause eventual fuel tank failure.
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
Fresh fuel has a higher volatility that will create greater pressure/vacuum. The best condition for testing is less than a full tank of fresh fuel. If possible, to assist in leak detection, add 1 gallon of fresh fuel to the tank (as long as it will not fill the tank), just before starting these procedures.

1. Fuel fill cap installation check:

- 1. Check the fuel fill cap (the cap must say TIGHTEN TO CLICK). It should turn 1/4 turn after it's tight, then it clicks.
Is the correct fuel fill cap installed and properly tightened?
YES
Go to step 2.
NO
Replace the fuel fill cap or tighten the cap, then do the repair verification.

2. Fuel fill cap seal and fuel fill pipe mating surface visual check:

- 1. Check the fuel fill cap seal (A) and the fuel fill pipe mating surface (B). Verify that the fuel fill cap tether cord (C) is not caught under the cap.
| Courtesy of HONDA, U.S.A., INC.
Is the fuel fill cap seal missing or damaged, is the fuel fill pipe damaged, or is the tether cord caught under the cap?
YES
Replace the fuel fill cap or the fuel fill pipe, then do the repair verification.
NO
The fuel fill cap and the fuel fill pipe are OK. Go to step 3.

3. Fuel tank vapor recirculation tube visual check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Check for a poor connection or damage at the fuel tank vapor recirculation tube.
Is the tube OK?
YES
The tube is OK. Go to step 4.
NO
Replace the fuel tank vapor recirculation tube, or replace the fuel tank, if needed, then do the repair verification.

4. EVAP canister purge valve leak check:

- 1. Disconnect the vacuum hose (purge line) (A) from the EVAP canister purge valve (B) in the engine compartment, then connect a T-fitting (C), a vacuum gauge, and a vacuum pump/gauge, 0-30 inHg, to the vacuum hose as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Slowly apply about 2 kPa (0.6 inHg, 15 mmHg) of vacuum to the hose.
Does it hold vacuum for 1 minute?
YES
Leak not detected at the EVAP canister purge valve. Go to step 5.
NO
Replace the EVAP canister purge valve - Refer to: EVAP Canister Purge Valve Removal and Installation (K24Z7) , EVAP Canister Purge Valve Removal and Installation (R18Z1) , then do the repair verification.

5. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM.
NO
The failure is duplicated. Go to step 6.

6. Determine possible failure area (EVAP canister vent shut valve, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the fuel tank vapor recirculation tube (A) from the EVAP canister (B), and plug the EVAP canister port (C).
| Courtesy of HONDA, U.S.A., INC.
- 1. Remove the T-fitting, and connect a vacuum pump/gauge to the vacuum hose (A) as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Apply vacuum to the hose until the FTP reads 1.90 V (- 0.59 inHg, - 15.1 mmHg).
NOTE: Be careful not to exceed the vacuum. If you do, the FTP sensor can be damaged.
- 1. Monitor the FTP SENSOR in the DATA LIST for 1 minute with the HDS.
Does the voltage increase more than 0.2 V (0.1 inHg, 2.5 mmHg)?
YES
Go to step 7.
NO
Go to step 9.

7. EVAP canister vent shut valve operation check:

- 1. Select EVAP CVS OFF in the INSPECTION MENU with the HDS.
- 1. Disconnect the fresh air hose (A) from the EVAP canister (B), and plug the EVAP canister vent shut valve ports (C).
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP PCS ON in the INSPECTION MENU with the HDS.
- 1. Apply vacuum to the hose until the FTP reads 1.90 V (- 0.59 inHg, - 15.1 mmHg).
NOTE: Be careful not to exceed the vacuum. If you do, the FTP sensor can be damaged.
- 1. Select EVAP PCS OFF in the INSPECTION MENU with the HDS.
- 1. Monitor the FTP SENSOR in the DATA LIST for 1 minute with the HDS.
Does the voltage increase more than 0.2 V (0.1 inHg, 2.5 mmHg)?
YES
The EVAP canister vent shut valve is OK. Go to step 8.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.

8. EVAP canister purge line visual check:Is the line OK?
YES
Replace these parts, then do the repair verification.
FTP sensor O-ring
EVAP canister vent shut valve case and O-ring
EVAP canister
NO
Reconnect or repair the EVAP canister purge line, then do the repair verification.
2. Fuel fill pipe and fuel vapor return pipe visual check:

- 1. Select EVAP CVS OFF in the INSPECTION MENU with the HDS.
- 1. Check for looseness or damage at the fuel fill pipe and the fuel vapor return pipe.
Are the parts OK?
YES
The fuel fill pipe and fuel vapor return pipe are OK. Check the fuel tank unit base gasket and the fuel tank, then do the repair verification.
NO
Repair or replace the damaged parts, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Install the parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Troubleshooting is complete.
NO
Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM, then go to step 1.

## P0457 — page 16711

- Source file: pages/16711.html

DTC P0457: EVAP System Leak Detected/Fuel Fill Cap Loose or Missing (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Fuel fill cap installation check:

- 1. Check the fuel fill cap (the cap must say TIGHTEN TO CLICK). It should turn 1/4 turn after it's tight, then it clicks.
Is the correct fuel fill cap installed and properly tightened?
YES
Go to step 2.
NO
Replace the fuel fill cap or tighten the cap, then do the repair verification.

2. Fuel fill cap seal and fuel fill pipe mating surface visual check:

- 1. Check the fuel fill cap seal (A) and the fuel fill pipe mating surface (B). Verify that the fuel fill cap tether cord (C) is not caught under the cap.
| Courtesy of HONDA, U.S.A., INC.
Is the fuel fill cap seal missing or damaged, is the fuel fill pipe damaged, or is the tether cord caught under the cap?
YES
Replace the fuel fill cap or the fuel fill pipe, then do the repair verification.
NO
The fuel fill cap and the fuel fill pipe are OK. Go to step 3.

3. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister vent shut valve, and the ECM/PCM.
NO
The failure is duplicated. Go to step 4.

4. EVAP canister vent shut valve operation check:

- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the EVAP canister vent shut valve from the EVAP canister - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) .
- 1. Connect the 2P connector to the EVAP canister vent shut valve.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Check the EVAP canister vent shut valve (A) operation.
| Courtesy of HONDA, U.S.A., INC.
Is the valve free of corrosion and does it operate properly?
YES
The EVAP canister vent shut valve is OK. Go to step 5.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.

5. EVAP canister purge line check:

- 1. Check the routing of the EVAP canister purge line.
Is there any damage or blockage?
YES
Remove the blockage and replace the EVAP canister purge line if needed, then do the repair verification.
NO
Do the DTC P0455 troubleshooting - Refer to: DTC P0455: EVAP System Large Leak Detected (KA KC KL models K24Z7) , DTC P0455: EVAP System Large Leak Detected (KA KC KL models R18Z1) .
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Install the parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Troubleshooting is complete.
NO
Check for poor connections or loose terminals at the FTP sensor, the EVAP canister vent shut valve, and the ECM/PCM, then go to step 1.

## P0461 — page 16713

- Source file: pages/16713.html

DTC P0461: Fuel Level Sensor (Fuel Gauge Sending Unit) Circuit Range/Performance Problem (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
Because it requires 124 miles (200 km) of driving without refueling to complete this diagnosis, DTC P0461 cannot be duplicated during this troubleshooting.

1. Fuel gauge sending unit performance check:

- 1. Test the fuel gauge sending unit.
Is the fuel gauge sending unit OK?
YES
Check for poor connections or loose terminals at the fuel gauge sending unit and the gauge control module (tach).
NO
Replace the fuel gauge sending unit - Refer to: Fuel Pump/Fuel Gauge Sending Unit Removal and Installation (K24Z7) , Fuel Pump/Fuel Gauge Sending Unit Removal and Installation (R18Z1) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are any Pending or Confirmed DTCs indicated?
YES
Go to the indicated DTCs Troubleshooting.
NO
Troubleshooting is complete.

## P0462 — page 16715

- Source file: pages/16715.html

DTC P0462: Fuel Level Sensor (Fuel Gauge Sending Unit) Circuit Low Voltage (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Remove the access panel from the floor.
- 1. Clear the DTC with the HDS.
- 1. Wait 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0462 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the gauge control module (tach) and the fuel gauge sending unit.

2. Fuel gauge sending unit check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Fuel tank unit 4P connector
- 1. Turn the ignition switch to ON (II).
- 1. Wait 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0463 indicated?
YES
Replace the fuel gauge sending unit - Refer to: Fuel Pump/Fuel Gauge Sending Unit Removal and Installation (K24Z7) , Fuel Pump/Fuel Gauge Sending Unit Removal and Installation (R18Z1) , then do the repair verification.
NO
The fuel gauge sending unit is OK. Go to step 3.

3. Determine possible failure area (FUEL_GAGE+ line, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| Test circuit | FUEL_GAGE+
| Test point 1 | Fuel tank unit 4P connector No. 2 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 5.
NO
Go to step 4.

4. Shorted wire check (FUEL_GAGE+ line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the gauge control module (tach). - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) ('13) , Gauge Control Module (Tach) Removal and Installation (Gasoline) ('13) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) ('13) , Gauge Control Module (Speedo) Removal and Installation ('14-'15) , Gauge Control Module (Tach) Removal and Installation ('14-'15) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('14-'15)
- 1. Disconnect the following connector.
| Gauge control module (tach) 32P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| Test circuit | FUEL_GAGE+
| Test point 1 | Fuel tank unit 4P connector No. 2 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the FUEL_GAGE+ wire between the gauge control module (tach) and the fuel tank unit, then do the repair verification.
NO
The FUEL_GAGE+ wire is OK. Replace the gauge control module (tach) - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) ('13) , Gauge Control Module (Tach) Removal and Installation (Gasoline) ('13) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) ('13) , Gauge Control Module (Speedo) Removal and Installation ('14-'15) , Gauge Control Module (Tach) Removal and Installation ('14-'15) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('14-'15) , then do the repair verification.

5. Gauge control module (tach) check:

- 1. Reconnect the following connector.
| Gauge control module (tach) 32P connector
- 1. Remove the fuel tank unit. - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1)
- 1. Connect the following connector.
| Fuel tank unit 4P connector
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Set the float (A) to the E position.
| Courtesy of HONDA, U.S.A., INC.
- 1. Check the fuel gauge.
Does the gauge move to the empty position?
YES
The gauge control module (tach) is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the gauge control module (tach) - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) ('13) , Gauge Control Module (Tach) Removal and Installation (Gasoline) ('13) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) ('13) , Gauge Control Module (Speedo) Removal and Installation ('14-'15) , Gauge Control Module (Tach) Removal and Installation ('14-'15) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('14-'15) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reinstall all removed parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0462 indicated?
YES
Check for poor connections or loose terminals at the gauge control module (tach) and the fuel gauge sending unit. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0463 — page 16717

- Source file: pages/16717.html

DTC P0463: Fuel Level Sensor (Fuel Gauge Sending Unit) Circuit High Voltage (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Wait 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0463 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the gauge control module (tach) and the fuel gauge sending unit.

2. Determine possible failure area (FUEL_GAGE+ line or FUEL_GAGE- line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the access panel from the floor.
- 1. Disconnect the following connector.
| Fuel tank unit 4P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Fuel tank unit 4P connector: disconnected
| Test circuit | FUEL_GAGE+, FUEL_GAGE-
| Test point 1 | Fuel tank unit 4P connector No. 2 (PUR)
| Test point 2 | Fuel tank unit 4P connector No. 4 (ORN)
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 5.
NO
Go to step 3.

3. Open wire check (FUEL_GAGE- line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Fuel tank unit 4P connector No. 4 (ORN)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Remove the gauge control module (tach). - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) ('13) , Gauge Control Module (Tach) Removal and Installation (Gasoline) ('13) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) ('13) , Gauge Control Module (Speedo) Removal and Installation ('14-'15) , Gauge Control Module (Tach) Removal and Installation ('14-'15) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('14-'15)
- 1. Disconnect the following connector.
| Gauge control module (tach) 32P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Fuel tank unit 4P connector: disconnected
| | Fuel tank unit 4P connector No. 4: jumped to body ground
| | Gauge control module (tach) 32P connector: disconnected
| Test circuit | FUEL_GAGE-
| Test point 1 | Gauge control module (tach) 32P connector No. 21 (LT BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The FUEL_GAGE- wire is OK. Go to step 4.
NO
Repair an open in the FUEL_GAGE- wire between the gauge control module (tach) and the fuel tank unit, then do the repair verification.

4. Open wire check (FUEL_GAGE+ line):

- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Fuel tank unit 4P connector No. 2 (PUR)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Fuel tank unit 4P connector: disconnected
| | Fuel tank unit 4P connector No. 2: jumped to body ground
| | Gauge control module (tach) 32P connector: disconnected
| Test circuit | FUEL_GAGE+
| Test point 1 | Gauge control module (tach) 32P connector No. 22 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The FUEL_GAGE+ wire is OK. Go to step 5.
NO
Repair an open in the FUEL_GAGE+ wire between the gauge control module (tach) and the fuel tank unit, then do the repair verification.

5. Fuel gauge sending unit performance check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire from the fuel tank unit 4P connector.
- 1. Remove the fuel tank unit. - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1)
- 1. Test the fuel gauge sending unit. - Refer to: Fuel Gauge Sending Unit Test (K24Z7) , Fuel Gauge Sending Unit Test (R18Z1)
Is the fuel gauge sending unit OK?
YES
The fuel gauge sending unit is OK. Go to step 6.
NO
Repair the fuel gauge sending unit, then do the repair verification.

6. Gauge control module (tach) check:

- 1. Connect the following connectors.
| Fuel tank unit 4P connector
| Gauge control module (tach) 32P connector
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Set the float (A) to the F position.
| Courtesy of HONDA, U.S.A., INC.
- 1. Check the fuel gauge.
Does the gauge move to the full position?
YES
The gauge control module (tach) is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the gauge control module (tach) - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) ('13) , Gauge Control Module (Tach) Removal and Installation (Gasoline) ('13) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) ('13) , Gauge Control Module (Speedo) Removal and Installation ('14-'15) , Gauge Control Module (Tach) Removal and Installation ('14-'15) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module ('14-'15) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reinstall all removed parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0463 indicated?
YES
Check for poor connections or loose terminals at the gauge control module (tach) and the fuel gauge sending unit. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0496 — page 16719

- Source file: pages/16719.html

DTC P0496: EVAP System High Purge Flow Detected (KA KC KL models R18Z1)
Special Tools Required
Vacuum Pump/Gauge, 0-30 inHg Snap-on YA4000A or equivalent, commercially available
Vacuum/Pressure Gauge, 0-4 inHg 07JAZ-001000B
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the EVAP FUNCTION TEST with the HDS.
NOTE: If the test could not be completed because of another DTC, do the troubleshooting for that DTC.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM.
NO
Go to step 2.

2. EVAP canister purge valve open stuck check:

- 1. Select EVAP PCS OFF in the INSPECTION MENU with the HDS.
- 1. Disconnect the vacuum hose (purge line) (A) from the EVAP canister purge valve (B) in the engine compartment, then connect a T-fitting (C), a vacuum gauge, and a vacuum pump/gauge, 0-30 inHg, to the vacuum hose as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Slowly apply about 2 kPa (0.6 inHg, 15 mmHg) of vacuum to the hose.
Does it hold vacuum?
YES
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.
NO
Replace the EVAP canister purge valve - Refer to: EVAP Canister Purge Valve Removal and Installation (K24Z7) , EVAP Canister Purge Valve Removal and Installation (R18Z1) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Troubleshooting is complete.
NO
Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM. then go to step 1.

## P0497 — page 16721

- Source file: pages/16721.html

DTC P0497: EVAP System Low Purge Flow Detected (KA KC KL models R18Z1)
Special Tools Required
Vacuum Pump/Gauge, 0-30 inHg Snap-on YA4000A or equivalent, commercially available
Vacuum/Pressure Gauge, 0-4 inHg 07JAZ-001000B
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM.
NO
The failure is duplicated. Go to step 2.

2. EVAP canister purge line visual check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Check for a poor connection, a blockage, or damage at the EVAP canister purge line between throttle body and the EVAP canister.
Is the line OK?
YES
Go to step 3.
NO
Reconnect or repair the EVAP canister purge line, then do the repair verification.

3. Determine possible failure area (EVAP canister purge valve, others):

- 1. Disconnect the vacuum hose (purge line) (A) from the EVAP canister purge valve (B) in the engine compartment, then connect a T-fitting (C), a vacuum gauge, and a vacuum pump/gauge, 0-30 inHg, to the vacuum hose as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP PCS ON in the INSPECTION MENU with the HDS.
- 1. Slowly apply about 2 kPa (0.6 inHg, 15 mmHg) of vacuum to the hose.
Does it hold vacuum?
YES
Replace the EVAP canister purge valve - Refer to: EVAP Canister Purge Valve Removal and Installation (K24Z7) , EVAP Canister Purge Valve Removal and Installation (R18Z1) , then do the repair verification.
NO
Go to step 4.

4. Determine possible failure area (EVAP canister purge line, others):

- 1. Reconnect the vacuum hose to the EVAP canister purge valve.
- 1. Disconnect the vacuum hose (A) from the purge line (at the EVAP canister side), then connect the T-fitting (C), the vacuum gauge, and the vacuum pump/gauge to the hose as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Select EVAP PCS ON in the INSPECTION MENU with the HDS.
- 1. Apply about 2 kPa (0.6 inHg, 15 mmHg) of vacuum to the purge valve.
Does it hold vacuum?
YES
Check for a blockage in the purge line between the EVAP canister purge valve and the EVAP canister, then do the repair verification.
NO
Go to step 5.

5. FTP sensor performance check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the FTP sensor with its connector connected.
- 1. Connect the the T-fitting (A), the vacuum pump/gauge, and the vacuum gauge to the FTP sensor (B) as shown in llustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check and record the FTP SENSOR reading in the DATA LIST with the HDS.
- 1. Slowly apply about 1.3 kPa (0.4 inHg, 10 mmHg) of vacuum to the hose.
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Does the value change?
YES
Go to step 6.
NO
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.

6. EVAP canister purge line check:

- 1. Reconnect the vacuum hoses to the EVAP canister purge line (EVAP canister side), and reinstall the FTP sensor.
- 1. Disconnect the vacuum hose (purge line) (A) from the EVAP canister purge valve (B) in the engine compartment, then connect the T-fitting (C), the vacuum gauge, and the vacuum pump/gauge to the vacuum hose as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Slowly apply about 2 kPa (0.6 inHg, 15 mmHg) of vacuum to the hose.
Does the hose hold vacuum?
YES
Check for a blockage at the EVAP canister port, then do the repair verification.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Install the parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EVAP FUNCTION TEST with the HDS.
Is the result OK?
YES
Troubleshooting is complete.
NO
Check for poor connections or loose terminals at the FTP sensor, the EVAP canister purge valve, the EVAP canister vent shut valve, and the ECM/PCM, then go to step 1.

## P0498 — page 16722

- Source file: pages/16722.html

DTC P0498: EVAP Canister Vent Shut Valve Circuit Low Voltage ('12-'13: KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. EVAP system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0498 indicated?
YES
Go to step 3.
NO
Go to step 2.

2. Problem verification:

- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0498 indicated?
YES
The failure is duplicated. Go to step 3.
NO
The EVAP system is OK. Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EVAP canister vent shut valve and the ECM/PCM.

3. Open wire check (FI_SUB_RLY_OUT line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EVAP canister vent shut valve 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | EVAP canister vent shut valve 2P connector: disconnected
| Test circuit | FI_SUB_RLY_OUT
| Test point 1 | EVAP canister vent shut valve 2P connector No. 1 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The FI_SUB_RLY_OUT wire is OK. Go to step 4.
NO
Repair an open in the FI_SUB_RLY_OUT wire between the EVAP canister vent shut valve and the PGM-FI subrelay, then do the repair verification.

4. EVAP canister purge valve internal resistance check:

- 1. Turn the ignition switch to LOCK (0).
- 1. At the vent shut valve side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister vent shut valve 2P connector: disconnected
| Test circuit | VSV, FI_SUB_RLY_OUT
| Test point 1 | EVAP canister vent shut valve 2P connector No. 1 (EVAP canister vent shut valve side)
| Test point 2 | EVAP canister vent shut valve 2P connector No. 2 (EVAP canister vent shut valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 25 -30 Ω at room temperature?
YES
The EVAP canister vent shut valve is OK. Go to step 5.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.

5. Shorted wire check (VSV line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister vent shut valve 2P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VSV
| Test point 1 | ECM/PCM connector A (49P) No. 32 (LT GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VSV wire between the EVAP canister vent shut valve and the ECM/PCM (A32), then do the repair verification.
NO
The VSV wire is not shorted. Go to step 6.

6. Open wire check (VSV line):

- 1. Connect terminals A and B with a jumper wire.
| Terminal A | EVAP canister vent shut valve 2P connector No. 2 (LT GRN)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EVAP canister vent shut valve 2P connector: disconnected
| | EVAP canister vent shut valve 2P connector No. 2: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VSV
| Test point 1 | ECM/PCM connector A (49P) No. 32 (LT GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VSV wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VSV wire between the EVAP canister vent shut valve and the ECM/PCM (A32), then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Monitor the OBD STATUS for DTC P0498 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0499 — page 16726

- Source file: pages/16726.html

DTC P0499: EVAP Canister Vent Shut Valve Circuit High Voltage (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0499 indicated?
YES
Failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EVAP canister vent shut valve and the ECM/PCM.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Monitor the OBD STATUS for DTC P0499 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0506 — page 16730

- Source file: pages/16730.html

DTC P0506: Idle Control System RPM Lower Than Expected (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for these conditions in the DATA LIST with the HDS:
ECT SENSOR 1 above 158°F (70°C)
IAT SENSOR above 32°F (0°C)
VEHICLE SPEED is 0 mph (0 km/h)
ST FUEL TRIM between 0.69 and 1.47
FSS is CLOSED
- 1. Monitor the OBD STATUS for DTC P0506 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 2.
NO
If the HDS indicates PASSED, go to step 3. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-4 and recheck.

2. Throttle body visual check:

- 1. Remove the intake air duct from the throttle body. - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7)
- 1. Check for dirt, carbon, or damage in the throttle bore.
Is there dirt, carbon, or damage in the throttle bore?
YES
If there is dirt or carbon, clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) . Also check for damage to the air cleaner element, then do the repair verification. If there is damage in the throttle bore, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
The throttle body is OK. Check the A/C system, then go to step 4.

3. Throttle body visual check:

- 1. Remove the intake air duct from the throttle body. - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Cleaning (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Cleaning (Except K24Z7) , Throttle Body Disassembly and Reassembly (Except K24Z7)
- 1. Check for dirt, carbon, or damage in the throttle bore.
Is there dirt, carbon, or damage in the throttle bore?
YES
If there is dirt or carbon, clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) . Also check for damage to the air cleaner element, then do the repair verification. If there is damage in the throttle bore, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
The throttle body is OK. Go to step 4.

4. Idle control system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for these conditions in the DATA LIST with the HDS:
ECT SENSOR 1 above 158°F (70°C)
IAT SENSOR above 32°F (0°C)
VEHICLE SPEED is 0 mph (0 km/h)
ST FUEL TRIM between 0.69 and 1.47
FSS is CLOSED
NOTE: Recheck with different load conditions (turn on the headlights, the blower motor, the rear window defogger, and/or the A/C, change the gear position, etc.).
- 1. Monitor the OBD STATUS for DTC P0506 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED?
YES
Intermittent failure, the system is OK at this time.
NO
If the HDS indicates FAILED, check the A/C system, then go to step 1 and recheck. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 4-4.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for these conditions in the DATA LIST with the HDS:
ECT SENSOR 1 above 158°F (70°C)
IAT SENSOR above 32°F (0°C)
VEHICLE SPEED is 0 mph (0 km/h)
ST FUEL TRIM between 0.69 and 1.47
FSS is CLOSED
- 1. Monitor the OBD STATUS for DTC P0506 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep idling until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0507 — page 16732

- Source file: pages/16732.html

DTC P0507: Idle Control System RPM Higher Than Expected (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P0507 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 2.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, recheck with different load conditions (electrical, A/C, gear position, etc.), then go to step 1-3.

2. Vacuum leak check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Check for vacuum leaks at these parts:
PCV valve
PCV hose
EVAP canister purge valve
Throttle body
Intake manifold
Brake booster and hose
Are there any leaks?
YES
Repair or replace the leaking part(s), then do the repair verification.
NO
Recheck with different load conditions (turn on the headlights, blower motor, or A/C; change the gear position, etc.), then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P0507 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P050A — page 16734

- Source file: pages/16734.html

DTC P050A: Cold Start Idle Air Control System Performance Problem (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are any Pending or Confirmed DTCs other than P050A indicated?
YES
Go to the indicated DTC's troubleshooting.
NO
Go to step 2.

2. Intake air duct visual check:

- 1. Check for a poor connection or blockage at the intake air duct.
Is it OK?
YES
Go to step 3.
NO
Reconnect or repair the intake air duct, then do the repair verification.

3. Air cleaner housing visual check:

- 1. Check for a poor connection or blockage at the air cleaner housing.
Is OK?
YES
Go to step 4.
NO
Replace the air cleaner housing - Refer to: Air Cleaner Removal and Installation (K24Z7) , Air Cleaner Removal and Installation (Except K24Z7) , then do the repair verification.

4. Air cleaner element visual check:

- 1. Check for dirt or debris on the air cleaner element.
Is it dirty?
YES
Replace the air cleaner element - Refer to: Air Cleaner Element Removal, Installation, and Inspection (K24Z7) , Air Cleaner Element Removal, Installation, and Inspection (Except K24Z7) , then do the repair verification.
NO
Go to step 5.

5. Problem verification:

- 1. Let the engine cool until the value of ECT SENSOR 1 is 122°F (50°C) or less.
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 10 seconds or more.
- 1. Monitor the OBD STATUS for DTC P050A with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 6.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body, the MAF sensor/IAT sensor, and the ECM/PCM. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 5-1. and recheck.

6. Throttle body test:

- 1. Do the ETCS TEST with the HDS.
Is the THROTTLE ACTUATOR CONTROL VALVE normal?
YES
Go to step 7.
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.

7. MAF sensor performance check:

- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. While holding the engine speed at 2, 500 rpm for 30 seconds, check the MAF SENSOR in the DATA LIST with the HDS.
Is there about 5.3-6.5 g/s
YES
The MAF sensor is OK. Go to step 8.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

8. IAT sensor performance check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the engine to cool to ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of the IAT SENSOR in the DATA LIST with the HDS.
- 1. Compare the value of the IAT SENSOR with the ambient temperature.
Does the value of the IAT SENSOR differ 5.4°F (3°C) or more from the ambient temperature?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
The IAT sensor is OK. Check for dirt, carbon, or damage in the throttle bore. If there is dirt or carbon, clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , then do the repair verification. If there is damage in the throttle bore, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Let the engine cool until the value of ECT SENSOR 1 is 122°F (50°C) or less.
- 1. Start the engine, and let it idle for 10 seconds or more.
- 1. Monitor the OBD STATUS for DTC P050A in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P050B — page 16736

- Source file: pages/16736.html

DTC P050B: Cold Start Ignition Timing Control System Performance Problem (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are any Pending or Confirmed DTCs other than P050B indicated?
YES
Go to the indicated DTC's troubleshooting.
NO
Only DTC P050B is indicated. Go to step 2.

2. Intake air duct visual check:

- 1. Check for a poor connection or blockage at the intake air duct.
Is OK?
YES
Go to step 3.
NO
Reconnect or repair the intake air duct, then do the repair verification.

3. Air cleaner housing visual check:

- 1. Check for a poor connection or blockage at the air cleaner housing.
Is OK?
YES
Go to step 4.
NO
Replace the air cleaner housing - Refer to: Air Cleaner Removal and Installation (K24Z7) , Air Cleaner Removal and Installation (Except K24Z7) , then do the repair verification.

4. Air cleaner element visual check:

- 1. Check for dirt or debris on the air cleaner element.
Is dirty?
YES
Replace the air cleaner element - Refer to: Air Cleaner Element Removal, Installation, and Inspection (K24Z7) , Air Cleaner Element Removal, Installation, and Inspection (Except K24Z7) , then do the repair verification.
NO
Go to step 5.

5. Problem verification:

- 1. Let the engine cool until the value of ECT SENSOR 1 is 122°F (50°C) or less.
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 10 seconds or more.
- 1. Monitor the OBD STATUS for DTC P050B with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 6.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the CKP sensor, the throttle body, the MAF sensor/IAT sensor, ECT sensor 1, ECT sensor 2, and the ECM/PCM. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 5-1. and recheck.

6. Determine possible failure area (CKP sensor, others):

- 1. Check the ignition timing - Refer to: Ignition Timing Inspection (K24Z7) , Ignition Timing Inspection (Except K24Z7) .
Is the ignition timing OK?
YES
Go to step 8.
NO
Go to step 7.

7. CKP sensor and CKP pulse plate visual check:

- 1. Check for damage at the CKP sensor and the CKP pulse plate.
Is the CKP sensor or the CKP pulse plate damaged?
YES
Replace the CKP sensor - Refer to: CKP Sensor Removal and Installation (K24Z7) , CKP Sensor Removal and Installation (Except K24Z7) and/or the CKP pulse plate - Refer to: Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (K24Z7) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18Z1) , Crankshaft and CKP Pulse Plate Removal, Installation, and Inspection (R18A9) , then do the repair verification.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

8. Throttle body test:

- 1. Do the ETCS TEST with the HDS.
Is the THROTTLE ACTUATOR CONTROL VALVE normal?
YES
The throttle body is OK. Go to step 9.
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.

9. MAF sensor performance check:

- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. While holding the engine speed at 2, 500 rpm for 30 seconds, check the MAF SENSOR in the DATA LIST with the HDS.
Is there about 5.3-6.5 g/s-
YES
The MAF sensor is OK. Go to step 10.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
10.ECT sensor 1 and ECT sensor 2 performance check:
- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the engine to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of ECT SENSOR 1 and ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Compare the value of ECT SENSOR 1 and the value of ECT SENSOR 2 to the ambient temperature individually.
Does either sensor differ more than 5.4°F (3°C) from the ambient temperature?
YES
Replace the sensor that differed more than 5.4°F (3°C) from the ambient temperature, then do the repair verification.
NO
The ECT sensors are OK. Check and repair any problems with the following items. Repair or replace them if needed, then do the repair verification. If every item is OK, Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Engine compression and cylinder leakdown
Valve pause system
Engine oil
A/C system
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Let the engine cool until the value of ECT SENSOR 1 is 122°F (50°C) or less.
- 1. Start the engine, and let it idle for 10 seconds or more.
- 1. Monitor the OBD STATUS for DTC P050B in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0532 — page 16737

- Source file: pages/16737.html

DTC P0532: A/C Pressure Sensor Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Start the engine, and let it idle.
- 1. Turn the blower switch on.
- 1. Turn the A/C switch on.
- 1. Check the A/C PRESSURE SENSOR in the DATA LIST with the HDS.
Is there about 0.24 V or less?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/C pressure sensor and the ECM/PCM.

2. Determine possible failure area (PD_SENSOR line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| A/C pressure sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the A/C PRESSURE SENSOR in the DATA LIST with the HDS.
Is there about 0.24 V or less?
YES
Go to step 5.
NO
Go to step 3.

3. Determine possible failure area (A/C pressure sensor, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | A/C pressure sensor 3P connector: disconnected
| Test circuit | VCC6, SG6
| Test point 1 | A/C pressure sensor 3P connector No. 1 (BLK)
| Test point 2 | A/C pressure sensor 3P connector No. 3 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the A/C pressure sensor - Refer to: A/C Line Removal and Installation (K24Z7) , A/C Line Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. Open wire check (VCC6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/C pressure sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VCC6
| Test point 1 | ECM/PCM connector A (49P) No. 30 (RED)
| Test point 2 | A/C pressure sensor 3P connector No. 3 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC6 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC6 wire between the ECM/PCM (A30) and the A/C pressure sensor, then do the repair verification.

5. Shorted wire check (PD_SENSOR line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/C pressure sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | PD_SENSOR
| Test point 1 | A/C pressure sensor 3P connector No. 2 (LT GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the PD_SENSOR wire between the ECM/PCM (A27) and the A/C pressure sensor, then do the repair verification.
NO
The PD_SENSOR wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle.
- 1. Turn the blower switch on.
- 1. Turn the A/C switch on.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0532 indicated?
YES
Check for poor connections or loose terminals at the A/C pressure sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0533 — page 16740

- Source file: pages/16740.html

DTC P0533: A/C Pressure Sensor Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Start the engine, and let it idle.
- 1. Turn the blower switch on.
- 1. Turn the A/C switch on.
- 1. Check the A/C PRESSURE SENSOR in the DATA LIST with the HDS.
Is there about 4.75 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/C pressure sensor and the ECM/PCM.

2. Determine possible failure area (A/C pressure sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| A/C pressure sensor 3P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | A/C pressure sensor 3P connector No. 1 (BLK)
| Terminal B | A/C pressure sensor 3P connector No. 2 (LT GRN)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the A/C PRESSURE SENSOR in the DATA LIST with the HDS.
Is there about 4.75 V or more?
YES
Go to step 3.
NO
Replace the A/C pressure sensor - Refer to: A/C Line Removal and Installation (K24Z7) , A/C Line Removal and Installation (Except K24Z7) , then do the repair verification.

3. Determine possible failure area (PD_SENSOR line, SG6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | A/C pressure sensor 3P connector: disconnected
| Test circuit | SG6, VCC6
| Test point 1 | A/C pressure sensor 3P connector No. 1 (BLK)
| Test point 2 | A/C pressure sensor 3P connector No. 3 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (PD_SENSOR line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/C pressure sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | PD_SENSOR
| Test point 1 | ECM/PCM connector A (49P) No. 27 (LT GRN)
| Test point 2 | A/C pressure sensor 3P connector No. 2 (LT GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PD_SENSOR wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the PD_SENSOR wire between the ECM/PCM (A27) and the A/C pressure sensor, then do the repair verification.

5. Open wire check (SG6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/C pressure sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | SG6
| Test point 1 | ECM/PCM connector A (49P) No. 10 (BLK)
| Test point 2 | A/C pressure sensor 3P connector No. 1 (BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG6 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG6 wire between the ECM/PCM (A10) and the A/C pressure sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle.
- 1. Turn the blower switch on.
- 1. Turn the A/C switch on.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0533 indicated?
YES
Check for poor connections or loose terminals at the A/C pressure sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0562 — page 16744

- Source file: pages/16744.html

DTC P0562: Charging System Low Voltage (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. DTC check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle without load for 1 minute.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16BB indicated?
YES
Do the troubleshooting for DTC P16BB - Refer to: DTC P16BB (K24Z7) , DTC P16BB (Except K24Z7) .
NO
Go to step 2.

2. Problem verification:

- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0562 is indicated?
YES
Repair or replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) .
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the +B terminal on the alternator and the under-hood fuse/relay box.

## P0563 — page 16745

- Source file: pages/16745.html

DTC P0563: ECM/PCM Power Source Circuit Unexpected Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0563 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at PGM-FI main relay 1 and the ECM/PCM.

2. Determine possible failure area (FI_MAIN_RLY_OUT line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| Test circuit | FI_MAIN_RLY_CL-
| Test point 1 | ECM/PCM connector A (49P) No. 7 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 5.
NO
Go to step 3.

3. Shorted wire check (FI_MAIN_RLY_CL- line):

- 1. Remove PGM-FI main relay 1 (A) from the under-dash fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | PGM-FI main relay 1: disconnected
| Test circuit | FI_MAIN_RLY_CL-
| Test point 1 | ECM/PCM connector A (49P) No. 7 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the FI_MAIN_RLY_CL- wire between the ECM/PCM (A7) and PGM-FI main relay 1, then do the repair verification.
NO
The FI_MAIN_RLY_CL- wire is OK. Go to step 4.

4. Relay check:

- 1. Test PGM-FI main relay 1 - Refer to: Power Relay Test ('12-'13: except Natural Gas models) , Power Relay Test ('12-'13: Natural Gas models) .
Is PGM-FI main relay 1 OK?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace PGM-FI main relay 1, then do the repair verification.

5. Shorted wire check (FI_MAIN_RLY_OUT line to power):

- 1. Remove PGM-FI main relay 1 (A) from the under-dash fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | PGM-FI main relay 1: disconnected
| Test circuit | FI_MAIN_RLY_OUT
| Test point 1 | ECM/PCM connector A (49P) No. 9 (ORN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Repair a short to power in the FI_MAIN_RLY_OUT wire between the ECM/PCM (A9) and PGM-FI main relay 1, then do the repair verification.
NO
The FI_MAIN_RLY_OUT wire is OK. Go to step 6.

6. Relay check:

- 1. Test PGM-FI main relay 1 - Refer to: Power Relay Test ('12-'13: except Natural Gas models) , Power Relay Test ('12-'13: Natural Gas models) .
Is PGM-FI main relay 1 OK?
YES
PGM-FI main relay 1 is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace PGM-FI main relay 1, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0).
- 1. Wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0563 indicated?
YES
Check for poor connections or loose terminals at PGM-FI main relay 1 and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0602 — page 16752

- Source file: pages/16752.html

DTC P0602: ECM/PCM Programming Error (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
This DTC is indicated when an ECM/PCM update is not completed.
Do not turn the ignition switch to ACC (I) or LOCK (0) while updating the ECM/PCM. If you do, the ECM/PCM can be damaged.

1. Problem verification (update):

- 1. Do the ECM/PCM update procedure - Refer to: ECM/PCM Update .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0602 indicated?
YES
Replace the ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) .
NO
Update is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P060A — page 16756

- Source file: pages/16756.html

DTC P060A: ECM/PCM Internal Control Module Malfunction (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P060A indicated?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P060A indicated?
YES
If the ECM/PCM was updated, substitute a knowngood ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P062F — page 16764

- Source file: pages/16764.html

DTC P062F: ECM/PCM Internal Control Module Keep Alive Memory (KAM) Error (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P062F indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P062F indicated?
YES
If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0630 — page 16766

- Source file: pages/16766.html

DTC P0630: VIN Not Programmed or Mismatch (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
This DTC is stored only when the ECM/PCM does not have the VIN information of the vehicle. Use the HDS to input the missing VIN information.

1. VIN check:

- 1. Turn the ignition switch to ON (II).
- 1. Check the VIN with the HDS.
Does the HDS show the vehicle's VIN?
YES
Go to step 4.
NO
Go to step 2.

2. VIN check (rewrite):

- 1. Input the VIN to the ECM/PCM with the HDS.
Does the HDS show COMPLETE?
YES
Go to step 4.
NO
Go to step 3.

3. DTC check:

- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P062F indicated?
YES
Go to the DTC P062F troubleshooting - Refer to: DTC Troubleshooting: P062F (K24Z7) , DTC Troubleshooting: P062F (Except K24Z7) .
NO
Go to step 4.

4. ECM/PCM check:

- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II), and wait 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0630 indicated?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck.
NO
Intermittent failure, the system is OK at this time. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0641 — page 16767

- Source file: pages/16767.html

DTC P0641: Sensor Reference Voltage A Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0641 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ECM/PCM and these parts:
MAP sensor
Output shaft (countershaft) speed sensor
Throttle body
APP sensor

2. Determine possible failure area (VCC lines, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the connectors from these parts:
MAP sensor
Output shaft (countershaft) speed sensor
Throttle body
APP sensor
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0641 indicated?
YES
Go to step 4.
NO
Go to step 3.

3. Shorted part check:

- 1. Check for Pending or Confirmed DTCs with the HDS while reconnecting these parts, one at a time:
MAP sensor
Output shaft (countershaft) speed sensor
Throttle body
APP sensor
Is DTC P0641 indicated?
YES
Replace the part that caused the DTC when it was reconnected, then do the repair verification.
NO
Check for poor connections or loose terminals at the ECM/PCM and the following parts, then go to step 1.
MAP sensor
Output shaft (countershaft) speed sensor
Throttle body
APP sensor

4. Shorted wire check (VCC lines):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector A (49P)
| ECM/PCM connector C (49P)
- 1. Check for continuity between these test points and body ground individually.
| Test condition | IG LOCK (0)
| | MAP sensor 3P connector: disconnected
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| | Throttle body 6P connector: disconnected
| | APP sensor 6P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| | ECM/PCM connector C (49P): disconnected
| Connector | Test circuit | Terminal
| ECM/PCM connector A (49P) | VCC4 | No. 36 (BRN)
| ECM/PCM connector C (49P) | VCC1 | No. 23 (YEL/RED)
| VCC3 | No. 22 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VCC wire between the ECM/PCM (A36, C22, or C23) and each part that was disconnected in step 2-2, then do the repair verification.
NO
The VCC wires are OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0641 indicated?
YES
Check for poor connections or loose terminals at the ECM/PCM and the following parts, then go to step 1.
MAP sensor
Output shaft (countershaft) speed sensor
Throttle body
APP sensor
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0117 — page 17883

- Source file: pages/17883.html

DTC P0117: ECT Sensor 1 Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM.

2. ECT sensor 1 performance check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| ECT sensor 1 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
Go to step 3.
NO
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.

3. Shorted wire check (TW line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT sensor 1 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | TW
| Test point 1 | ECT sensor 1 2P connector No. 1 (RED/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TW wire between the ECM/PCM (B34) and ECT sensor 1, then do the repair verification.
NO
The TW wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0117 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0118 — page 17886

- Source file: pages/17886.html

DTC P0118: ECT Sensor 1 Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about - 40°F (- 40°C) or less, or 4.92 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM.

2. Determine possible failure area (ECT sensor 1, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| ECT sensor 1 2P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | ECT sensor 1 2P connector No. 1 (RED/WHT)
| Terminal B | ECT sensor 1 2P connector No. 2 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about - 40°F (- 40°C) or less, or 4.92 V or more indicated?
YES
Go to step 3.
NO
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.

3. Determine possible failure area (TW line, SG2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ECT sensor 1 2P connector: disconnected
| Test circuit | TW
| Test point 1 | ECT sensor 1 2P connector No. 1 (RED/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (SG2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT sensor 1 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SG2
| Test point 1 | ECM/PCM connector B (49P) No. 44 (GRN/YEL)
| Test point 2 | ECT sensor 1 2P connector No. 2 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG2 wire between the ECM/PCM (B44) and ECT sensor 1, then do the repair verification.

5. Open wire check (TW line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT sensor 1 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | TW
| Test point 1 | ECM/PCM connector B (49P) No. 34 (RED/WHT)
| Test point 2 | ECT sensor 1 2P connector No. 1 (RED/WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TW wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TW wire between the ECM/PCM (B34) and ECT sensor 1, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0118 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0101 — page 17893

- Source file: pages/17893.html

DTC P0101: MAF Sensor Circuit Range/Performance Problem (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If DTC P0106, P2228, and/or P2229 are stored at the same time as DTC P0101, troubleshoot those DTCs first, then recheck for DTC P0101.

1. Parts condition check:

- 1. Check for poor connections or damage to these parts:
PCV hose
Intake air duct
Air cleaner
Purge (PCS) line
Are the parts OK?
YES
Go to step 2.
NO
Repair or replace the damaged part(s), then do the repair verification.

2. Intake air duct visual check:

- 1. Check for damage or looseness of the air duct in the air cleaner.
Is it OK?
YES
Go to step 3.
NO
Reconnect or replace the air duct in the air cleaner, then do the repair verification.

3. Air cleaner element visual check:

- 1. Check for a dirty air cleaner element.
Is it dirty?
YES
Replace the air cleaner element - Refer to: Air Cleaner Element Removal, Installation, and Inspection (K24Z7) , Air Cleaner Element Removal, Installation, and Inspection (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. MAF sensor signal check (without engine running):

- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
Is there about 0.2 g/s or 0.5 V?
YES
Go to step 5.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

5. MAF sensor signal check (with engine running):

- 1. Start the engine.
- 1. Vary the engine speed between 2, 000 rpm and 3, 000 rpm.
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
Does the reading change?
YES
Go to step 6.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

6. MAF sensor check:

- 1. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
MAP SENSOR
TP SENSOR
- 1. Monitor the OBD STATUS for DTC P0101 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, go to step 6-2 and recheck.
Repair verification (OBD STATUS):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15)
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
MAP SENSOR
TP SENSOR
- 1. Monitor the OBD STATUS for DTC P0101 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0102 — page 17897

- Source file: pages/17897.html

DTC P0102: MAF Sensor Circuit Low Voltage ('13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II), and wait 2 seconds.
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
Is about 0 g/s, or 0.1 V or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.

2. Shorted wire check (IG1_ACG/IG1(AFM) line)

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. B2 (10 A)
| Location | Under-dash fuse/relay box
Is the fuse OK?
YES
The IG1_ACG/IG1(AFM) wires are not shorted. Go to step 3.
NO
Repair a short in the IG1_ACG/IG1(AFM) wire between the MAF sensor/IAT sensor and the No. B2 (10 A) fuse. Also replace the No. B2 (10 A) fuse, then do the repair verification.

3. Open wire check (IG1_ACG/IG1(AFM) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| MAF sensor/IAT sensor 5P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | MAF sensor/IAT sensor 5P connector: disconnected
| Test circuit | IG1(AFM)
| Test point 1 | MAF sensor/IAT sensor 5P connector No. 1 (BLK/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IG1_ACG/IG1(AFM) wire is OK. Go to step 4.
NO
Repair an open in the IG1_ACG/IG1(AFM) wire between the No. B2 (10 A) fuse and the MAF sensor/IAT sensor, then do the repair verification.

4. Determine possible failure area (VGP line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| Test circuit | VGP
| Test point 1 | MAF sensor/IAT sensor 5P connector No. 3 (RED/BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there 190-210 kΩ?
YES
Go to step 7.
NO
Go to step 5.

5. Shorted wire check (VGP line):

- 1. Jump the SCS line with the HDS
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VGP
| Test point 1 | ECM/PCM connector B (49P) No. 22 (RED/BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VGP wire between the ECM/PCM (B22) and the MAF sensor/IAT sensor, then do the repair verification.
NO
The VGP wire is not shorted. Go to step 6.

6. Open wire check (VGP line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VGP
| Test point 1 | ECM/PCM connector B (49P) No. 22 (RED/BLU)
| Test point 2 | MAF sensor/IAT sensor 5P connector No. 3 (RED/BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VGP wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VGP wire between the ECM/PCM (B22) and the MAF sensor/IAT sensor, then do the repair verification.

7. MAF sensor/IAT sensor check (substitution):

- 1. Substitute a known-good MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 2, 000 rpm without load (A/T in P or N, M/T in neutral).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0102 indicated?
YES
The MAF sensor/IAT sensor is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the original MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0102 indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0103 — page 17899

- Source file: pages/17899.html

DTC P0103: MAF Sensor Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II), and wait 2 seconds.
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
Is about 202 g/s, or 4.89 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.

2. Open wire check (VGM line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| MAF sensor/IAT sensor 5P connector
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VGM
| Test point 1 | ECM/PCM connector B (49P) No. 43 (BLK/RED)
| Test point 2 | MAF sensor/IAT sensor 5P connector No. 2 (BLK/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VGM wire is OK. Go to step 3.
NO
Repair an open in the VGM wire between the ECM/PCM (B43) and the MAF sensor/IAT sensor, then do the repair verification.

3. ECM/PCM internal circuit check:

- 1. Reconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | MAF sensor/IAT sensor 5P connector No. 2 (BLK/RED)
| Terminal B | MAF sensor/IAT sensor 5P connector No. 3 (RED/BLU)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0103 indicated?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
The ECM/PCM internal circuit is OK. Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0103 indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0107 — page 17902

- Source file: pages/17902.html

DTC P0107: MAP Sensor Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is about 3 kPa (1.0 inHg, 26 mmHg), or 0.23 V or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM.

2. Determine possible failure area (MAP(PB) line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| MAP sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is about 3 kPa (1.0 inHg, 26 mmHg), or 0.23 V or less indicated?
YES
Go to step 5.
NO
Go to step 3.

3. Determine possible failure area (MAP sensor, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | MAP sensor 3P connector: disconnected
| Test circuit | VCC1, SG1
| Test point 1 | MAP sensor 3P connector No. 1 (GRY/WHT)
| Test point 2 | MAP sensor 3P connector No. 3 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. Open wire check (VCC1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAP sensor 3P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VCC1
| Test point 1 | ECM/PCM connector C (49P) No. 23 (YEL/RED)
| Test point 2 | MAP sensor 3P connector No. 3 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC1 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC1 wire between the ECM/PCM (C23) and the MAP sensor, then do the repair verification.

5. Shorted wire check (MAP(PB) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG ON (II)
| | MAP sensor 3P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | MAP(PB)
| Test point 1 | MAP sensor 3P connector No. 2 (GRN/RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the MAP(PB) wire between the ECM/PCM (B31) and the MAP sensor, then do the repair verification.
NO
The MAP(PB) wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0107 indicated?
YES
Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0108 — page 17905

- Source file: pages/17905.html

DTC P0108: MAP Sensor Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is about 160 kPa (47.1 inHg, 1, 197 mmHg), or 4.49 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM.

2. Determine possible failure area (MAP sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| MAP sensor 3P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | MAP sensor 3P connector No. 1 (GRN/WHT)
| Terminal B | MAP sensor 3P connector No. 2 (GRN/RED)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is about 160 kPa (47.1 inHg, 1, 197 mmHg), or 4.49 V or more indicated?
YES
Go to step 3.
NO
Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

3. Determine possible failure area (MAP(PB) line, SG1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | MAP sensor 3P connector: disconnected
| Test circuit | VCC1, SG1
| Test point 1 | MAP sensor 3P connector No. 1 (GRN/WHT)
| Test point 2 | MAP sensor 3P connector No. 3 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 5.
NO
Go to step 4.

4. Open wire check (SG1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAP sensor 3P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | SG1
| Test point 1 | ECM/PCM connector C (49P) No. 24 (GRY)
| Test point 2 | MAP sensor 3P connector No. 1 (GRY)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG1 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG1 wire between the ECM/PCM (C24) and the MAP sensor, then do the repair verification.

5. Open wire check (MAP(PB) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAP sensor 3P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | MAP(PB)
| Test point 1 | ECM/PCM connector B (49P) No. 31 (GRN/RED)
| Test point 2 | MAP sensor 3P connector No. 2 (GRN/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The MAP(PB) wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the MAP(PB) wire between the ECM/PCM (B31) and the A/C pressure sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0108 indicated?
YES
Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0111 — page 17909

- Source file: pages/17909.html

DTC P0111: IAT Sensor Circuit Range/Performance Problem (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Connector visual check:

- 1. Check for poor or loose connections and terminals at these locations:
MAF sensor/IAT sensor
ECT sensor 1
ECT sensor 2
Are the connections and terminals OK?
YES
Go to step 2.
NO
Repair the connections or terminals, then do the repair verification.

2. IAT sensor performance check (low temperature):

- 1. Remove the MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Allow the MAF sensor/IAT sensor to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Connect the MAF sensor/IAT sensor to its 5P connector, but do not install it.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of the IAT SENSOR in the DATA LIST with the HDS.
- 1. Compare the value of IAT SENSOR to the ambient temperature.
Does the value of IAT SENSOR differ 5.4°F (3°C) or more from the ambient temperature?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. IAT sensor performance check (high temperature):

- 1. Disconnect the following connector.
| MAF sensor/IAT sensor 5P connector
- 1. Using a heat gun, blow hot air on the MAF sensor/IAT sensor for a few seconds. Do not apply the heat longer than a few seconds or you will damage the sensor.
- 1. Connect the MAF sensor/IAT sensor to its 5P connector, but do not install it.
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Does the IAT SENSOR change 76°F (42°C) or more?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0111 indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0112 — page 17910

- Source file: pages/17910.html

DTC P0112: IAT Sensor Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.

2. Determine possible failure area (IAT sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| MAF sensor/IAT sensor 5P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
Go to step 3.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

3. Shorted wire check (TA line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | TA
| Test point 1 | MAF sensor/IAT sensor 5P connector No. 4 (RED/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TA wire between the ECM/PCM (B42) and the MAF sensor/IAT sensor, then do the repair verification.
NO
The TA wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0112 indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0113 — page 17913

- Source file: pages/17913.html

DTC P0113: IAT Sensor Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Is about - 40°F (- 40°C) or less, or 4.92 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.

2. Determine possible failure area (IAT sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| MAF sensor/IAT sensor 5P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | MAF sensor/IAT sensor 5P connector No. 4 (RED/YEL)
| Terminal B | MAF sensor/IAT sensor 5P connector No. 5 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Is about - 40°F (- 40°C) or less, or 4.92 V or more indicated?
YES
Go to step 3.
NO
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.

3. Determine possible failure area (TA line, SG2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | MAF sensor/IAT sensor 5P connector: disconnected
| Test circuit | TA
| Test point 1 | MAF sensor/IAT sensor 5P connector No. 4 (RED/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (SG2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SG2
| Test point 1 | ECM/PCM connector B (49P) No. 44 (GRN/YEL)
| Test point 2 | MAF sensor/IAT sensor 5P connector No. 5 (GRN/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG2 wire between the ECM/PCM (B44) and the MAF sensor/IAT sensor, then do the repair verification.

5. Open wire check (TA line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | MAF sensor/IAT sensor 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | TA
| Test point 1 | ECM/PCM connector B (49P) No. 42 (RED/YEL)
| Test point 2 | MAF sensor/IAT sensor 5P connector No. 4 (RED/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TA wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TA wire between the ECM/PCM (B42) and the MAF sensor/IAT sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0113 indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0116 — page 17917

- Source file: pages/17917.html

DTC P0116: ECT Sensor 1 Circuit Range/Performance Problem (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. ECT sensor 1 check:

- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 176°F (80°C) or more indicated?
YES
Note the value of ECT SENSOR 1, then go to step 3.
NO
Note the value of ECT SENSOR 1, then go to step 2.

2. Problem verification (low temperature):

- 1. Note the value of ECT SENSOR 1 in the DATA LIST with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Does ECT SENSOR 1 change 18°F (10°C) or more?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM.
NO
The failure is duplicated. Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.

3. Problem verification (high temperature):

- 1. Turn the ignition switch to LOCK (0).
- 1. Open the hood, and let the engine cool for 3 hours.
- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Does ECT SENSOR 1 change 18°F (10°C) or more?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM.
NO
The failure is duplicated. Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0116 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1 and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P011B — page 17918

- Source file: pages/17918.html

DTC P011B: IAT Sensor Circuit Range/Performance Problem ('13-'15: KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Connector visual check:

- 1. Check for poor connections or loose terminals at these parts:
MAF sensor/IAT sensor
ECT sensor 1
ECT sensor 2
Are the connections and the terminals OK?
YES
Go to step 2.
NO
Repair the connections or the terminals, then do the repair verification.

2. IAT sensor performance check (low temperature):

- 1. Remove the MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Allow the MAF sensor/IAT sensor to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Connect the MAF sensor/IAT sensor to its 5P connector, but do not install it.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of the IAT SENSOR in the DATA LIST with the HDS.
- 1. Compare the value of IAT SENSOR to the ambient temperature.
Does the value of IAT SENSOR differ 5.4°F (3°C) or more from the ambient temperature?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. IAT sensor performance check (high temperature):

- 1. Disconnect the following connector.
| MAF sensor/IAT sensor 5P connector
- 1. Using a heat gun, blow hot air on the MAF sensor/IAT sensor for a few seconds. Do not apply the heat longer than a few seconds or you will damage the sensor.
- 1. Connect the MAF sensor/IAT sensor to its 5P connector, but do not install it.
- 1. Check the IAT SENSOR in the DATA LIST with the HDS.
Does the IAT SENSOR change 76°F (42°C) or more?
YES
Replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P011B indicated?
YES
Check for poor connections or loose terminals at the MAF sensor/IAT sensor and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0122 — page 17920

- Source file: pages/17920.html

DTC P0122: TP Sensor A Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check TP SENSOR A in the DATA LIST with the HDS.
Is there about 0.3 V or less?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Determine possible failure area (VCC3 line, TH_1 line):

- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P0122 and P0222 indicated at the same time?
YES
Go to step 4.
NO
Go to step 3.

3. Shorted wire check (TH_1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | TH_1
| Test point 1 | Throttle body 6P connector No. 6 (RED/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TH_1 wire between the ECM/PCM (C30) and the throttle body, then do the repair verification.
NO
The TH_1 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Determine possible failure area (VCC3 line, throttle body):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Throttle body 6P connector: disconnected
| Test circuit | VCC3
| Test point 1 | Throttle body 6P connector No. 5 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly(K24Z7) , Throttle Body Removal and Installation(R18Z1) , Throttle Body Removal and Installation(R18A9) , Throttle Body Disassembly and Reassembly(Except K24Z7) , then do the repair verification.
NO
Go to step 5.

5. Open wire check (VCC3 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Throttle body 6P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VCC3
| Test point 1 | ECM/PCM connector C (49P) No. 22 (GRN)
| Test point 2 | Throttle body 6P connector No. 5 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC3 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC3 wire between the ECM/PCM (C22) and the throttle body, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0122 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0123 — page 17923

- Source file: pages/17923.html

DTC P0123: TP Sensor A Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check TP SENSOR A in the DATA LIST with the HDS.
Is there about 4.8 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Determine possible failure area (SG3 line, others):

- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P0123 and P0223 indicated at the same time?
YES
Go to step 5.
NO
Go to step 3.

3. Determine possible failure area (TH_1 line, throttle body):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Throttle body 6P connector: disconnected
| Test circuit | TH_1
| Test point 1 | Throttle body 6P connector No. 6 (RED/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the throttle body - Refer to: Throttle Body Disassembly and Reassembly(K24Z7) , Throttle Body Removal and Installation (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. Open wire check (TH_1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Throttle body 6P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | TH_1
| Test point 1 | ECM/PCM connector C (49P) No. 30 (RED/BLK)
| Test point 2 | Throttle body 6P connector No. 6 (RED/BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TH_1 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TH_1 wire between the ECM/PCM (C30) and the throttle body, then do the repair verification.

5. Open wire check (SG3 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | SG3
| Test point 1 | ECM/PCM connector C (49P) No. 14 (BLU)
| Test point 2 | Throttle body 6P connector No. 3 (BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG3 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG3 wire between the ECM/PCM (C14) and the throttle body, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0123 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0125 — page 17927

- Source file: pages/17927.html

DTC P0125: ECT Sensor 1 Malfunction/Slow Response (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. ECT sensor 1 check:

- 1. Start the engine, and let it idle for 5 minutes or more.
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 0°F (- 12°C) or less indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM. If they are OK, replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
Go to step 2.

2. Problem verification:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the engine to cool to 104°F (40°C) or less.
- 1. Start the engine, and let it idle until ECT SENSOR 1 goes up to about 158°F (70°C).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Does ECT SENSOR 2 also read about 158°F (70°C)?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.

3. Thermostat check:

- 1. Check the thermostat - Refer to: Thermostat Removal, Installation, and Test (K24Z7) , Thermostat Removal, Installation, and Test (R18Z1) , Thermostat Removal, Installation, and Test (R18A9) .
Is the thermostat OK?
YES
Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM. If the connections and terminals are OK, replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
Replace the thermostat - Refer to: Thermostat Removal, Installation, and Test (K24Z7) , Thermostat Removal, Installation, and Test (R18Z1) , Thermostat Removal, Installation, and Test (R18A9) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the engine to cool to 104°F (40°C) or less.
- 1. Start the engine, and let it idle until ECT SENSOR 1 goes up to about 158°F (70°C).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Does ECT SENSOR 2 also read about 158°F (70°C)?
YES
Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0128 — page 17929

- Source file: pages/17929.html

DTC P0128: Cooling System Malfunction (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Radiator fan control command check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the blower switch off.
- 1. Check the FAN HIGH CTRL and FAN LOW CTRL in the DATA LIST with the HDS.
Are they OFF?
YES
Go to step 2.
NO
Wait until the FAN HIGH CTRL and FAN LOW CTRL are off, then go to step 4.

2. Radiator fan operation check:

- 1. Check the radiator fan operation.
Does the radiator fan keep running?
YES
Go to step 3.
NO
Wait until the FAN HIGH CTRL and FAN LOW CTRL are off, then go to step 4.

3. Radiator fan circuit and radiator fan relay check:

- 1. Check the radiator fan circuit, and the radiator fan relay.
Is it OK?
YES
Repair the radiator fan circuit, and the radiator fan relay, then do the repair verification.
NO
Update the ECM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Problem verification:

- 1. Let the engine cool until the coolant temperature is 104°F (40°C) or less.
- 1. Note the value of ECT SENSOR 1 and ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Start the engine, and let it idle.
- 1. Let the engine idle until ECT SENSOR 1 goes up 36°F (20°C) or more from the recorded temperature.
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Compare the recorded value of ECT SENSOR 2 and the present value of ECT SENSOR 2.
Did temperature rise 9°F (5°C) or more?
YES
Test the thermostat - Refer to: Thermostat Removal, Installation, and Test (K24Z7) , Thermostat Removal, Installation, and Test (R18Z1) , Thermostat Removal, Installation, and Test (R18A9) and replace it if needed, then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Let the engine cool until the coolant temperature is between 21°F (- 6°C) and 104°F (40°C).
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Test-drive at a steady speed between 15-75 mph (25-120 km/h) for 30 minutes.
- 1. Monitor the OBD STATUS for DTC P0128 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P0133 — page 17931

- Source file: pages/17931.html

DTC P0133: A/F Sensor (Sensor 1) Malfunction/Slow Response (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If DTC P0139 is stored at the same time as DTC P0133, troubleshoot DTC P0139 first, then recheck for DTC P0133.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 3rd or 4th
Engine speed between 1, 500-3, 300 rpm
Drive the vehicle at 25 mph (40 km/h) or less for 5 minutes, then drive at a steady speed of 32 mph (52 km/h) or more
- 1. Monitor the OBD STATUS for DTC P0133 with the HDS.
Does the HDS indicate FAILED?
YES
Replace A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation(R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1) and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-3 on this step and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 3rd or 4th
Engine speed between 1, 500-3, 300 rpm
Drive the vehicle at 25 mph (40 km/h) or less for 5 minutes, then drive at a steady speed of 32 mph (52 km/h) or more
- 1. Monitor the OBD STATUS for DTC P0133 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0134 — page 17934

- Source file: pages/17934.html

DTC P0134: A/F Sensor (Sensor 1) Heater System Malfunction (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If the vehicle was out of fuel and the engine stalled before this DTC was stored, refuel and clear the DTC with the HDS.
If DTC P0135 is stored at the same time as DTC P0134, troubleshoot DTC P0135 first, then recheck for DTC P0134.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0134 indicated?
YES
The failure is duplicated. Replace the A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation(R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), the PGM-FI subrelay, and the ECM/PCM.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Monitor the OBD STATUS for DTC P0134 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Keep the test condition until a result comes on.

## P0135 — page 17935

- Source file: pages/17935.html

DTC P0135: A/F Sensor (Sensor 1) Heater Circuit Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0135 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), the PGM-FI subrelay, and the ECM/PCM.

2. Fuse check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. A15 (15 A)
| Location | Under-hood fuse/relay box
Is the fuse OK?
YES
Go to step 3.
NO
Go to step 9.

3. Relay check:

- 1. Remove the PGM-FI subrelay (A) from the under-hood fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Test the PGM-FI subrelay. - Refer to: Power Relay Test ('12-'13: except Natural Gas models) , Power Relay Test ('12-'13: Natural Gas models)
Is the PGM-FI subrelay OK?
YES
Go to step 4.
NO
Replace the PGM-FI subrelay, then do the repair verification.

4. A/F sensor (Sensor 1) internal heater resistance check:

- 1. Disconnect the following connector.
| A/F sensor (Sensor 1) 4P connector
- 1. At the sensor side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| Test circuit | IGPS(LAF+), AFHT
| Test point 1 | A/F sensor (Sensor 1) 4P connector No. 3 (A/F sensor (Sensor 1) side)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 4 (A/F sensor (Sensor 1) side)
| Courtesy of HONDA, U.S.A., INC.
Is there 2.2-2.7 - at room temperature-
YES
Go to step 5.
NO
Replace the A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

5. A/F sensor (Sensor 1) internal circuit check:

- 1. At the sensor side, check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| Test circuit | AFS-, AFHT
| Test point 1 | A/F sensor (Sensor 1) 4P connector No. 2 (A/F sensor (Sensor 1) side)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 4 (A/F sensor (Sensor 1) side)
| Test circuit | AFS-, IGPS(LAF+)
| Test point 1 | A/F sensor (Sensor 1) 4P connector No. 2 (A/F sensor (Sensor 1) side)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 3 (A/F sensor (Sensor 1) side)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Replace the A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Go to step 6.

6. Open wire check (AFHT line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | AFHT
| Test point 1 | ECM/PCM connector C (49P) No. 9 (GRN)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 4 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The AFHT wire is OK. Go to step 7.
NO
Repair an open in the AFHT wire between the ECM/PCM (C9) and the A/F sensor (Sensor 1), then do the repair verification.

7. Open wire check (IGPS(LAF+), FI_SUB_RLY_OUT lines):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | IGPS(LAF+), FI_SUB_RLY_OUT
| Test point 1 | PGM-FI subrelay 4P socket No. 2
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 3 (WHT/BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The IGPS(LAF+)/FI_SUB_RLY_OUT wires are OK. Go to step 8.
NO
Repair an open in the IGPS(LAF+)/FI_SUB_RLY_OUT wire between the A/F sensor (Sensor 1) and the PGM-FI subrelay, then do the repair verification.

8. Open wire check (FI_SUB_RLY_CL-T line):

- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | FI_SUB_RLY_CL-
| Test point 1 | ECM/PCM connector A (49P) No. 20 (LT BLU)
| Test point 2 | PGM-FI subrelay 4P socket No. 4
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The FI_SUB_RLY_CL- wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the FI_SUB_RLY_CL- wire between the ECM/PCM (A20) and the PGM-FI subrelay, then do the repair verification.

9. Shorted wire check (IGPS(LAF+), FI_SUB_RLY_OUT lines):

- 1. Remove the PGM-FI subrelay (A) and the A/C condenser fan relay (B) from the under-hood fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector A (49P)
| A/F sensor (Sensor 1) 4P connector
| EVAP canister vent shut valve 2P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | PGM-FI subrelay: disconnected
| | A/C condenser fan relay: disconnected
| | ECM/PCM connector A (49P): disconnected
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | EVAP canister vent shut valve 2P connector: disconnected
| Test circuit | IGPS(LAF+)
| Test point 1 | A/F sensor (Sensor 1) 4P connector No. 3 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the IGPS(LAF+)/FI_SUB_RLY_OUT wire between the PGM-FI subrelay and the A/F sensor (Sensor 1), the A/C condenser fan relay, and the EVAP canister vent shut valve. Also replace the No. A15 (15 A) fuse, then do the repair verification.
NO
The IGPS(LAF+)/FI_SUB_RLY_OUT wires are OK. Replace the under-hood fuse/relay box, then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Monitor the OBD STATUS for DTC P0135 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep the test condition until a result comes on.

## P0137 — page 17938

- Source file: pages/17938.html

DTC P0137: Secondary HO2S (Sensor 2) Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Monitor the OBD STATUS for DTC P0137 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 2.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at secondary HO2S (Sensor 2) and the ECM/PCM. If the HDS indicates EXECUTING, keep idling until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-3 and recheck.

2. Shorted wire check (SO2_SG line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
| Secondary HO2S (Sensor 2) 4P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| Test circuit | SO2_SG
| Test point 1 | ECM/PCM connector C (49P) No. 47 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the SO2_SG wire between the ECM/PCM (C47) and secondary HO2S (Sensor 2), then do the repair verification.
NO
The SO2_SG wire is not open. Go to step 3.

3. Shorted wire check (SO2 line):

- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SO2
| Test point 1 | ECM/PCM connector B (49P) No. 46 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the SO2 wire between the ECM/PCM (B46) and secondary HO2S (Sensor 2), then do the repair verification.
NO
The SO2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P0137 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0138 — page 17942

- Source file: pages/17942.html

DTC P0138: Secondary HO2S (Sensor 2) Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Check the HO2S S2 OUTPUT VOLTAGE in the DATA LIST with the HDS.
Does the voltage stay at 1.27 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at secondary HO2S (Sensor 2) and the ECM/PCM.

2. Determine possible failure area (Secondary HO2S (Sensor 2), others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Secondary HO2S (Sensor 2) 4P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Secondary HO2S (Sensor 2) 4P connector No. 1 (BLK)
| Terminal B | Secondary HO2S (Sensor 2) 4P connector No. 2 (WHT)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the HO2S S2 OUTPUT VOLTAGE in the DATA LIST with the HDS.
Does the voltage stay at 1.27 V or more?
YES
Go to step 3.
NO
Replace secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

3. Open wire check (SO2_SG line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire from the secondary HO2S (Sensor 2) 4P connector.
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Secondary HO2S (Sensor 2) 4P connector No. 2 (WHT)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the HO2S S2 OUTPUT VOLTAGE in the DATA LIST with the HDS.
Does the voltage stay at 1.27 V or more?
YES
The SO2_SG wire is OK. Go to step 4.
NO
Repair an open in the SO2_SG wire between the ECM/PCM (C47) and secondary HO2S (Sensor 2), then do the repair verification.

4. Open wire check (SO2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Secondary HO2S (Sensor 2) 4P connector terminal No. 2: jumped to body ground
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SO2
| Test point 1 | ECM/PCM connector B (49P) No. 46 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SO2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SO2 wire between the ECM/PCM (B46) and secondary HO2S (Sensor 2), then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
Transmission in D
Engine speed between 1, 500-3, 000 rpm
Drive 1 minute or more
- 1. Monitor the OBD STATUS for DTC P0138 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0139 — page 17947

- Source file: pages/17947.html

DTC P0139: Secondary HO2S (Sensor 2) Slow Response (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive at a steady speed between 55-75 mph (90-120 km/h) for 1 minute, then decelerate (with the throttle fully closed) for 10 seconds
- 1. Monitor the OBD STATUS for DTC P0139 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Replace the secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Secondary HO2S Removal and Installation(R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at secondary HO2S (Sensor 2) and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-3 on this step and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Drive at a steady speed between 55-75 mph (90-120 km/h) for 1 minute, then decelerate (with the throttle fully closed) for 10 seconds
- 1. Monitor the OBD STATUS for DTC P0139 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P0141 — page 17951

- Source file: pages/17951.html

DTC P0141: Secondary HO2S (Sensor 2) Heater Circuit Malfunction ('13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0141 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at secondary HO2S (Sensor 2), and the ECM/PCM.

2. Shorted wire check (IG1_ACG/IG1(SO2) line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. B2 (10 A)
| Location | Under-dash fuse/relay box
Is the fuse OK?
YES
The IG1_ACG/IG1(SO2) wire is not shorted. Go to step 3.
NO
Repair a short in the IG1_ACG/IG1(SO2) wire between secondary HO2S (Sensor 2) and the No. B2 (10 A) fuse. Also replace the No. B2 (10 A) fuse, then do the repair verification.

3. Secondary HO2S (Sensor 2) internal heater resistance check:

- 1. Disconnect the following connector.
| Secondary HO2S (Sensor 2) 4P connector
- 1. At the sensor side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| Test circuit | IG1(SO2), SO2HT
| Test point 1 | Secondary HO2S (Sensor 2) 4P connector No. 3 (secondary HO2S (Sensor 2) side)
| Test point 2 | Secondary HO2S (Sensor 2) 4P connector No. 4 (secondary HO2S (Sensor 2) side)
| Courtesy of HONDA, U.S.A., INC.
Is there 5.4-7.3 - at room temperature-
YES
Go to step 4.
NO
Replace secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Secondary HO2S (Sensor 2) internal circuit check:

- 1. At the sensor side, check for continuity under the following conditions.
| Test condition | IG LOCK (0)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| Test circuit | SO2HT
| Test point 1 | Secondary HO2S (Sensor 2) 4P connector No. 3 (secondary HO2S (Sensor 2) side)
| Test point 2 | Body ground
| Test circuit | IG1(SO2)
| Test point 1 | Secondary HO2S (Sensor 2) 4P connector No. 4 (secondary HO2S (Sensor 2) side)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Replace secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Go to step 5.

5. Determine possible failure area (SO2HT line, others):

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| Test circuit | IG1(SO2), SO2HT
| Test point 1 | Secondary HO2S (Sensor 2) 4P connector No. 3 (BLK/WHT)
| Test point 2 | Secondary HO2S (Sensor 2) 4P connector No. 4 (BLK/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 6.
NO
Go to step 7.

6. Shorted wire check (SO2HT line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SO2HT
| Test point 1 | ECM/PCM connector B (49P) No. 4 (BLK/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the SO2HT wire between the ECM/PCM (B4) and secondary HO2S (Sensor 2), then do the repair verification.
NO
The SO2HT wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

7. Open wire check (IG1_ACG/IG1(SO2) line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| Test circuit | IG1(SO2)
| Test point 1 | Secondary HO2S (Sensor 2) 4P connector No. 4 (BLK/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IG1_ACG/IG1(SO2) wire is OK. Go to step 8.
NO
Repair an open in the IG1_ACG/IG1(SO2) wire between the No. B2 (10 A) fuse and secondary HO2S (Sensor 2), then do the repair verification.

8. Open wire check (SO2HT line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Secondary HO2S (Sensor 2) 4P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SO2HT
| Test point 1 | ECM/PCM connector B (49P) No. 4 (BLK/WHT)
| Test point 2 | Secondary HO2S (Sensor 2) 4P connector No. 3 (BLK/WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SO2HT wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SO2HT wire between the ECM/PCM (B4) and secondary HO2S (Sensor 2), then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Monitor the OBD STATUS for DTC P0141 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Meet the test conditions, and check the OBD STATUS again.

## P0171 — page 17956

- Source file: pages/17956.html

DTC P0171: Fuel System Too Lean (R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information.
Do not clear the DTC or reset the ECM/PCM before beginning this troubleshooting procedure until instructed to clear it. Clearing the DTC or resetting the ECM/PCM can delete valuable diagnostic information, even if the on-board snapshot and the freeze data is saved.
Inspect for aftermarket fuel system and engine components before beginning troubleshooting.
If any of the DTCs listed below are stored at the same time as DTC P0171, troubleshoot those DTCs first, then recheck for P0171.
P0101, P0102, P0103: MAF sensor
P0106, P0107, P0108: MAP sensor
P0133, P1172, P1157, P2238, P2252, P2A00: A/F sensor (Sensor 1)
P0134, P0135: A/F sensor (Sensor 1) heater
P0137, P0138, P0139, P2270: Secondary HO2S (Sensor 2)
P0141: Secondary HO2S (Sensor 2) heater
P0201, P0202, P0203, P0204: Fuel injector
P0401, P0404, P0406, P2413: EGR system
P0443, P0496: EVAP canister purge valve
P2646, P2647, P2648, P2649: VTEC system

1. Failure Confirmation-Idle:

- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral).
- 1. Monitor AF FB (ST FUEL TRIM) in the DATA LIST with the HDS.
Does AF FB (ST FUEL TRIM) read 1.25 or more at idle?
YES
AF FB (ST FUEL TRIM) is too high at idle. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK at idle. Go to step 2.

2. Failure Confirmation-On-board Snapshot:

- 1. Review the on-board snapshot and record the value of the parameters below when AF FB (ST FUEL TRIM) was at its highest point.
| ENGINE SPEED |
| VEHICLE SPEED |
| REL TP SENSOR |
| APP SENSOR |
- 1. Drive the vehicle according to the conditions determined above.
Does AF FB (ST FUEL TRIM) read 1.25 or more under the on-board snapshot conditions?
YES
AF FB (ST FUEL TRIM) is too high under the conditions above. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK under the same conditions as the on-board snapshot. The system is normal at this time.

3. Check for MAF Sensor Debris:

- 1. Remove the MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Check for lint, bugs, and other foreign objects in the sensor.
Is there debris in the MAF sensor?
YES
Remove the debris from the MAF sensor and then use a vacuum cleaner to remove any debris from the air cleaner housing, then do the confirm repair procedure.
NO
Reinstall the MAF sensor/IAT sensor. Go to step 4.

4. Check for Disconnected or Damaged Components:

- 1. Visually inspect each of the components below for disconnected hoses, cracks, or damage:
Crankcase breather hose (A)
Air cleaner housing (B)
Oil cap (C) and dipstick (D)
| Courtesy of HONDA, U.S.A., INC.
Are any components disconnected or damaged?
YES
Repair as necessary, then do the confirm repair procedure.
NO
Go to step 5.

5. Check Brake Booster

- 1. Continue to let the engine idle.
- 1. Pinch the brake booster vacuum hose (A) connecting to the intake manifold.
| Courtesy of HONDA, U.S.A., INC.
- 1. Monitor AF FB (ST FUEL TRIM) with the vacuum hose pinched.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
The brake booster is not leaking. Go to step 6.
NO
There is a leak in the brake booster or hoses. Check the brake booster vacuum hoses for leaks. If no leaks are found, replace the brake booster, then do the confirm repair procedure.

6. Check PCV System:

- 1. Continue to let the engine idle.
- 1. Pinch the PCV vacuum hose (A).
| Courtesy of HONDA, U.S.A., INC.
- 1. Monitor AF FB (ST FUEL TRIM) with the vacuum hose pinched.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
The PCV system is not leaking. Go to step 7.
NO
There is a leak in the PCV system. Check the vacuum hoses for leaks and repair as needed, then do the confirm repair procedure.

7. Check MAF Sensor Reading:

- 1. Start the engine and hold the engine speed at 3, 000 rpm until the radiator fan comes on.
- 1. Make sure the following conditions are met:
ECT Sensor 1 above 158°F (70°C)
A/T in P or N, M/T in neutral
All engine and electrical loads turned off
- 1. Monitor the ENGINE SPEED in the DATA LIST with the HDS, then hold the engine speed at 2, 500 rpm with the accelerator pedal held steady.
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
| Model | Specifications
| M/T | 5.3-6.5 gm/s
| A/T | 5.3-6.5 gm/s
Is there MAF sensor within specifications at 2, 500 rpm with the engine fully warm and all loads off?
YES
The MAF sensor is OK. Go to step 8.
NO
The MAF sensor is reading incorrectly. Inspect for a cracked, damaged, or incorrectly installed air cleaner assembly. If OK, replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the confirm repair procedure.

8. Check Fuel Pressure:

- 1. Check the fuel pressure under the conditions when AF FB (ST FUEL TRIM) is 1.25 or more.
Is the fuel pressure between 380-430 kPa (3.9-4.4 kgf/cm 2 , 55-63 psi)?
YES
The fuel pressure is OK. Go to step 9.
NO
Replace the fuel tank unit - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation(R18Z1) , then do the confirm repair procedure.

9. Check for Intake Manifold Vacuum Leaks:

- 1. Disconnect the intake air duct from the air cleaner housing.
- 1. Connect a large intake manifold adaptor cone into the intake air duct from the OTC LeakMaster evaporative emissions system tester kit.
- 1. Use the OTC LeakMaster evaporative emissions system tester to direct smoke into the intake manifold.
- 1. Inspect each of the areas below for signs of leaks:
Intake manifold and gaskets
Throttle body and gasket
EGR valve gasket
Air intake ducting
All vacuum and air hoses
Are there any leaks?
YES
Repair the vacuum leak, then do the confirm repair procedure.
NO
Reconnect any disconnected components, then go to step 10.
 1. Replace Fuel Injectors:

- 1. Replace the fuel injectors with known-good injectors. - Refer to: Injector Removal and Installation (K24Z7) Injector Removal and Installation(R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , Injector Removal and Installation (R18Z1 (CVT)) ('14-'15)
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 1.25 or more.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
The fuel injectors are not the cause of the high AF FB (ST FUEL TRIM) value. Go to step 11.
NO
The fuel injectors were causing the high AF FB (ST FUEL TRIM) value. Replace the original injectors - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , Injector Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the confirm repair procedure.
 1. Replace A/F Sensor (Sensor 1)

- 1. Replace the A/F sensor (Sensor 1) with a known-good sensor. - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15)
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 1.25 or more.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
The A/F sensor (Sensor 1) is not the cause of the high AF FB (ST FUEL TRIM) value. Go to step 12.
NO
The A/F sensor (Sensor 1) was causing the high AF FB (ST FUEL TRIM) value. Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the confirm repair procedure.
 1. Replace Fuel: NOTE: There is an increased likelihood of a fuel related issue if the misfire appeared shortly after refueling. The MIL DIST (MIL Distance) value in the DATA LIST in combination with the fuel level may help you make this determination.

- 1. Remove the fuel tank unit and then drain and refill the fuel tank with known-good fuel.
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 1.25 or more.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
The fuel is not the cause of the high AF FB (ST FUEL TRIM) value. Substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then do the confirm repair procedure.
NO
The fuel was causing the high AF FB (ST FUEL TRIM) value. Do the repair confirm procedure.
Confirm Repair:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS.
- 1. Do the ECM/PCM idle learn procedure.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 1.25 or more.
Is AF FB (ST FUEL TRIM) still 1.25 or more?
YES
AF FB (ST FUEL TRIM) is too high. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK. Troubleshooting is complete.

## P0172 — page 17959

- Source file: pages/17959.html

DTC P0172: Fuel System Too Rich (R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information.
Do not clear the DTC or reset the ECM/PCM before beginning this troubleshooting procedure until instructed to clear it. Clearing the DTC or resetting the ECM/PCM can delete valuable diagnostic information, even if the on-board snapshot and the freeze data is saved.
Inspect for aftermarket fuel system and engine components before beginning troubleshooting.
If any of the DTCs listed below are stored at the same time as P0172, troubleshoot those DTCs first, then recheck for P0172.
P0101, P0102, P0103: MAF sensor
P0106, P0107, P0108: MAP sensor
P0133, P1172, P1157, P2238, P2252, P2A00: A/F sensor (Sensor 1)
P0134, P0135: A/F sensor (Sensor 1) heater
P0137, P0138, P0139, P2270: Secondary HO2S (Sensor 2)
P0141: Secondary HO2S (Sensor 2) heater
P0401, P0404, P0406, P2413: EGR system
P0443, P0496: EVAP canister purge valve
P2646, P2647, P2648, P2649: VTEC system

1. Failure Confirmation-Idle:

- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral).
- 1. Monitor AF FB (ST FUEL TRIM) in the DATA LIST with the HDS.
Does AF FB (ST FUEL TRIM) read 0.79 or less at idle?
YES
AF FB (ST FUEL TRIM) is too low at idle. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK at idle. Go to step 2.

2. Failure Confirmation-On-board Snapshot:

- 1. Review the on-board snapshot and record the value of the parameters below when AF FB (ST FUEL TRIM) was at its lowest point.
| ENGINE SPEED |
| VEHICLE SPEED |
| REL TP SENSOR |
| APP SENSOR |
- 1. Drive the vehicle according to the conditions determined above.
Does AF FB (ST FUEL TRIM) read 0.79 or less under the on-board snapshot conditions?
YES
AF FB (ST FUEL TRIM) is too low under the conditions above. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK under the same conditions as the on-board snapshot. The system is normal at this time.

3. Check for MAF Sensor Debris:

- 1. Remove the MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Check for lint, bugs, and other foreign objects in the sensor.
Is there debris in the MAF sensor?
YES
Remove the debris from the MAF sensor and then use a vacuum cleaner to remove any debris from the air cleaner housing, then do the confirm repair procedure.
NO
Reinstall the MAF sensor/IAT sensor. Go to step 4.

4. Check Fuel Pressure:

- 1. Check the fuel pressure under the conditions when AF FB (ST FUEL TRIM) is 0.79 or less.
Is the fuel pressure between 380-430 kPa (3.9-4.4 kgf/cm 2 , 55-63 psi)?
YES
The fuel pressure is OK. Go to step 5.
NO
Replace the fuel tank unit - Refer to: Fuel Tank Unit Removal and Installation (K24Z7) , Fuel Tank Unit Removal and Installation (R18Z1) , then do the confirm repair procedure.

5. Check MAF Sensor Reading:

- 1. Start the engine and hold the engine speed at 3, 000 rpm until the radiator fan comes on.
- 1. Make sure the following conditions are met:
ECT Sensor 1 above 158°F (70°C)
A/T in P or N, M/T in neutral
All engine and electrical loads turned off
- 1. Monitor the ENGINE SPEED in the DATA LIST with the HDS, then hold the engine speed at 2, 500 rpm with the accelerator pedal held steady.
- 1. Check the MAF SENSOR in the DATA LIST with the HDS.
| Model | Specifications
| M/T | 5.3-6.5 g/s
| A/T | 5.3-6.5 g/s
Is there MAF sensor within specifications at 2, 500 rpm with the engine fully warm and all loads off?
YES
The MAF sensor is OK. Go to step 6.
NO
The MAF sensor is reading incorrectly. Inspect for a cracked, damaged, or incorrectly installed air cleaner assembly. If OK, replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the confirm repair procedure.

6. Check Engine Oil:

- 1. Start the engine, and let it idle.
- 1. Monitor AF FB (ST FUEL TRIM) in the DATA LIST with the HDS.
- 1. Gently pinch the PCV vacuum hose (A).
| Courtesy of HONDA, U.S.A., INC.
Does AF FB (ST FUEL TRIM) increase above 0.79 when the PCV hose is pinched?
YES
The engine oil is contaminated. Replace the engine oil and oil filter - Refer to: Engine Oil Filter Removal and Installation (K24Z7) , Engine Oil Filter Removal and Installation (Except K24Z7) , then do the confirm repair procedure.
NO
The engine oil is OK. Go to step 7.

7. Replace Fuel Injectors:

- 1. Replace the fuel injectors with known-good injectors. - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , Injector Removal and Installation (R18Z1 (CVT)) ('14-'15)
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 0.79 or less.
Is AF FB (ST FUEL TRIM) still 0.79 or less?
YES
The fuel injectors are not the cause of the low AF FB (ST FUEL TRIM) value. Go to step 8.
NO
The fuel injectors were causing the low AF FB (ST FUEL TRIM) value. Replace the original injectors - Refer to: Injector Removal and Installation (K24Z7) , Injector Removal and Installation (R18A9) , Injector Removal and Installation (R18Z1 (A/T, M/T)) , Injector Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the confirm repair procedure.

8. Replace A/F Sensor (Sensor 1)

- 1. Replace the A/F sensor (Sensor 1) with a known-good sensor. - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation(R18Z1 (CVT)) ('14-'15)
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 0.79 or less.
Is AF FB (ST FUEL TRIM) still 0.79 or less?
YES
The A/F sensor (Sensor 1) is not the cause of the low AF FB (ST FUEL TRIM) value. Go to step 9.
NO
The A/F sensor (Sensor 1) was causing the low AF FB (ST FUEL TRIM) value. Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the confirm repair procedure.

9. Replace Fuel: NOTE: There is an increased likelihood of a fuel related issue if the misfire appeared shortly after refueling. The MIL DIST (MIL Distance) value in the DATA LIST in combination with the fuel level may help you make this determination.

- 1. Remove the fuel tank unit and then drain and refill the fuel tank with known-good fuel.
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 0.79 or less.
Is AF FB (ST FUEL TRIM) still 0.79 or less?
YES
The fuel is not the cause of the low AF FB (ST FUEL TRIM) value. Substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the confirm repair procedure.
NO
The fuel was causing the low AF FB (ST FUEL TRIM) value. Do the repair confirm procedure.
Confirm Repair:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15)
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Operate the vehicle under the same conditions where AF FB (ST FUEL TRIM) was 0.79 or less.
Is AF FB (ST FUEL TRIM) still 0.79 or less?
YES
AF FB (ST FUEL TRIM) is too less. Go to step 3.
NO
AF FB (ST FUEL TRIM) is OK. Troubleshooting is complete.

## P1109 — page 18029

- Source file: pages/18029.html

DTC P1109: BARO Sensor Circuit Out of Range High (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1109 indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1109 indicated?
YES
If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update ECM Removal and Installation (K24Z7) ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update ECM Removal and Installation (K24Z7) ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1116 — page 18031

- Source file: pages/18031.html

DTC P1116: ECT Sensor 1 Range/Performance Problem (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If DTC P0111 is stored at the same time as DTC P1116, troubleshoot DTC P0111 first, then recheck for DTC P1116.

1. Connector visual check:

- 1. Check for poor connections or loose terminals at these locations:
ECT sensor 1
ECT sensor 2
Are the connections and terminals OK?
YES
The parts are OK. Go to step 2.
NO
Repair the connections or terminals, then do the repair verification.

2. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P1116 and P2183 indicated at the same time?
YES
Go to step 5.
NO
Only DTC P1116 is indicated. Go to step 3.

3. ECT sensor 1 performance check:

- 1. Start the engine, and let it idle for 10 minutes.
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 108°F (42°C) or less indicated?
YES
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
ECT sensor 1 is OK at this time. Go to step 4.

4. Problem verification:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow ECT sensor 1 to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of ECT SENSOR 1 in the DATA LIST with the HDS.
- 1. Compare the value of ECT SENSOR 1 and the ambient temperature.
Does the value of ECT SENSOR 1 differ 5.4°F (3°C) or more from the ambient temperature?
YES
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.

5. ECT sensor 1 performance check:

- 1. Start the engine, and let it idle for 10 minutes.
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 108°F (42°C) or less indicated?
YES
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
Go to step 6.

6. ECT sensor 2 performance check:

- 1. Let the engine idle for 10 minutes.
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 108°F (42°C) or less indicated?
YES
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) , ECT Sensor 2 Removal and Installation (R18Z1) , ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.
NO
Go to step 7.

7. Problem verification:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the sensors to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of ECT SENSOR 1 and ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Compare the value of ECT SENSOR 1 and the value of ECT SENSOR 2 to the ambient temperature individually.
Does either sensor differ more than 5.4°F (3°C) from the ambient temperature?
YES
Replace the sensor that differed more than 5.4°F (3°C) from the ambient temperature, then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1116 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1128 — page 18033

- Source file: pages/18033.html

DTC P1128: MAP Sensor Signal Lower Than Expected (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
Before you begin, check for poor connections or a blockage at the intake air duct.

1. MAP sensor performance check:

- 1. Turn the ignition switch to ON (II).
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is less than 54.1 kPa (16.0 inHg, 406 mmHg) or 1.61 V held for more than 5 seconds?
YES
Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 2.

2. Problem verification:

- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
Engine speed between 1, 000-4, 000 rpm
A/T in D, M/T in 3rd
Vehicle speed accelerated from 16 to 31 mph (25 to 50 km/h) under half throttle
- 1. Monitor the OBD STATUS for DTC P1128 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, go to step 2-2 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
Engine speed between 1, 000-4, 000 rpm
A/T in D, M/T in 3rd
Vehicle speed accelerated from 16 to 31 mph (25 to 50 km/h) under half throttle
- 1. Monitor the OBD STATUS for DTC P1128 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P1129 — page 18035

- Source file: pages/18035.html

DTC P1129: MAP Sensor Signal Higher Than Expected (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Vacuum leak check:

- 1. Check for vacuum leaks in these parts:
PCV valve
PCV hose
EVAP canister purge valve
Throttle body
Intake manifold
Brake booster
Brake booster hose
Are there any vacuum leaks?
YES
Repair or replace the parts with vacuum leaks, then do the repair verification.
NO
Go to step 2.

2. MAP sensor performance check:

- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check the MAP SENSOR in the DATA LIST with the HDS.
Is more than 36.9 kPa (11.0 inHg, 277 mmHg) or 1.1 V held for more than for 5 seconds?
YES
Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Problem verification:

- 1. Clear the DTC with the HDS.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
Engine speed between 1, 200-4, 800 rpm
A/T in D, M/T in 3rd
Vehicle speed between 55-75 mph (90-120 km/h) for 10 seconds, then decelerate (with throttle fully closed) for 5 seconds
- 1. Monitor the OBD STATUS for DTC P1129 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Replace the MAP sensor - Refer to: MAP Sensor Removal and Installation (K24Z7) , MAP Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the MAP sensor and the ECM/PCM. If the HDS indicates NOT COMPLETED, go to step 3-2 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
Engine speed between 1, 200-4, 800 rpm
A/T in D, M/T in 3rd
Vehicle speed between 55-75 mph (90-120 km/h) for 10 seconds, then decelerate (with throttle fully closed) for 5 seconds
- 1. Monitor the OBD STATUS for DTC P1129 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P1157 — page 18036

- Source file: pages/18036.html

DTC P1157: A/F Sensor (Sensor 1) AFS Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and wait 1 minute.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1157 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), the PGM-FI subrelay, and the ECM/PCM.

2. Open wire check (AFS+ line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| A/F sensor (Sensor 1) 4P connector
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | AFS+
| Test point 1 | ECM/PCM connector C (49P) No. 39 (RED)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 1 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The AFS+ wire is OK. Go to step 3.
NO
Repair an open in the AFS+ wire between the ECM/PCM (C39) and the A/F sensor (Sensor 1), then do the repair verification.

3. Open wire check (AFS- line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | AFS-
| Test point 1 | ECM/PCM connector C (49P) No. 40 (RED/YEL)
| Test point 2 | A/F sensor (Sensor 1) 4P connector No. 2 (RED/YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The AFS- wire is OK. Go to step 4.
NO
Repair an open in the AFS- wire between the ECM/PCM (C40) and the A/F sensor (Sensor 1), then do the repair verification.

4. A/F sensor (Sensor 1) check (substitution):

- 1. Substitute a known-good A/F sensor (Sensor 1).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1157 indicated?
YES
The A/F sensor (Sensor 1) is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1157 indicated?
YES
Check for poor connections or loose terminals at the A/F sensor (Sensor 1) and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1172 — page 18040

- Source file: pages/18040.html

DTC P1172: A/F Sensor (Sensor 1) Circuit Out of Range High (KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P1172 with the HDS.
Does the HDS indicate FAILED?
YES
Replace A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F Sensor (Sensor 1) and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-3 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Monitor the OBD STATUS for DTC P1172 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep idling until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P1297 — page 18045

- Source file: pages/18045.html

DTC P1297: ELD Circuit Low Voltage ('12-'13: KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the ELD in the DATA LIST with the HDS.
Is 72 A or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the under-hood fuse/relay box and the ECM/PCM.

2. Determine possible failure area (ELD, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Under-hood fuse/relay box 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the ELD in the DATA LIST with the HDS.
Is 72 A or more indicated?
YES
Go to step 3.
NO
Replace the ELD , then do the repair verification.

3. Shorted wire check (ELD line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Under-hood fuse/relay box 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | ELD
| Test point 1 | ECM/PCM connector A (49P) No. 34 (ORN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the ELD wire between the ECM/PCM (A34) and the under-hood fuse/relay box, then do the repair verification.
NO
The ELD wire is OK. Update the PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine.
- 1. Turn on the headlights.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1297 indicated?
YES
Check for poor connections or loose terminals at the under-hood fuse/relay box and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1298 — page 18051

- Source file: pages/18051.html

DTC P1298: ELD Circuit High Voltage ('13: KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the ELD in the DATA LIST with the HDS.
Is 0.2 A or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the under-hood fuse/relay box and the ECM/PCM.

2. Open wire check (IG1_ACG line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Under-hood fuse/relay box 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Under-hood fuse/relay box 3P connector: disconnected
| Test circuit | IG1_ACG
| Test point 1 | Under-hood fuse/relay box 3P connector No. 3 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The IG1_ACG wire is OK. Go to step 3.
NO
Check the No. B2 (10 A) fuse in the under-dash fuse/relay box. If the fuse is OK, repair an open in the IG1_ACG wire between the No. B2 (10 A) fuse and the under-hood fuse/relay box, then do the repair verification.

3. Open wire check (ELD line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Under-hood fuse/relay box 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | ELD
| Test point 1 | ECM/PCM connector A (49P) No. 34 (ORN)
| Test point 2 | Under-hood fuse/relay box 3P connector No. 1 (ORN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The ELD wire is OK. Go to step 4.
NO
Repair an open in the ELD wire between the ECM/PCM (A34) and the under-hood fuse/relay box, then do the repair verification.

4. Open wire check (GND line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Under-hood fuse/relay box 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | GND
| Test point 1 | Under-hood fuse/relay box 3P connector No. 2 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The GND wire is OK. Go to step 5.
NO
Repair an open in the GND wire between the under-hood fuse/relay box and G301, then do the repair verification.

5. ELD check (substitution):

- 1. Substitute a known-good ELD .
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1298 indicated?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the original ELD , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1298 indicated?
YES
Check for poor connections or loose terminals at the under-hood fuse/relay box and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1454 — page 18054

- Source file: pages/18054.html

DTC P1454: FTP Sensor Range/Performance Problem; DTC P2422: EVAP Canister Vent Shut Valve Stuck Closed Malfunction (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. EVAP system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the fuel fill cap, and wait 1 minute.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Go to step 2.
NO
Go to step 5.

2. Problem verification:

- 1. Install the fuel fill cap.
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Monitor the EVAP PC DUTY (purge control duty) in the DATA LIST with the HDS, while holding the engine speed at 3, 000 rpm with the EVAP PC DUTY at 20 % or more for at least 4 minutes, then let the engine idle.
- 1. Monitor the OBD STATUS for DTC P1454 and/or P2422 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 3.
NO
If the HDS indicates PASSED, there might be an intermittent failure, and the system is OK at this time. Check the on-board snapshot, and repeat the driving conditions when the DTC set (fuel level, engine speed, vehicle speed, and outside temperature). If no problem is found, check for poor connections or loose terminals between the FTP sensor and the ECM/PCM. Also check for a blockage in the FTP sensor vent port and vent hose, and the EVAP canister vent hose and drain joint. If the HDS indicates NOT COMPLETED, let the engine idle until a result comes on.

3. EVAP canister vent line check:

- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the EVAP canister vent hose (A) from the EVAP canister (B).
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Check for a blockage in the EVAP canister vent hose or drain tube joint, then do the repair verification.
NO
Go to step 4.

4. EVAP canister vent shut valve operation check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the EVAP canister vent shut valve from the EVAP canister - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) .
- 1. Connect the 2P connector to the EVAP canister vent shut valve.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Check the EVAP canister vent shut valve (A) operation.
| Courtesy of HONDA, U.S.A., INC.
Is the valve free of corrosion, and does it operate properly?
YES
The EVAP canister vent shut valve is OK. Go to step 5.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.

5. FTP sensor vent hose restriction check:

- 1. Disconnect the vent hose (A) from the FTP sensor (B).
| Courtesy of HONDA, U.S.A., INC.
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Check for a blockage in the FTP sensor vent hose, then do the repair verification.
NO
The FTP sensor vent hose is OK. Go to step 6.

6. FTP sensor port restriction check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the FTP sensor (A) from the EVAP canister with its connector connected.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
The FTP sensor port is OK. The EVAP canister has an internal blockage; replace the EVAP canister - Refer to: EVAP Canister Removal and Installation (K24Z7) , EVAP Canister Removal and Installation (R18Z1) , then do the repair verification.
NO
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Install the parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Monitor the OBD STATUS for DTC P1454 and/or P2422 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Hold the engine speed at 3, 000 rpm with the EVAP PC DUTY at 20% or more until a result comes on.

## P1458 — page 18056

- Source file: pages/18056.html

DTC P1458: FTP Sensor Circuit Range/Performance Problem (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If any of DTCs listed below are indicated at the same time as DTC P1458, troubleshoot those DTCs first, then recheck for P1458.
P0451, P0452, P0453, P1454: FTP sensor
P2422: EVAP canister vent shut valve
Do not start the engine during this troubleshooting.

1. Fuel level check:

- 1. Turn the ignition switch to ON (II).
- 1. Note the recorded on-board snapshot parameters with the HDS:
MAX Value of FTP SENSOR
MIN Value of FTP SENSOR
FUEL LEVEL
| Courtesy of HONDA, U.S.A., INC.
- 1. Check the FUEL LEVEL in the DATA LIST with the HDS.
Is the FUEL LEVEL less than it is on the on-board snapshot?
YES
Go to step 2.
NO
Drain the fuel until the FUEL LEVEL is less than it is on the on-board snapshot, then go to step 2.

2. Determine possible failure area (FTP sensor, others):

- 1. Take a snapshot with the HDS for 3 minutes without starting the engine.
- 1. Check the recorded snapshot parameters with the HDS:
MAX Value of FTP SENSOR (FINE)
MIN Value of FTP SENSOR (FINE)
| Courtesy of HONDA, U.S.A., INC.
Is the difference between the MAX and MIN values of the FTP SENSOR (FINE) about 0.67 kPa (0.2 inHg, 5 mmHg) or less?
YES
Go to step 3.
NO
Go to step 5.

3. FTP sensor performance check 1:

- 1. Check the FTP SENSOR (FINE) in the recorded snapshot with the HDS.
Does it vary 0.067 kPa (0.02 inHg, 0.5 mmHg) or more for 3 seconds?
YES
Go to step 4.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister vent shut valve, and the ECM/PCM. Also check for a blockage in the EVAP vent hose.

4. FTP sensor performance check 2:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) .
- 1. Reconnect the FTP sensor 3P connector.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR (FINE) in the DATA LIST with the HDS.
Does it vary 0.067 kPa (0.02 inHg, 0.5 mmHg) or more for 3 seconds?
YES
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the FTP sensor, the EVAP canister vent shut valve, and the ECM/PCM. Also check for a blockage in the EVAP vent hose.

5. Determine possible failure area (EVAP canister vent line, others):

- 1. Remove the fuel fill cap.
- 1. Check the FTP SENSOR (FINE) in the DATA LIST with the HDS.
Is the value about 0.67 kPa (0.2 inHg, 5 mmHg) or more, or about -0.67 kPa (-0.2 inHg, -5 mmHg) or less?
YES
Reinstall the fuel fill cap, then go to step 8.
NO
Go to step 6.

6. FTP sensor vent hose check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) .
- 1. Reconnect the FTP sensor 3P connector.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR (FINE) in the DATA LIST with the HDS.
Is the value about 0.67 kPa (0.2 inHg, 5 mmHg) or more, or about -0.67 kPa (-0.2 inHg, -5 mmHg) or less?
YES
Remove the blockage in the FTP sensor vent hose. Replace the hose if needed.
NO
Go to step 7.

7. FTP sensor port visual check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the FTP sensor 3P connector.
- 1. Check for a blockage or damage at the FTP sensor ports (EVAP canister side and vent side).
| Courtesy of HONDA, U.S.A., INC.
Is there any blockage or damage?
YES
Remove the blockage, and replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) if needed, then do the repair verification.
NO
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.

8. EVAP canister fresh air hose check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the fresh air hose (A) from the EVAP canister (B), then connect a vacuum pump/gauge (C) to the EVAP vent shut valve as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Try to apply vacuum to the hose valve (not more than 5.3 kPa (1.6 inHg, 40 mmHg)) while checking the FTP SENSOR (FINE) in the DATA LIST with the HDS.
NOTE: To avoid damaging the FTP sensor, do not apply more than 5.3 kPa (1.6 inHg, 40 mmHg) of vacuum.
Does the valve hold more than 1.3 kPa (0.4 inHg, 10 mmHg) of vacuum?
YES
The EVAP canister fresh air hose is OK. Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.
NO
Remove the blockage in the fresh air hose. Replace hose if needed, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connecters and the fresh air hose.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Take a snapshot with the HDS for 3 minutes without starting the engine.
- 1. Check the recorded snapshot parameters with the HDS:
MAX Value of FTP SENSOR (FINE)
MIN Value of FTP SENSOR (FINE)
| Courtesy of HONDA, U.S.A., INC.
Is the difference between the MAX and MIN values of the FTP SENSOR (FINE) about 0.67 kPa (0.2 inHg, 5 mmHg) or less?
YES
Troubleshooting is complete.
NO
Check for poor connections or loose terminals at the FTP sensor, the EVAP canister vent shut valve, and the ECM/PCM, then go to step 1.

## P145C — page 18058

- Source file: pages/18058.html

DTC P145C: EVAP System Purge Flow Malfunction (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If DTC P145C is indicated alone, do the troubleshooting for DTC P0496 and P0497 using the freeze data for P145C.
If DTC P0497 and P145C are stored at the same time, check for a poor connection, a blockage, or damage at the EVAP canister purge line between the EVAP canister purge valve and the EVAP canister. Also check for a stuck closed EVAP canister purge valve.
If any of the DTCs listed below are indicated at the same time as DTC P145C, troubleshoot those DTCs first, then recheck for P145C.
P0496, P0497: EVAP system purge flow

1. DTC check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P0496 or P0497 and P145C indicated at the same time?
YES
Go to the indicated DTC's troubleshooting.
NO
Go to the DTC P0496 troubleshooting - Refer to: DTC Troubleshooting: P0496 (KA KC KL models K24Z7) , DTC Troubleshooting: P0496 (KA KC KL models R18Z1) and the DTC P0497 troubleshooting - Refer to: DTC Troubleshooting: P0497 (KA KC KL models K24Z7) , DTC Troubleshooting: P0497 (KA KC KL models R18Z1) .

## P1549 — page 18060

- Source file: pages/18060.html

DTC P1549: Charging System High Voltage (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If a high voltage battery (24 V, etc.) is connected to the vehicle, this DTC can be stored.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine.
- 1. Check under these conditions:
A/C off
Headlights off
Rear window defogger off
- 1. Hold the engine speed at 3, 000 rpm (A/T in P or N, M/T in neutral) for 1 minute.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1549 indicated?
YES
Replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the alternator and the under-hood fuse/relay box.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine.
- 1. Check under these conditions:
A/C off
Headlights off
Rear window defogger off
- 1. Monitor the OBD STATUS for DTC P1549 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P154A — page 18062

- Source file: pages/18062.html

DTC P154A: Battery Sensor Internal Failure (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS, and wait 5 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P154A indicated?
YES
Replace the battery sensor .
NO
Check for poor connections or loose terminals at the battery sensor and the ECM/PCM.

## P1658 — page 18063

- Source file: pages/18063.html

DTC P1658: ETCS Control Relay ON Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Do the ETCS TEST with the HDS.
Is the RELAY circuit OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ETCS control relay and the ECM/PCM.
NO
The failure is duplicated. Go to step 2.

2. ETCS control relay test:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the ETCS control relay (A) from the under-dash fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Test the ETCS control relay - Refer to: Power Relay Test ('12-'13: except Natural Gas models) , Power Relay Test ('12-'13: Natural Gas models) .
Is the ETCS control relay OK?
YES
The ETCS control relay is OK. Go to step 3.
NO
Replace the ETCS control relay, then do the repair verification.

3. Shorted wire check (ETCS_RLY_CL- line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | ETCS_RLY_CL-
| Test point 1 | ECM/PCM connector A (49P) No. 48 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the ETCS_RLY_CL- wire between the ECM/PCM (A48) and the throttle body, then do the repair verification.
NO
The ETCS_RLY_CL- wire is OK. Go to step 4.

4. Shorted wire check (VB ETCS line to power):

- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ETCS control relay: disconnected
| | ECM/PCM connector A (49P): disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VB ETCS
| Test point 1 | ECM/PCM connector C (49P) No. 1 (WHT/GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Repair a short to power in the VB ETCS wire between the ECM/PCM (C1) and the ETCS control relay, then do the repair verification.
NO
The VB ETCS wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1658 indicated?
YES
Check for poor connections or loose terminals at the ETCS control relay and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1659 — page 18066

- Source file: pages/18066.html

DTC P1659: ETCS Control Relay OFF Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1659 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ETCS control relay and the ECM/PCM.

2. Determine possible failure area (VB ETCS line or +B_ETCS line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. A22 (15 A)
| Location | Under-hood fuse/relay box
Is the fuse OK?
YES
Go to step 3.
NO
Go to step 9.

3. ETCS control relay test:

- 1. Remove the ETCS control relay (A) from the under-dash fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Test the ETCS control relay - Refer to: Power Relay Test ('12-'13: except Natural Gas models) , Power Relay Test ('12-'13: Natural Gas models) .
Is the ETCS control relay OK?
YES
The ETCS control relay is OK. Go to step 4.
NO
Replace the ETCS control relay, then do the repair verification.

4. Determine possible failure area (under-dash fuse/relay box, others):

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ETCS control relay: disconnected
| Test circuit | FI_MAIN_RLY_OUT
| Test point 1 | ETCS control relay 4P socket No. 3
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 5.
NO
Replace the under-dash fuse/relay box - Refer to: Under-Dash Fuse/Relay Box Removal and Installation (KA models with Keyless Access System) , Under-Dash Fuse/Relay Box Removal and Installation (KA KX models without Keyless Access System) , then do the repair verification

5. Determine possible failure area (+B_ETCS line, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ETCS control relay: disconnected
| Test circuit | +B_ETCS
| Test point 1 | ETCS control relay 4P socket No. 1
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
Go to step 7.
NO
Go to step 6.

6. Open wire check (+B_ETCS line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Under-dash fuse/relay box connector D (32P)
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | Under-dash fuse/relay box connector D (32P): disconnected
| Test circuit | +B_ETCS
| Test point 1 | Under-dash fuse/relay box connector D (32P) No. 12 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The +B_ETCS wire is OK. Replace the under-dash fuse/relay box, then do the repair verification.
NO
Repair an open in the +B_ETCS wire between the No. A22 (15 A) fuse and the under-dash fuse/relay box, then do the repair verification.

7. Open wire check (VB ETCS, ETCS_RLY_OUT lines):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VB ETCS, ETCS_RLY_OUT
| Test point 1 | ECM/PCM connector C (49P) No. 1 (WHT/GRN)
| Test point 2 | ETCS control relay 4P socket No. 2
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VB ETCS/ETCS_RLY_OUT wires are OK. Go to step 8.
NO
Repair an open in the VB ETCS/ETCS_RLY_OUT wire between the ECM/PCM (C1) and the ETCS control relay, then do the repair verification.

8. Open wire check (ETCS_RLY_CL- line):

- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector C (49P): disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | ETCS_RLY_CL-
| Test point 1 | ECM/PCM connector A (49P) No. 48 (YEL)
| Test point 2 | ETCS control relay 4P socket No. 4
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The ETCS_RLY_CL- wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the ETCS_RLY_CL- wire between the ECM/PCM (A48) and the ETCS control relay, then do the repair verification.

9. Shorted wire check (VB ETCS, ETCS_RLY_OUT lines):

- 1. Remove the ETCS control relay (A) from the under-hood fuse/relay box.
| Courtesy of HONDA, U.S.A., INC.
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VB ETCS
| Test point 1 | ECM/PCM connector C (49P) No. 1 (WHT/GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VB ETCS/ETCS_RLY_OUT wire between the ECM/PCM (C1) and the ETCS control relay, then do the repair verification.
NO
The VB ETCS/ETCS_RLY_OUT wires are OK. Go to step 10.
 1. Determine possible failure area (+B_ETCS line, ECM/PCM):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | +B_ETCS
| Test point 1 | ETCS control relay 4P socket No. 1
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 11.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
 1. Shorted wire check (+B_ETCS line):

- 1. Disconnect the following connector.
| Under-dash fuse/ relay box connector D (32P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ETCS control relay: disconnected
| | ECM/PCM connector C (49P): disconnected
| | Under-dash fuse/ relay box connector D (32P): disconnected
| Test circuit | +B_ETCS
| Test point 1 | Under-dash fuse/relay box connector D (32P) No. 12 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the +B_ETCS wire between the No. A22 (15 A) fuse and the under-dash fuse/relay box, then do the repair verification.
NO
The +B_ETCS wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1658 indicated?
YES
Check for poor connections or loose terminals at the ETCS control relay and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1683 — page 18070

- Source file: pages/18070.html

DTC P1683: Throttle Valve Default Position Spring Performance Problem (Except K24Z7)
| Courtesy of HONDA, U.S.A., INC.
Do not insert your fingers into the installed throttle body when you turn the ignition switch to ON (II), or while the ignition switch is in ON (II). If you do, you will seriously injure your fingers if the throttle valve is activated.
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Turn the ignition switch to LOCK (0), and wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1683 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Throttle body default position spring check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the intake air duct from the throttle body.
- 1. Push the throttle valve closed as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Release the throttle valve.
Does the throttle valve return?
YES
Clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , then do the repair verification. If DTC is indicated, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0), and wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1683 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P1684 — page 18072

- Source file: pages/18072.html

DTC P1684: Throttle Valve Return Spring Performance Problem (Except K24Z7)
| Courtesy of HONDA, U.S.A., INC.
Do not insert your fingers into the installed throttle body when you turn the ignition switch to ON (II), or while the ignition switch is in ON (II). If you do, you will seriously injure your fingers if the throttle valve is activated.
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Turn the ignition switch to LOCK (0), and wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1684 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

2. Throttle body return spring check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the intake air duct from the throttle body.
- 1. Push the throttle valve open as shown in illustration.
| Courtesy of HONDA, U.S.A., INC.
- 1. Release the throttle valve.
Does the throttle valve return?
YES
Clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , then do the repair verification. If DTC is indicated, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0), and wait 10 seconds.
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P1684 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P16BB — page 18074

- Source file: pages/18074.html

DTC P16BB: Alternator B Terminal Circuit Low Voltage (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) , How to Troubleshoot the CVT System (CVT) ('14-'15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine.
- 1. Check under these conditions:
A/C on
Temperature control at maximum cool
Blower fan at maximum speed
Headlights on high beam
Rear window defogger on
- 1. Hold the engine speed at 2, 000 rpm (A/T in P or N, M/T in neutral) for 1 minute.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16BB indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the alternator and the under-hood fuse/relay box. Also check the battery performance .

2. +B terminal check:

- 1. Check for poor connections or loose terminals at the alternator and the under-hood fuse/relay box (+B line).
Are the connections and terminals OK?
YES
Go to step 3.
NO
Repair the connections or the terminals, then do the repair verification.

3. Open wire check (+B line):

- 1. Check for an open in the wire between the alternator and the under-hood fuse/relay box at the engine harness.
Is the harness OK?
YES
Replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) , then do the repair verification.
NO
Repair an open in the +B wire between the alternator and the under-hood fuse/relay box, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine.
- 1. Check under these conditions:
A/C on
Temperature control at maximum cool
Blower fan at maximum speed
Headlights on high beam
Rear window defogger on
- 1. Hold the engine speed at 2, 000 rpm (A/T in P or N, M/T in neutral) for 1 minute.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16BB indicated?
YES
Check for poor connections or loose terminals at the alternator and the under-hood fuse/relay box, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P16E2 — page 18075

- Source file: pages/18075.html

DTC P16E2: PGM-FI-ACG LIN Communication Error ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch ON (II).
- 1. Clear the DTC with the HDS, and wait for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E2 indicated?
YES
Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the alternator, the battery sensor, and the ECM/PCM.

2. DTC Check:

- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E3 indicated?
YES
Go to step 3.
NO
Go to step 5.

3. Determine possible failure area (LIN circuit shorted, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Battery sensor 2P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Battery sensor 2P connector: disconnected
| Test circuit | LIN
| Test point 1 | Battery sensor 2P connector No. 2 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 4.
NO
Go to step 6.

4. Shorted wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Continue to check for continuity between battery sensor 2P connector terminal No. 2 and body ground, while disconnecting these connectors, one at a time:
Alternator 1P connector
ECM/PCM connector B (49P)
Does continuity go away when one of the above connectors is disconnected?
YES
The LIN wire is OK. Replace the part that caused an open when it was disconnected, then do the repair verification.
NO
Repair short to ground in the LIN wire between the battery sensor, alternator, and the ECM/PCM (B16), then do the repair verification.

5. Open wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| Alternator 1P connector
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Alternator 1P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | LIN
| Test point 1 | ECM/PCM connector B (49P) No. 16 (TAN)
| Test point 2 | Alternator 1P connector No. 1 (TAN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The LIN wire is OK. Replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) , then do the repair verification.
NO
Repair an open in the LIN wire between the alternator and the ECM/PCM (B16), then do the repair verification.

6. Open wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| Alternator 1P connector
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Battery sensor 2P connector: disconnected
| | Alternator 1P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | LIN
| Test point 1 | ECM/PCM connector B (49P) No. 16 (TAN)
| Test point 2 | Alternator 1P connector No. 1 (TAN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The LIN wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) then do the repair verification.
NO
Repair an open in the LIN wire between the alternator and the ECM/PCM (B16), then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E2 indicated?
YES
Check for poor connections or loose terminals at the alternator, the battery sensor, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P16E3 — page 18078

- Source file: pages/18078.html

DTC P16E3: PGM-FI-Battery Sensor LIN Communication Error ('12-'13: except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
Check for loose terminals on the battery sensor (battery negative (-) terminal).

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS, and wait for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E3 indicated?
YES
Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the alternator, the battery sensor, and the ECM/PCM.

2. DTC Check:

- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E2 indicated?
YES
Go to step 3.
NO
Go to step 6.

3. Fuse check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Check the following fuse.
| Fuse | No. A18 (10 A)
| Location | Under-food fuse/relay box
Is the fuse OK?
YES
The fuse is OK. Go to step 4.
NO
Repair a short in the +B_HORN wire between the battery sensor and the under-hood fuse/relay box and replace the No. A18 (10 A) fuse, then do the repair verification.

4. Determine possible failure area (LIN circuit shorted, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Alternator 1P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Alternator 1P connector: disconnected
| Test circuit | LIN
| Test point 1 | Alternator 1P connector No. 1 (TAN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 5.
NO
Go to step 8.

5. Shorted wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Continue to check for continuity between the alternator 1P connector terminal and body ground, while disconnecting these connectors, one at a time:
Battery sensor 2P connector
ECM/PCM connector B (49P)
Does continuity go away when one of the above connectors is disconnected?
YES
The LIN wire is OK. Replace the part that caused an open when it was disconnected, then do the repair verification.
NO
Repair short to ground in the LIN wire between the battery sensor, alternator, and the ECM/PCM (B16), then do the repair verification.

6. Open wire check (+B_HORN line):

- 1. Disconnect the following connector.
| Battery sensor 2P connector
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Battery sensor 2P connector: disconnected
| Test circuit | +B_HORN
| Test point 1 | Battery sensor 2P connector No. 1 (WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there battery voltage?
YES
The +B_HORN wire is OK. Go to step 7.
NO
Repair an open in the +B_HORN wire between the battery sensor and No. A18 (10 A) fuse in the under-hood fuse/relay box. Also check the No. A18 (10 A) fuse, then do the repair verification.

7. Open wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Battery sensor 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | LIN
| Test point 1 | ECM/PCM connector B (49P) No. 16 (TAN)
| Test point 2 | Battery sensor 2P connector No. 2 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The LIN wire is OK. Replace the battery sensor , then do the repair verification.
NO
Repair an open in the LIN wire between the battery sensor and the ECM/PCM (B16), then do the repair verification.

8. Open wire check (LIN line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| Battery sensor 2P connector
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Alternator 1P connector: disconnected
| | Battery sensor 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | LIN
| Test point 1 | ECM/PCM connector B (49P) No. 16 (TAN)
| Test point 2 | Battery sensor 2P connector No. 2 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The LIN wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the LIN wire between the battery sensor and the ECM/PCM (B16), then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II)
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E3 indicated?
YES
Check for poor connections or loose terminals at the alternator, the battery sensor, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P16E4 — page 18082

- Source file: pages/18082.html

DTC P16E4: ACG High-temperature (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
This DTC is stored when the alternator temperature is high (about 320°F (160°C) or more).

1. Problem verification:

- 1. If the temperature of the engine and the alternator is high, open the hood to cool the engine compartment.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P16E4 indicated?
YES
Replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) .
NO
Check the cause of the high temperature, and repair it.

## P2096 — page 18108

- Source file: pages/18108.html

DTC P2096: Post Catalyst Fuel Trim System Too Lean ('13-'15: KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If any of the DTCs listed below are stored at the same time as DTC P2096, troubleshoot those DTCs first, then recheck for P2096.
P0133, P1172, P2A00: A/F sensor (Sensor 1)
P0134: A/F sensor (Sensor 1) heater
P0137, P0138, P0139, P2270: Secondary HO2S (Sensor 2)
P0141: Secondary HO2S (Sensor 2) heater
P0171, P0172: Fuel system
P0201, P0202, P0203, P0204: Fuel injector
P0300: Random misfire
P0301, P0302, P0303, P0304: No. 1 - No. 4 cylinder misfire
P0351, P0352, P0353, P0354: Ignition coil

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2096 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 2.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), secondary HO2S (Sensor 2), and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-4 and recheck.

2. Exhaust system check:

- 1. Check for leakage, clogging, or blockage in the exhaust system.
Is there leakage in the exhaust system between the A/F sensor (Sensor 1) and secondary HO2S (Sensor 2), or are there clogging or restrictions in the exhaust system?
YES
Repair or replace the exhaust system - Refer to: Exhaust Pipe and Muffler Removal and Installation (K24Z7) , Exhaust Pipe and Muffler Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Exhaust Pipe and Muffler Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
Go to step 3.

3. A/F sensor (Sensor 1) check (substitution):

- 1. Substitute a known-good A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) .
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2096 with the HDS.
Does the HDS indicate PASSED?
YES
Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates FAILED, replace secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Secondary HO2S Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 3-7 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2096 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test conditions until a result comes on.
OUT OF CONDITION
Meet the test conditions, and check the OBD STATUS again.

## P2097 — page 18110

- Source file: pages/18110.html

DTC P2097: Post Catalyst Fuel Trim System Too Rich ('13-'15: KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
If any of the DTCs listed below are stored at the same time as DTC P2097, troubleshoot those DTCs first, then recheck for P2097.
P0133, P1172, P2A00: A/F sensor (Sensor 1)
P0134: A/F sensor (Sensor 1) heater
P0137, P0138, P0139, P2270: Secondary HO2S (Sensor 2)
P0141: Secondary HO2S (Sensor 2) heater
P0171, P0172: Fuel system
P0201, P0202, P0203, P0204: Fuel injector
P0300: Random misfire
P0301, P0302, P0303, P0304: No. 1 - No. 4 cylinder misfire
P0351, P0352, P0353, P0354: Ignition coil

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2097 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 2.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), secondary HO2S (Sensor 2), and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-4 and recheck.

2. Exhaust system check:

- 1. Check for leakage, clogging, or blockage in the exhaust system.
Is there leakage in the exhaust system between the A/F sensor (Sensor 1) and secondary HO2S (Sensor 2), or are there clogging or restrictions in the exhaust system?
YES
Repair or replace the exhaust system - Refer to: Exhaust Pipe and Muffler Removal and Installation (K24Z7) , Exhaust Pipe and Muffler Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Exhaust Pipe and Muffler Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
Go to step 3.

3. A/F sensor (Sensor 1) check (substitution):

- 1. Substitute a known-good A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) .
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2097 with the HDS.
Does the HDS indicate PASSED?
YES
Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification.
NO
If the HDS indicates FAILED, replace secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Secondary HO2S Removal and Installation (R18Z1 (CVT)) ('14-'15) , then do the repair verification. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 3-7 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14-'15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed below 4, 000 rpm
Vehicle speed between 15-75 mph (25-120 km/h) for 1 minute or more
- 1. Monitor the OBD STATUS for DTC P2097 with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test conditions until a result comes on.
OUT OF CONDITION
Meet the test conditions, and check the OBD STATUS again.

## P2101 — page 18112

- Source file: pages/18112.html

DTC P2101: ETCS Malfunction ('12-'13: except K24Z7)
| Courtesy of HONDA, U.S.A., INC.
Do not insert your fingers into the installed throttle body when you turn the ignition switch to ON (II), or while the ignition switch is in ON (II). If you do, you will seriously injure your fingers if the throttle valve is activated.
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. ETCS system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the ETCS TEST with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2101 indicated?
YES
The failure is duplicated. Go to step 3.
NO
The failure is not duplicated. Go to step 2.

2. Problem verification:

- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
APP SENSOR
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2101 indicated?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM.

3. Determine possible failure area (throttle body, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the intake air duct from the throttle body.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Visually check the throttle valve operation while doing the ETCS TEST in the INSPECTION MENU with the HDS.
Does the throttle valve operate smoothly?
YES
Clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , then do the repair verification. If DTC is indicated, replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Go to step 4.

4. Open wire check (MTR_1 line, MTR_2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Throttle body 6P connector No. 1 (YEL/RED)
| Terminal B | Throttle body 6P connector No. 2 (YEL/GRN)
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| | Throttle body 6P connector No. 1 and No. 2: jumped
| Test circuit | MTR_1, MTR_2
| Test point 1 | ECM/PCM connector C (49P) No. 3 (YEL/GRN)
| Test point 2 | ECM/PCM connector C (49P) No. 4 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The MTR_1 and MTR_2 wires are OK. Go to step 5.
NO
Repair an open in the MTR_1 wire or MTR_2 wire between the ECM/PCM (C3, C4) and the throttle body, then do the repair verification.

5. Throttle body internal circuit check:

- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. At the throttle body side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | MTR_1, MTR_2
| Test point 1 | Throttle body 6P connector No. 1 (throttle body side)
| Test point 2 | Throttle body 6P connector No. 2 (throttle body side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 1.0 kΩ or more?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
The throttle body internal circuit is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or replace the ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Test-drive the vehicle for several minutes in the range of these recorded freeze data parameters:
ENGINE SPEED
VEHICLE SPEED
APP SENSOR
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2101 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , go to step 1. If the ECM/PCM was updated, substitute a known-good ECM/PCM , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update ECM Removal and Installation (K24Z7) ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2118 — page 18115

- Source file: pages/18115.html

DTC P2118: Throttle Actuator Current Range/Performance Problem ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Slowly press the accelerator pedal to the floor.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2118 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) .

2. Open wire check (SG3 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | SG3
| Test point 1 | ECM/PCM connector C (49P) No. 14 (BLU)
| Test point 2 | Throttle body 6P connector No. 3 (BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG3 wire is OK. Go to step 3.
NO
Repair an open in the SG3 wire between the ECM/PCM (C14) and the throttle body, then do the repair verification.

3. Shorted wire check (MTR_1 line to MTR_2 line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | MTR_1, MTR_2
| Test point 1 | Throttle body 6P connector No. 1 (YEL/RED)
| Test point 2 | Throttle body 6P connector No. 2 (YEL/GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the MTR_1 and MTR_2 wires between the throttle body and the ECM/PCM (C3, C4), then do the repair verification.
NO
Go to step 4.

4. Shorted wire check (MTR_1, MTR_2 line to ground):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | MTR_1
| Test point 1 | Throttle body 6P connector No. 2 (YEL/GRN)
| Test point 2 | Body ground
| Test circuit | MTR_2
| Test point 1 | Throttle body 6P connector No. 1 (YEL/RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short to ground in the MTR_1, MTR_2 wire between the throttle body and the ECM/PCM (C3, C4), then do the repair verification.
NO
The MTR_1, MIR_2 wires are OK. Go to step 5.

5. Throttle body internal circuit check (short in wires):

- 1. At the throttle body side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | MTR_1, MTR_2
| Test point 1 | Throttle body 6P connector No. 1 (throttle body side)
| Test point 2 | Throttle body 6P connector No. 2 (throttle body side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 1.2-1.0 k--
YES
Go to step 6
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.

6. Throttle body internal circuit check (short to ground):

- 1. At the throttle body side, measure the resistance between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | MTR_1
| Test point 1 | Throttle body 6P connector No. 2 (throttle body side)
| Test point 2 | Body ground
| Test circuit | MTR_2
| Test point 1 | Throttle body 6P connector No. 1 (throttle body side)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
The throttle body internal circuit is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or replace the ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Slowly press the accelerator pedal to the floor.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2118 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2122 — page 18118

- Source file: pages/18118.html

DTC P2122: APP Sensor A (TP Sensor D) Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR A in the DATA LIST with the HDS.
Is there about 0.2 V or less?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the APP sensor and the ECM/PCM.

2. Determine possible failure area (VCC4 line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| APP sensor 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | APP sensor 6P connector: disconnected
| Test circuit | VCC4, SG4
| Test point 1 | APP sensor 6P connector No. 5 (BLU)
| Test point 2 | APP sensor 6P connector No. 6 (BRN)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 3.
NO
Go to step 6.

3. Shorted wire check (APS1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | APS1
| Test point 1 | APP sensor 6P connector No. 4 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the APS1 wire between the ECM/PCM (A28) and the APP sensor, then do the repair verification.
NO
The APS1 wire is not shorted. Go to step 4.

4. Open wire check (APS1 line):

- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 4 (YEL)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 4: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | APS1
| Test point 1 | ECM/PCM connector A (49P) No. 28 (YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The APS1 wire is OK. Go to step 5.
NO
Repair an open in the APS1 wire between the ECM/PCM (A28) and the APP sensor, then do the repair verification.

5. ECM/PCM internal circuit check:

- 1. Reconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 4 (YEL)
| Terminal B | APP sensor 6P connector No. 6 (BRN)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR A in the DATA LIST with the HDS.
Is there 0.2 V or less?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the APP sensor, - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) then do the repair verification.

6. Open wire check (VCC4 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 6 (BRN)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 6: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VCC4
| Test point 1 | ECM/PCM connector A (49P) No. 36 (BRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC4 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC4 wire between the ECM/PCM (A36) and the APP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2122 indicated?
YES
Check for poor connections or loose terminals at the APP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2123 — page 18121

- Source file: pages/18121.html

DTC P2123: APP Sensor A (TP Sensor D) Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR A in the DATA LIST with the HDS.
Is there about 4.9 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the APP sensor and the ECM/PCM.

2. Determine possible failure area (APP sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| APP sensor 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | APP sensor 6P connector: disconnected
| Test circuit | VCC4, SG4
| Test point 1 | APP sensor 6P connector No. 5 (BLU)
| Test point 2 | APP sensor 6P connector No. 6 (BRN)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the APP sensor - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Open wire check (SG4 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 5 (BLU)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 5: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | SG4
| Test point 1 | ECM/PCM connector A (49P) No. 46 (BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG4 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG4 wire between the ECM/PCM (A46) and the APP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2123 indicated?
YES
Check for poor connections or loose terminals at the APP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P065A — page 18461

- Source file: pages/18461.html

DTC P065A: ACG No Charging Malfunction (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. DTC check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0562 indicated at the same time?
YES
Go to DTC P0562 troubleshooting - Refer to: DTC Troubleshooting: P0562 (K24Z7) , DTC Troubleshooting: P0562 (Except K24Z7) .
NO
Go to step 2.

2. Drive belt check:

- 1. Inspect the drive belt - Refer to: Drive Belt Inspection (K24Z7) , Drive Belt Inspection (Except K24Z7) .
Is the drive belt OK?
YES
Repair or replace the alternator - Refer to: Alternator Overhaul (K24Z7) , Alternator Removal and Installation (K24Z7) , Alternator Removal and Installation (Except K24Z7) , Alternator Overhaul (Except K24Z7) .
NO
Replace the drive belt - Refer to: Drive Belt Removal and Installation (K24Z7) , Drive Belt Removal and Installation (Except K24Z7) .

## P0685 — page 18463

- Source file: pages/18463.html

DTC P0685: ECM/PCM Power Control Circuit/Internal Circuit Malfunction (Except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
If the problem doesn't return after you clear the DTC, or if this DTC is stored intermittently, check for loose terminals at the IGP line connectors before replacing the ECM/PCM.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle for 30 seconds.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0685 indicated?
YES
The failure is duplicated. Update the PCM, substitute a known-good PCM, if it does not have the latest software, or DTC P0641 goes away and the PCM was updated, troubleshooting is complete. If DTC P0641 goes away and the PCM was substituted- Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation(Except K24Z7 and R18Z1(CVT)) , then do the repair verification.
NO
Intermittent failure. Check the following items:
PGM-FI main relay 1
PGM-FI main fuse
Poor connections or loose terminals between the ECM/PCM and the under-hood fuse/relay box
Poor or loose ECM/PCM ground terminals and connections
Poor or loose battery cables and connections
Check the battery
Repair verification:
- 1. Start the engine, and let it idle for 30 seconds.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0685 indicated?
YES
If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P06A8 — page 18464

- Source file: pages/18464.html

DTC P06A8: Internal VCC Power Malfunction (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P06A8 indicated?
YES
Replace the ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) .
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ECM/PCM.

## P2183 — page 18472

- Source file: pages/18472.html

DTC P2183: ECT Sensor 2 Circuit Range/Performance Problem (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
If DTC P0111 is stored at the same time as DTC P2183, troubleshoot DTC P0111 first, then recheck for DTC P2183.

1. Connector visual check:

- 1. Check for poor connections or loose terminals at these sensors:
ECT sensor 1
ECT sensor 2
Are the connections and terminals OK?
YES
Go to step 2.
NO
Repair the connections or terminals, then do the repair verification.

2. PGM-FI system check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC P1116 and P2183 indicated at the same time?
YES
Go to step 5.
NO
Only DTC P2183 is indicated. Go to step 3.

3. ECT sensor 2 performance check:

- 1. Start the engine, and let it idle for 10 minutes.
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 120°F (49°C) or less indicated?
YES
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) , ECT Sensor 2 Removal and Installation (R18Z1) , ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.
NO
Go to step 4.

4. Problem verification:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow ECT sensor 2 to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Compare the value of ECT SENSOR 2 and the ambient temperature.
Does the value of ECT SENSOR 2 differ 5.4°F (3°C) or more from the ambient temperature?
YES
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) , ECT Sensor 2 Removal and Installation (R18Z1) , ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.

5. ECT sensor 1 performance check:

- 1. Start the engine, and let it idle for 10 minutes.
- 1. Check ECT SENSOR 1 in the DATA LIST with the HDS.
Is about 120°F (49°C) or less indicated?
YES
Replace ECT sensor 1 - Refer to: ECT Sensor 1 Removal and Installation (K24Z7) , ECT Sensor 1 Removal and Installation (R18Z1) , ECT Sensor 1 Removal and Installation (R18A9) , then do the repair verification.
NO
ECT sensor 1 is OK at this time. Go to step 6.

6. ECT sensor 2 performance check:

- 1. Let the engine idle for 10 minutes.
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 120°F (49°C) or less indicated?
YES
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) , ECT Sensor 2 Removal and Installation (R18Z1) , ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.
NO
Go to step 7.

7. Problem verification:

- 1. Turn the ignition switch to LOCK (0).
- 1. Allow the sensors to cool to the ambient temperature.
- 1. Note the ambient temperature.
- 1. Turn the ignition switch to ON (II).
- 1. Quickly note the value of ECT SENSOR 1 and ECT SENSOR 2 in the DATA LIST with the HDS.
- 1. Compare the value of ECT SENSOR 2 and the ambient temperature.
Does either sensor differ more than 5.4°F (3°C) from the ambient temperature?
YES
Replace the sensor that differed more than 5.4°F (3°C) from the ambient temperature, then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2183 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 1, ECT sensor 2, and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2195 — page 18475

- Source file: pages/18475.html

DTC P2195: A/F Sensor (Sensor 1) Signal Stuck Lean (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
If the vehicle was out of fuel and the engine stalled before this DTC was stored, refuel and clear the DTC with the HDS.
If DTC P2101, P2118, P2135, P2138, P2176, or a combination of P2122 and P2127, P2122, and P2138, or P2127 and P2138 is stored at the same time, troubleshoot them first, then recheck for DTC P2195.

1. MAF sensor/IAT sensor visual check:

- 1. Remove the MAF sensor/IAT sensor. - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7)
- 1. Check for dirt or debris in the MAF sensor/IAT sensor.
Is it dirty?
YES
Remove the debris. If needed, replace the MAF sensor/IAT sensor - Refer to: MAF Sensor/IAT Sensor Removal and Installation (K24Z7) , MAF Sensor/IAT Sensor Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 2.

2. A/F sensor (Sensor 1) installation check:

- 1. Check the installation of the A/F sensor (Sensor 1).
Is the A/F sensor loose or disconnected from the exhaust pipe?
YES
Reinstall the A/F sensor (Sensor 1), then do the repair verification.
NO
Go to step 3.

3. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2195 indicated?
YES
Check for poor connections or loose terminals at the A/F sensor (Sensor 1) and the ECM/PCM. Update the ECM/PCM if it does not have the latest software - Refer to: ECM/PCM Update , or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1) and the ECM/PCM.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Monitor the OBD STATUS for DTC P2195 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep the test condition until a result comes on.

## P2227 — page 18476

- Source file: pages/18476.html

DTC P2227: BARO Sensor Circuit Range/Performance Problem (KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
If DTC P0107, P0108, P1128, and/or P1129 are stored at the same time as DTC P2227, troubleshoot those DTCs first, then recheck for DTC P2227.

1. BARO Sensor performance check:

- 1. Turn the ignition switch to ON (II), and wait 2 seconds.
- 1. Check the BARO SENSOR in the DATA LIST with the HDS.
Is about 101 kPa (29.9 inHg, 760 mmHg), or about 2.9 V at sea level indicated?
YES
Go to step 2.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

2. Problem verification:

- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
REL TP SENSOR between 13 and 35 degrees for 3 seconds
- 1. Monitor the OBD STATUS for DTC P2227 with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 3.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. If the HDS indicates NOT COMPLETED, go to step 2-2 and recheck.

3. Intake air system check:

- 1. Check the intake air system for clogging or restrictions (foreign material, dirty air cleaner element, etc.).
Is the intake air system clogged or restricted?
YES
Remove the clog or restriction, then do the repair verification.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
REL TP SENSOR between 13 and 35 degrees for 3 seconds
- 1. Monitor the OBD STATUS for DTC P2227 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P2228 — page 18477

- Source file: pages/18477.html

DTC P2228: BARO Sensor Circuit Low Voltage (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the BARO SENSOR in the DATA LIST with the HDS.
Is about 53 kPa (15.6 inHg, 397 mmHg), or 1.31 V or less indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2228 indicated?
YES
Check for poor connections or loose terminals at the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2229 — page 18478

- Source file: pages/18478.html

DTC P2229: BARO Sensor Circuit High Voltage (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check the BARO SENSOR in the DATA LIST with the HDS.
Is about 160 kPa (47.2 inHg, 1, 200 mmHg), or 4.49 V or more indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2229 indicated?
YES
Check for poor connections or loose terminals at the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2270 — page 18480

- Source file: pages/18480.html

DTC P2270: Secondary HO2S (Sensor 2) Circuit Signal Stuck Lean (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed between 1, 500-3, 000 rpm
Drive 30 seconds or more
- 1. Monitor the OBD STATUS for DTC P2270 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the secondary HO2S (Sensor 2) - Refer to: Secondary HO2S Removal and Installation (K24Z7) , Secondary HO2S Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , Secondary HO2S Removal and Installation (R18Z1 (CVT)) ('14 '15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at secondary HO2S (Sensor 2) and the ECM/PCM. If the HDS indicates NOT COMPLETED, go to step 1-3 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 4th
Engine speed between 1, 500-3, 000 rpm
Drive 30 seconds or more
- 1. Monitor the OBD STATUS for DTC P2270 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
NOT COMPLETED
Meet the test condition, and check the OBD STATUS again.

## P2610 — page 18481

- Source file: pages/18481.html

DTC P2610: ECM/PCM Ignition Off Internal Timer Malfunction (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2610 indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
Repair verification:
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2610 indicated?
YES
If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2A00 — page 18486

- Source file: pages/18486.html

DTC P2A00: A/F Sensor (Sensor 1) Circuit Range/Performance Problem (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 3rd
Vehicle speed between 25-55 mph (40-90 km/h) for 5 minutes
Drive at a steady speed between 55-75 mph (88- 120 km/h) for 10 seconds, then decelerate (with throttle fully closed) for 4 seconds or more
- 1. Monitor the OBD STATUS for DTC P2A00 with the HDS.
Does the HDS indicate FAILED?
YES
Replace the A/F sensor (Sensor 1) - Refer to: , A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) A/F Sensor Removal and Installation (R18Z1 (CVT)) ('14 '15) , then do the repair verification.
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1) and the ECM/PCM. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 1-3 and recheck.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 158°F (70°C)
A/T in D, M/T in 3rd
Vehicle speed between 25-55 mph (40-90 km/h) for 5 minutes
Drive at a steady speed between 55-75 mph (88- 120 km/h) for 10 seconds, then decelerate (with throttle fully closed) for 4 seconds or more
- 1. Monitor the OBD STATUS for DTC P2A00 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep driving until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## U0029 — page 18487

- Source file: pages/18487.html

DTC U0029: F-CAN Malfunction (BUS-OFF (ECM/PCM)) (Except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0029 indicated?
YES
The failure is duplicated. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Intermittent failure, the system is OK at this time.
Repair verification:
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0029 indicated?
YES
If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2127 — page 18493

- Source file: pages/18493.html

DTC P2127: APP Sensor B (TP Sensor E) Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR B in the DATA LIST with the HDS.
Is there about 0.2 V or less?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the APP sensor and the ECM/PCM.

2. Determine possible failure area (VCC5 line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| APP sensor 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON(II)
| | APP sensor 6P connector: disconnected
| Test circuit | VCC5, SG5
| Test point 1 | APP sensor 6P connector No. 2 (LT BLU)
| Test point 2 | APP sensor 6P connector No. 3 (GRY)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 3.
NO
Go to step 6 .

3. Shorted wire check (APS2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | APS2
| Test point 1 | APP sensor 6P connector No. 1 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the APS2 wire between the ECM/PCM (A29) and the APP sensor, then do the repair verification.
NO
The APS2 wire is not shorted. Go to step 4.

4. Open wire check (APS2 line):

- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 1 (PUR)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 1: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | APS2
| Test point 1 | ECM/PCM connector A (49P) No. 29 (PUR)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The APS2 wire is OK. Go to step 5.
NO
Repair an open in the APS2 wire between the ECM/PCM (A29) and the APP sensor, then do the repair verification.

5. ECM/PCM internal circuit check:

- 1. Reconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 1 (PUR)
| Terminal B | APP sensor 6P connector No. 3 (GRY)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR B in the DATA LIST with the HDS.
Is there about 0.2 V or less?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the APP sensor - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) , then do the repair verification.

6. Open wire check (VCC5 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 3 (GRY)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 3: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | VCC5
| Test point 1 | ECM/PCM connector A (49P) No. 35 (GRY)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC5 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC5 wire between the ECM/PCM (A35) and the APP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2127 indicated?
YES
Check for poor connections or loose terminals at the APP Sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2128 — page 18495

- Source file: pages/18495.html

DTC P2128: APP Sensor B (TP Sensor E) Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check APP SENSOR B in the DATA LIST with the HDS.
Is there about 4.0 V or more?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the APP sensor and the ECM/PCM.

2. Determine possible failure area (APP sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| APP sensor 6P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | APP sensor 6P connector: disconnected
| Test circuit | VCC5, SG5
| Test point 1 | APP sensor 6P connector No. 2 (LT BLU)
| Test point 2 | APP sensor 6P connector No. 3 (GRY)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Replace the APP sensor - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Open wire check (SG5 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | APP sensor 6P connector No. 2 (LT BLU)
| Terminal B | Body ground
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | APP sensor 6P connector: disconnected
| | APP sensor 6P connector No. 2: jumped to body ground
| | ECM/PCM connector A (49P): disconnected
| Test circuit | SG5
| Test point 1 | ECM/PCM connector A (49P) No. 45 (LT BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG5 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG5 wire between the ECM/PCM (A45) and the APP sensor, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2128 indicated?
YES
Check for poor connections or loose terminals at the APP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2135 — page 18497

- Source file: pages/18497.html

DTC P2135: TP Sensor A/B Incorrect Voltage Correlation ('12-'13: except K24Z7)
| Courtesy of HONDA, U.S.A., INC.
Do not insert your fingers into the installed throttle body when you turn the ignition switch to ON (II), or while the ignition switch is in ON (II). If you do, you will seriously injure your fingers if the throttle valve is activated.
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the ETCS TEST with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2135 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECMPCM.

2. Throttle body operation check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the intake air duct from the throttle body.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Visually check the throttle valve operation.
Does the valve temporarily move to its fully closed position?
YES
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
NO
Go to step 3.

3. Determine possible failure area (ECM/PCM, other):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| Test circuit | THL1, THL2
| Test point 1 | ECM/PCM connector C (49P) No. 30 (RED/BLK)
| Test point 2 | ECM/PCM connector C (49P) No. 31 (YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 4.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Shorted wire check (THL1 line to THL2 line):

- 1. Disconnect the following connector.
| Throttle body 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| Test circuit | THL1, THL2
| Test point 1 | ECM/PCM connector C (49P) No. 30 (RED/BLK)
| Test point 2 | ECM/PCM connector C (49P) No. 31 (YEL)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the THL1 wire to THL2 wire between ECM/PCM connector terminals C30 (THL1 line) and C31 (THL2 line), then do the repair verification.
NO
The THL1 and THL2 wires are OK. Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2135 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2138 — page 18499

- Source file: pages/18499.html

DTC P2138: APP Sensor A/B (TP Sensor D/E) Incorrect Voltage Correlation ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Press the accelerator pedal to the floor.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2138 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the APP sensor and the ECM/PCM.

2. APP sensor signal check:

- 1. Check APP SENSOR A and APP SENSOR B in the DATA LIST with the HDS
Are they the same voltage?
YES
Go to step 3.
NO
Replace the APP sensor - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) , then do the repair verification.

3. Determine possible failure area (ECM/PCM, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| Test circuit | APS1, APS2
| Test point 1 | ECM/PCM connector A (49P) No. 28 (YEL)
| Test point 2 | ECM/PCM connector A (49P) No. 29 (PUR)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 4.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.

4. Shorted wire check (APS1 line to APS2 line):

- 1. Disconnect the following connector.
| APP sensor 6P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | APP sensor 6P connector: disconnected
| Test circuit | APS1, APS2
| Test point 1 | ECM/PCM connector A (49P) No. 28 (YEL)
| Test point 2 | ECM/PCM connector A (49P) No. 29 (PUR)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the APS1 wire to APS2 wire between ECM/PCM connector terminals A28 (APS1 line) and A29 (APS2 line), then do the repair verification.
NO
The APS1 and APS2 wires are OK. Replace the APP sensor - Refer to: Accelerator Pedal Module Removal and Installation (K24Z7) , Accelerator Pedal Module Removal and Installation (Except K24Z7) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Press the accelerator pedal to the floor.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2138 indicated?
YES
Check for poor connections or loose terminals at the APP sensor and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2176 — page 18501

- Source file: pages/18501.html

DTC P2176: Throttle Actuator Control System Idle Position Not Learned ('12-'13: except K24Z7)
| Courtesy of HONDA, U.S.A., INC.
Do not insert your fingers into the installed throttle body when you turn the ignition switch to ON (II), or while the ignition switch is in ON (II). If you do, you will seriously injure your fingers if the throttle valve is activated.
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
If DTC P2135 is stored at the same time as DTC P2176, troubleshoot DTC P2135 first, then recheck for DTC P2176.

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II), and wait 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2176 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then clean the throttle body.

2. Throttle body operation check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the intake air duct from the throttle body.
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Visually check the throttle valve operation while doing the ETCS TEST in the INSPECTION MENU with the HDS.
Does the throttle valve move to its fully closed position?
YES
Go to step 3.
NO
Go to step 4.

3. Throttle body condition check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) .
- 1. Check for sludge or carbon on the throttle valve.
Is there sludge or carbon on the throttle valve?
YES
Clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , then do the repair verification.
NO
Replace the throttle body - Refer to: Throttle Body Removal and Installation (K24Z7) , Throttle Body Disassembly and Reassembly (K24Z7) , Throttle Body Removal and Installation (R18Z1) , Throttle Body Removal and Installation (R18A9) , Throttle Body Disassembly and Reassembly (Except K24Z7) , then do the repair verification.

4. Open wire check (MTR1 line, MTR2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector C (49P)
| Throttle body 6P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | Throttle body 6P connector No. 1 (YEL/RED)
| Terminal B | Throttle body 6P connector No. 2 (YEL/GRN)
| Courtesy of HONDA, U.S.A., INC.
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| | Throttle body 6P connector: disconnected
| | Throttle body 6P connector No. 1 and No. 2: jumped
| Test circuit | MTR1, MTR2
| Test point 1 | ECM/PCM connector C (49P) No. 3 (YEL/GRN)
| Test point 2 | ECM/PCM connector C (49P) No. 4 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The MTR1 and MTR2 wires are OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the MTR1 wire and/or the MTR2 wire between the ECM/PCM (C3, C4) and the throttle body, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2176 indicated?
YES
Check for poor connections or loose terminals at the throttle body and the ECM/PCM, then clean the throttle body - Refer to: Throttle Body Cleaning (K24Z7) , Throttle Body Cleaning (Except K24Z7) , and go to step 1. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2184 — page 18504

- Source file: pages/18504.html

DTC P2184: ECT Sensor 2 Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 2 and the ECM/PCM.

2. ECT sensor 2 check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| ECT Sensor 2 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 356°F (180°C) or more, or 0.08 V or less indicated?
YES
Go to step 3.
NO
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) ECT Sensor 2 Removal and Installation (R18Z1) ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.

3. Shorted wire check (TW2_SENSOR line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT Sensor 2 2P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | TW2_SENSOR
| Test point 1 | ECT sensor 2 2P connector No. 2 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the TW2_SENSOR wire between the ECM/PCM (A44) and ECT sensor 2, then do the repair verification.
NO
The TW2_SENSOR wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2184 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 2 and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2185 — page 18506

- Source file: pages/18506.html

DTC P2185: ECT Sensor 2 Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 40°F (40°C) or less, or 4.92 V or more indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at ECT sensor 2 and the ECM/PCM.

2. Determine of possible failure area (ECT sensor 2, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| ECT sensor 2 2P connector
- 1. Connect terminals A and B with a jumper wire.
| Terminal A | ECT sensor 2 2P connector No. 1 (BLK)
| Terminal B | ECT sensor 2 2P connector No. 2 (GRN)
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check ECT SENSOR 2 in the DATA LIST with the HDS.
Is about 40°F (40°C) or less, or 4.92 V or more indicated?
YES
Go to step 3.
NO
Replace ECT sensor 2 - Refer to: ECT Sensor 2 Removal and Installation (K24Z7) , ECT Sensor 2 Removal and Installation (R18Z1) , ECT Sensor 2 Removal and Installation (R18A9) , then do the repair verification.

3. Determine possible failure area (TW2_SENSOR line, SG6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the jumper wire.
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ECT sensor 2 2P connector: disconnected
| Test circuit | TW2_SENSOR
| Test point 1 | ECT sensor 2 2P connector No. 2 (GRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (SG6 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT sensor 2 2P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | SG6
| Test point 1 | ECM/PCM connector A (49P) No. 10 (BLK)
| Test point 2 | ECT sensor 2 2P connector No. 1 (BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The SG6 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the SG6 wire between the ECM/PCM (A10) and ECT sensor 2, then do the repair verification.

5. Open wire check (TW2_SENSOR line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector A (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECT sensor 2 2P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| Test circuit | TW2_SENSOR
| Test point 1 | ECM/PCM connector A (49P) No. 44 (GRN)
| Test point 2 | ECT sensor 2 2P connector No. 2 (GRN)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The TW2_SENSOR wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the TW2_SENSOR wire between the ECM/PCM (A44) and ECT sensor 2, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2185 indicated?
YES
Check for poor connections or loose terminals at ECT sensor 2 and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P219A — page 18510

- Source file: pages/18510.html

DTC P219A: Air-fuel Ratio Variation of Cylinders (KA KC KL models R18Z1)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
If any of the DTCs listed below are indicated at the same time as DTC P219A, troubleshoot those DTCs first, then recheck for P219A.
P0133, P1157, P1172, P2195, P2238, P2252, P2A00: A/F sensor (Sensor 1)
P0134, P0135: A/F sensor (Sensor 1) heater
P0201, P0202, P0203, P0204: No. 1 - No. 4 cylinder injector(s)
P0300: Random misfire
P0301, P0302, P0303, P0304: No. 1, No. 2, No. 3, or No. 4 cylinder misfire detected
P0351, P0352, P0353, P0354: Ignition coil power circuit
P0400, P0401, P0404, P0406, P2413: EGR system
If the engine speed fluctuates at highway speeds, the EGR line may be blocked by carbon or sludge.

1. Problem verification (CYLINDER AF TEST with the HDS):

- 1. Turn the ignition switch to ON (II).
- 1. Record following HDS on-board snapshot parameters:
VEHICLE SPEED
ENGINE SPEED
MAP SENSOR
ECT SENSOR 1
APP SENSOR
AF FB (ST FUEL TRIM)
EGR COMMAND
EGR LIFT
- 1. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check under these conditions:
A/C off
All electrical loads off
- 1. Do the CYLINDER AF TEST three times in the INSPECTION MENU with the HDS.
Is the result OK?
YES
Go to step 2.
NO
Go to step 3.

2. Problem verification (OBD STATUS check):

- 1. Test-drive the vehicle in the range of these recorded on-board snapshot parameters:
VEHICLE SPEED
ENGINE SPEED
MAP SENSOR
ECT SENSOR 1
APP SENSOR
AF FB (ST FUEL TRIM)
EGR COMMAND
EGR LIFT
- 1. Monitor the OBD STATUS for DTC P219A in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 7 .
NO
If the HDS indicates PASSED, intermittent failure, the system is OK at this time. If the HDS indicates EXECUTING, keep driving until a result comes on. If the HDS indicates OUT OF CONDITION, go to step 2-1and recheck.

3. Engine compression check:

- 1. Do a cylinder compression test .
Is the compression of one cylinder lower than the other cylinders?
YES
Correct the engine compression problem, then do the repair verification.
NO
Go to step 4

4. Valve clearance check:

- 1. Inspect the valve clearance .
Is the valve clearance OK?
YES
Go to step 5.
NO
Adjust the valve clearance, then do the repair verification.

5. Problem verification (CYLINDER AF TEST with the HDS):

- 1. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check under these conditions:
A/C off
All electrical loads off
- 1. Do the CYLINDER AF TEST three times in the INSPECTION MENU with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time.
NO
Go to step 6.

6. Injector check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Exchange the injector from the problem cylinder with one from the another cylinder.
- 1. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check under these conditions:
A/C off
All electrical loads off
- 1. Do the CYLINDER AF TEST three times in the INSPECTION MENU with the HDS.
Is an injector failure indicated at the cylinder where the injector was exchanged?
YES
Replace the faulty injector - Refer to: Injector Removal and Installation (K24Z7) Injector Removal and Installation (R18A9) Injector Removal and Installation (R18Z1 (A/T, M/T)) Injector Removal and Installation (R18Z1 (CVT)) ('14 '15) , then do the repair verification.
NO
Go to step 9 .

7. Valve clearance check:

- 1. Inspect the valve clearance. - Refer to: Valve Clearance Adjustment (K24Z7) , Valve Clearance Adjustment (Except K24Z7)
Is the valve clearance OK?
YES
Go to step 8.
NO
Adjust the valve clearance - Refer to: Valve Clearance Adjustment (K24Z7) , Valve Clearance Adjustment (Except K24Z7) , then do the repair verification.

8. EGR flow line visual check:

- 1. Check for carbon or sludge in the EGR line.
Is there carbon or sludge?
YES
Clean the intake manifold EGR port and the EGR pipe with throttle plate cleaner. Also, clean the passage inside the EGR valve with throttle plate cleaner, then do the repair verification.
NO
Go to step 9.

9. Intake and exhaust manifold visual check (oil leak):

- 1. Check for carbon build-up in the intake manifold and the exhaust manifold.
Is there carbon build-up in the intake manifold and/or the cylinder head?
YES
Check the engine for oil leaks. If needed, repair the engine. Remove the carbon, then do the repair verification.
NO
Intermittent failure, the system is OK at this time. Check the engine for oil leaks.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Test-drive the vehicle in the range of these recorded on-board snapshot parameters:
VEHICLE SPEED
ENGINE SPEED
MAP SENSOR
ECT SENSOR 1
APP SENSOR
AF FB (ST FUEL TRIM)
EGR COMMAND
EGR LIFT
- 1. Monitor the OBD STATUS for DTC P219A in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, EXECUTING, or OUT OF CONDITION?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck.
EXECUTING
Keep the test condition until a result comes on.
OUT OF CONDITION
Meet the test condition, and check the OBD STATUS again.

## P2238 — page 18514

- Source file: pages/18514.html

DTC P2238: A/F Sensor (Sensor 1) AFS+ Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2238 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), the PGM-FI subrelay, and the ECM/PCM.

2. Shorted wire check (AFS+ line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors
| A/F sensor (Sensor 1) 4P connector
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | AFS+
| Test point 1 | ECM/PCM connector C (49P) No. 39 (RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the AFS+ wire between the ECM/PCM (C39) and the A/F sensor (Sensor 1), then do the repair verification.
NO
The AFS+ wire is OK. Go to step 3.

3. ECM/PCM internal circuit check:

- 1. Reconnect the ECM/PCM connector C (49P).
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | A/F sensor (Sensor 1) 4P connector: disconnected
| Test circuit | AFS+
| Test point 1 | A/F sensor (Sensor 1) 4P connector No. 1 (RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is the voltage about 2.2 V?
YES
The ECM/PCM internal circuit is OK. Replace the A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Monitor the OBD STATUS for DTC P2238 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep idling until a result comes on.

## P2252 — page 18517

- Source file: pages/18517.html

DTC P2252: A/F Sensor (Sensor 1) AFS- Circuit Low Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2252 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the A/F sensor (Sensor 1), the PGM-FI subrelay, and the ECM/PCM.

2. Shorted wire check (AFS- line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| A/F sensor (Sensor 1) 4P connector
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | A/F sensor (Sensor 1) 4P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | AFS-
| Test point 1 | ECM/PCM connector C (49P) No. 40 (RED/YEL)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the AFS- wire between the ECM/PCM (C40) and the A/F sensor (Sensor 1), then do the repair verification.
NO
The AFS- wire is OK. Go to step 3.

3. A/F sensor (Sensor 1) check (substitution):

- 1. Substitute a known-good A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) .
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2252 indicated?
YES
The A/F sensor (Sensor 1) is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the original A/F sensor (Sensor 1) - Refer to: A/F Sensor Removal and Installation (K24Z7) , A/F Sensor Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Monitor the OBD STATUS for DTC P2252 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1 and recheck. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then check the OBD STATUS again.
NOT COMPLETED
Keep idling until a result comes on.

## P2413 — page 18521

- Source file: pages/18521.html

DTC P2413: EGR System Malfunction ('12-'13: R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Do the EGR TEST in the INSPECTION MENU with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EGR valve and the ECM/PCM.
NO
Go to step 2.

2. Determine possible failure area (EGR valve position sensor, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Check the EGR VLS in the DATA LIST with the HDS.
Is about 0 V indicated?
YES
Go to step 3.
NO
Go to step 7.

3. Determine possible failure area (VCC2 line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| EGR 5P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | EGR 5P connector: disconnected
| Test circuit | VCC2
| Test point 1 | EGR valve 5P connector No. 2 (YEL/BLU)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 5.
NO
Go to step 4.

4. Open wire check (VCC2 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VCC2
| Test point 1 | ECM/PCM connector B (49P) No. 29 (YEL/BLU)
| Test point 2 | EGR valve 5P connector No. 2 (YEL/BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VCC2 wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VCC2 wire between the ECM/PCM (B29) and the EGR valve, then do the repair verification.

5. Shorted wire check (EGR_L line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | EGR_L
| Test point 1 | ECM/PCM connector B (49P) No. 41 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the EGR_L wire between the ECM/PCM (B41) and the EGR valve, then do the repair verification.
NO
The EGR_L wire is not shorted. Go to step 6.

6. Open wire check (EGR_L line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | EGR_L
| Test point 1 | ECM/PCM connector B (49P) No. 41 (WHT/BLK)
| Test point 2 | EGR valve 5P connector No. 4 (WHT/BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The EGR_L wire is OK. Go to step 7.
NO
Repair an open in the EGR_L wire between the ECM/PCM (B41) and the EGR valve, then do the repair verification.

7. EGR valve position sensor internal circuit check 1:

- 1. Turn the ignition switch to LOCK (0).
- 1. If not already done, disconnect the following connector.
| EGR 5P connector
- 1. At the EGR valve side, measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected (on step 5)
| Test circuit | EGR_L, SG2
| Test point 1 | EGR valve 5P connector No. 3 (EGR valve side)
| Test point 2 | EGR valve 5P connector No. 4 (EGR valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there 100 k? or more?
YES
Replace the EGR valve , then do the repair verification.
NO
Go to step8.

8. EGR valve position sensor internal circuit check 2:

- 1. At the EGR valve side, measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected (on step 5)
| Test circuit | EGR_L, VCC2
| Test point 1 | EGR valve 5P connector No. 2 (EGR valve side)
| Test point 2 | EGR valve 5P connector No. 4 (EGR valve side)
| Courtesy of HONDA, U.S.A., INC.
Is there 100 kΩ or more?
YES
Replace the EGR valve , then do the repair verification.
NO
Go to step9.

9. Shorted wire check (E_EGR line):

- 1. If ECM/PCM connector B (49P) is not disconnected, jump the SCS line with the HDS.
- 1. If not already done, disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | E_EGR
| Test point 1 | ECM/PCM connector B (49P) No. 2 (BLU/RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the E_EGR wire between the ECM/PCM (B2) and the EGR valve, then do the repair verification.
NO
The E_EGR wire is not shorted. Go to step 10.
10.Open wire check (E_EGR line):
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | E_EGR
| Test point 1 | ECM/PCM connector B (49P) No. 2 (BLU/RED)
| Test point 2 | EGR valve 5P connector No. 5 (BLU/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The E_EGR wire is OK. Go to step 11.
NO
Repair an open in the E_EGR wire between the ECM/PCM (B2) and the EGR valve, then do the repair verification.
11.Open wire check (PG line):
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | EGR 5P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | PG
| Test point 1 | EGR valve 5P connector No. 1 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The PG wire is OK. Go to step 12.
NO
Repair an open in the PG wire between the EGR valve and G101, then do the repair verification.
12.EGR valve operation check:
- 1. Reconnect ECM/PCM connector B (49P).
- 1. At the EGR valve side, connect terminals A and B with a jumper wire.
| Terminal A | EGR valve 6P connector No. 5 (EGR valve side)
| Terminal B | Battery positive terminal
| Courtesy of HONDA, U.S.A., INC.
- 1. Start the engine, and let it idle.
- 1. At the EGR valve side, connect terminals A and B with a jumper wire.
| Terminal A | EGR valve 6P connector No. 1 (EGR valve side)
| Terminal B | Battery negative terminal
| Courtesy of HONDA, U.S.A., INC.
Does the engine stall or run roughly?
YES
The EGR valve is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Go to step 13.
 1. EGR port and passage visual check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the EGR valve .
- 1. Clean the intake manifold EGR port with throttle plate cleaner. Also, clean the passage inside the EGR valve with throttle plate cleaner.
- 1. Install the EGR valve .
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EGR TEST with the HDS.
Is the result OK?
YES
Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Replace the EGR valve , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the EGR TEST with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2413 indicated?
YES
Check for poor connections or loose terminals at the EGR valve and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2422 — page 18524

- Source file: pages/18524.html

DTC P1454: FTP Sensor Range/Performance Problem; DTC P2422: EVAP Canister Vent Shut Valve Stuck Closed Malfunction (KA KC KL models R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. EVAP system check:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the fuel fill cap, and wait 1 minute.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Go to step 2.
NO
Go to step 5.

2. Problem verification:

- 1. Install the fuel fill cap.
- 1. Clear the DTC with the HDS.
- 1. Start the engine, and let it idle without load (A/T in P or N, M/T in neutral) until the radiator fan comes on.
- 1. Monitor the EVAP PC DUTY (purge control duty) in the DATA LIST with the HDS, while holding the engine speed at 3, 000 rpm with the EVAP PC DUTY at 20 % or more for at least 4 minutes, then let the engine idle.
- 1. Monitor the OBD STATUS for DTC P1454 and/or P2422 in the DTCs MENU with the HDS.
Does the HDS indicate FAILED?
YES
The failure is duplicated. Go to step 3.
NO
If the HDS indicates PASSED, there might be an intermittent failure, and the system is OK at this time. Check the on-board snapshot, and repeat the driving conditions when the DTC set (fuel level, engine speed, vehicle speed, and outside temperature). If no problem is found, check for poor connections or loose terminals between the FTP sensor and the ECM/PCM. Also check for a blockage in the FTP sensor vent port and vent hose, and the EVAP canister vent hose and drain joint. If the HDS indicates NOT COMPLETED, let the engine idle until a result comes on.

3. EVAP canister vent line check:

- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the EVAP canister vent hose (A) from the EVAP canister (B).
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Check for a blockage in the EVAP canister vent hose or drain tube joint, then do the repair verification.
NO
Go to step 4.

4. EVAP canister vent shut valve operation check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the EVAP canister vent shut valve from the EVAP canister - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) .
- 1. Connect the 2P connector to the EVAP canister vent shut valve.
- 1. Turn the ignition switch to ON (II).
- 1. Select EVAP CVS ON in the INSPECTION MENU with the HDS.
- 1. Check the EVAP canister vent shut valve (A) operation.
| Courtesy of HONDA, U.S.A., INC.
Is the valve free of corrosion, and does it operate properly?
YES
The EVAP canister vent shut valve is OK. Go to step 5.
NO
Replace the EVAP canister vent shut valve - Refer to: EVAP Canister Vent Shut Valve Removal and Installation (K24Z7) , EVAP Canister Vent Shut Valve Removal and Installation (R18Z1) , then do the repair verification.

5. FTP sensor vent hose restriction check:

- 1. Disconnect the vent hose (A) from the FTP sensor (B).
| Courtesy of HONDA, U.S.A., INC.
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
Check for a blockage in the FTP sensor vent hose, then do the repair verification..
NO
The FTP sensor vent hose is OK. Go to step 6.

6. FTP sensor port restriction check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the FTP sensor (A) from the EVAP canister with its connector connected.
| Courtesy of HONDA, U.S.A., INC.
- 1. Turn the ignition switch to ON (II).
- 1. Check the FTP SENSOR in the DATA LIST with the HDS.
Is it between - 0.67 and 0.67 kPa (- 0.2 and 0.2 inHg, - 5 and 5 mmHg), or 2.4 and 2.6 V?
YES
The FTP sensor port is OK. The EVAP canister has an internal blockage; replace the EVAP canister - Refer to: EVAP Canister Removal and Installation (K24Z7) , EVAP Canister Removal and Installation (R18Z1) , then do the repair verification.
NO
Replace the FTP sensor - Refer to: FTP Sensor Removal and Installation (K24Z7) , FTP Sensor Removal and Installation (R18Z1) , then do the repair verification.
Repair verification (OBD STATUS check):
- 1. Turn the ignition switch to LOCK (0).
- 1. Install the parts in the reverse order of removal.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Monitor the OBD STATUS for DTC P1454 and/or P2422 in the DTCs MENU with the HDS.
Does the HDS indicate PASSED, FAILED, or NOT COMPLETED?
PASSED
Troubleshooting is complete. If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
FAILED
Go to step 1and recheck.
NOT COMPLETED
Hold the engine speed at 3, 000 rpm with the EVAP PC DUTY at 20% or more until a result comes on.

## P2646 — page 18526

- Source file: pages/18526.html

DTC P2646: Rocker Arm Oil Pressure Switch Circuit Low Voltage ('12-'13: KA KC KL models except K24Z7)
Special Tools Required
Pressure Gauge Adapter 07NAJ-P07010A
A/T Low Pressure Gauge w/Panel 07406-0070301
A/T Pressure Test Hose 07AAJ-PY4A100
A/T Pressure Adapter 07MAJ-PY40120
Oil Pressure Hose 07ZAJ-S5AA200
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
If DTC P2648 and/or P2649 are stored at the same time as DTC P2646, troubleshoot those DTCs first, then recheck for DTC P2646.

1. Oil level check:

- 1. Check the engine oil level.
Is the level OK?
YES
Go to step 2.
NO
Adjust the engine oil to the proper level - Refer to: Engine Oil Level Check(K24Z7) , Engine Oil Level Check(Except K24Z7) , then do the repair verification.

2. Problem verification:

- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM.
NO
Go to step 3.

3. Determine of possible failure area (VTM line, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Rocker arm oil pressure switch 2P connector
- 1. Turn the ignition switch to ON (II).
- 1. Check the VTEC PRES SW in the DATA LIST with the HDS.
Does is show ON in the DATA LIST?
YES
Go to step 5.
NO
Go to step 4.

4. VTEC system oil pressure check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Remove the rocker arm oil pressure switch (A) from the rocker arm oil control valve, and attach the special tools as shown in illustration, then attach the rocker arm oil pressure switch to the oil pressure gauge adapter (B).
Fig 1: Rocker Arm Oil Pressure Switch With Torque Specifications Courtesy of HONDA, U.S.A., INC.
- 1. Reconnect the rocker arm oil pressure switch 2P connector.
- 1. Start the engine.
- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
- 1. Check the oil pressure.
Does the oil pressure increase to at least 392 kPa (4.0 kgf/cm 2 , 56.9 psi)?
YES
The VTEC system oil pressure is OK. Replace the rocker arm oil pressure switch , then do the repair verification.
NO
Inspect the VTEC system. If it is OK, replace the rocker arm oil control valve - Refer to: Rocker Arm Oil Control Valve Removal and Installation (K24Z7) , Rocker Arm Oil Control Valve Removal and Installation (Except K24Z7) , then do the repair verification.

5. Shorted wire check (VTM line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VTM
| Test point 1 | ECM/PCM connector C (49P) No. 13 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VTM wire between the ECM/PCM (C13) and the rocker arm oil pressure switch, then do the repair verification.
NO
The VTM wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9)
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2646 indicated?
YES
Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2647 — page 18528

- Source file: pages/18528.html

DTC P2647: Rocker Arm Oil Pressure Switch Circuit High Voltage ('12-'13: KA KC KL models except K24Z7)
NOTE:
Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
If DTC P2648 and/or P2649 are stored at the same time as DTC P2647, troubleshoot those DTCs first, then recheck for DTC P2647.

1. Oil level check:

- 1. Check the engine oil level.
Is the level OK?
YES
Go to step 2.
NO
Adjust the engine oil to the proper level - Refer to: Engine Oil Level Check(K24Z7) , Engine Oil Level Check(Except K24Z7) , then do the repair verification.

2. Problem verification:

- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
NOTE: If DTC P2647 is indicated, go to step 6 . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM.
NO
Go to step 3.

3. Determination of possible failure area (rocker arm oil control valve, others):

- 1. Check the result of VTEC TEST.
Is VTEC Switch Failure, VTEC Switch Open, VTEC Switch SIG Line Open, or VTEC Switch GND Line Open indicated?
YES
Go to step 4.
NO
Check for poor connections or loose terminals at the rocker arm oil pressure switch. If it is OK, replace the rocker arm oil control valve - Refer to: Rocker Arm Oil Control Valve Removal and Installation (K24Z7) , Rocker Arm Oil Control Valve Removal and Installation (Except K24Z7) , then do the repair verification.

4. Rocker arm oil pressure switch internal Ω check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Rocker arm oil pressure switch 2P connector
- 1. At the rocker arm oil pressure switch side, check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil pressure switch 2P connector: disconnected
| Test circuit | LG, VTM
| Test point 1 | Rocker arm oil pressure switch 2P connector No. 1 (rocker arm oil pressure switch side)
| Test point 2 | Rocker arm oil pressure switch 2P connector No. 2 (rocker arm oil pressure switch side)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 5.
NO
Replace the rocker arm oil pressure switch , then do the repair verification.

5. Open wire check (LG line):

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Rocker arm oil pressure switch 2P connector: disconnected
| Test circuit | VTM
| Test point 1 | Rocker arm oil pressure switch 2P connector No. 2 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
The VTM wire is OK. Repair an open in the LG wire between the rocker arm oil pressure switch and G101, then do the repair verification.
NO
Go to step 6.

6. Open wire check (VTM line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector C (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil pressure switch 2P connector: disconnected
| | ECM/PCM connector C (49P): disconnected
| Test circuit | VTM
| Test point 1 | ECM/PCM connector C (49P) No. 13 (WHT/BLK)
| Test point 2 | Rocker arm oil pressure switch 2P connector No. 2 (WHT/BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VTM wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VTM wire between the rocker arm oil pressure switch and the ECM/PCM (C13), then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine, and let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2647 indicated?
YES
Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2648 — page 18530

- Source file: pages/18530.html

DTC P2648: Rocker Arm Oil Control Solenoid Circuit Low Voltage ('12-'13: KA KC KL models except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
Is the result OK?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM.
NO
Go to step 2.

2. Rocker arm oil control solenoid internal Ω check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Rocker arm oil control solenoid 2P connector
- 1. At the rocker arm oil control solenoid side, check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil control solenoid 2P connector: disconnected
| Test circuit | VT_SOL, PG
| Test point 1 | Rocker arm oil control solenoid 2P connector No. 1 (rocker arm oil control solenoid side)
| Test point 2 | Rocker arm oil control solenoid 2P connector No. 2 (rocker arm oil control solenoid side)
| Courtesy of HONDA, U.S.A., INC.
Is there 14-30 - at room temperature-
YES
Go to step 3.
NO
Replace the rocker arm oil control valve - Refer to: Rocker Arm Oil Control Valve Removal and Installation (K24Z7) , Rocker Arm Oil Control Valve Removal and Installation (Except K24Z7) , then do the repair verification.

3. Shorted wire check (VT_SOL line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Rocker arm oil control solenoid 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VT_SOL
| Test point 1 | ECM/PCM connector B (49P) No. 33 (BLK/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VT_SOL wire between the rocker arm oil control solenoid and the ECM/PCM (B33), then do the repair verification.
NO
The VT_SOL wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Do the VTEC TEST in the INSPECTION MENU with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2648 indicated?
YES
Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update ECM Removal and Installation (K24Z7) ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P2649 — page 18532

- Source file: pages/18532.html

DTC P2649: Rocker Arm Oil Control Solenoid Circuit High Voltage ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2649 indicated?
YES
Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM.

2. Rocker arm oil control solenoid internal Ω check:

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Rocker arm oil control solenoid 2P connector
- 1. At the rocker arm oil control solenoid side, check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil control solenoid 2P connector: disconnected
| Test circuit | VT_SOL, PG
| Test point 1 | Rocker arm oil control solenoid 2P connector No. 1 (rocker arm oil control solenoid side)
| Test point 2 | Rocker arm oil control solenoid 2P connector No. 2 (rocker arm oil control solenoid side)
| Courtesy of HONDA, U.S.A., INC.
Is there 14-30 - at room temperature-
YES
Go to step 3.
NO
Replace the rocker arm oil control valve - Refer to: Rocker Arm Oil Control Valve Removal and Installation (K24Z7) Rocker Arm Oil Control Valve Removal and Installation (Except K24Z7) , then do the repair verification.

3. Open wire check (PG line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil control solenoid 2P connector: disconnected
| Test circuit | PG
| Test point 1 | Rocker arm oil control solenoid 2P connector No. 1 (BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Go to step 4.
NO
Repair an open in the PG wire between the rocker arm oil control solenoid and the G101, then do the repair verification.

4. Shorted wire check (VT_SOL line):

- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Rocker arm oil control solenoid 2P connector: disconnected
| | ECM/PCM connector B (49P): disconnected
| Test circuit | VT_SOL
| Test point 1 | ECM/PCM connector B (49P) No. 33 (BLK/WHT)
| Test point 2 | Rocker arm oil control solenoid 2P connector No. 2 (BLK/WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The VT_SOL wire is OK. Update the ECM/PCM - Refer to: ECM/PCM Update if it does not have the latest software, or substitute a known-good ECM/PCM - Refer to: ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair an open in the VT_SOL wire between the rocker arm oil control solenoid and the ECM/PCM (B33), then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) How to Troubleshoot the Fuel and Emissions Systems (R18Z1) How to Troubleshoot the A/T System (A/T) How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Start the engine. Hold the engine speed at 3, 000 rpm without load (A/T in P or N, M/T in neutral) until the radiator fan comes on, then let it idle
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P2649 indicated?
YES
Check for poor connections or loose terminals at the rocker arm oil pressure switch, the rocker arm oil control solenoid, and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to:, ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## U0122 — page 18542

- Source file: pages/18542.html

DTC U0122: F-CAN Malfunction (ECM/PCM-VSA Modulator-Control Unit) ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. DTC check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC U0029 and U0122 indicated at the same time?
YES
Go to the troubleshooting for DTC U0029 - Refer to: DTC Troubleshooting: U0029 (ECM/PCM (K24Z7)) , DTC Troubleshooting: U0029 (ECM/PCM (Except K24Z7)) .
NO
Go to step 2.

2. Problem verification:

- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0122 indicated?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the VSA modulator-control unit, and the ECM/PCM.

3. VSA system check:

- 1. Check for communication to the VSA system with the HDS.
Does the HDS communicate with the VSA modulator-control unit?
YES
Go to step 4.
NO
Go to the symptom troubleshooting for ABS indicator, brake system indicator, and VSA indicator do not go off - Refer to: VSA System Symptom Troubleshooting ABS indicator, brake system indicator, and VSA indicator do not go off , VSA System Symptom Troubleshooting VSA cannot be turned off ('12) , VSA System Symptom Troubleshooting VSA cannot be turned off ('13-'15) (2013 2014 2015) .

4. Open wire check (F_CAN-H line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector A (49P)
| VSA modulator-control unit 22P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | VSA modulator-control unit 22P connector: disconnected
| Test circuit | F_CAN-H
| Test point 1 | ECM/PCM connector A (49P) No. 3 (WHT)
| Test point 2 | VSA modulator-control unit 22P connector No. 10 (WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-H wire is OK. Go to step 5.
NO
Repair an open in the F_CAN-H wire between the ECM/PCM (A3) and the VSA modulator-control unit, then do the repair verification.

5. Open wire check (F_CAN-L line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | VSA modulator-control unit 22P connector: disconnected
| Test circuit | F_CAN-L
| Test point 1 | ECM/PCM connector A (49P) No. 4 (RED)
| Test point 2 | VSA modulator-control unit 22P connector No. 15 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-L wire is OK. Update the VSA modulator-control unit - Refer to: VSA Modulator-Control Unit Update VSA Sensor Neutral Position Memorization VSA Modulator-Control Unit Removal and Installation (Natural Gas models) VSA Modulator-Control Unit Removal and Installation (Except Natural Gas models) if it does not have the latest software, then do the repair verification. If DTC U0122 is indicated after updated, replace the original VSA modulator-control unit , then do the repair verification.
NO
Repair an open in the F_CAN-L wire between the ECM/PCM (A4) and the VSA modulator-control unit, then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0122 indicated?
YES
Check for poor connections or loose terminals at the VSA modulator-control unit and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## U0131 — page 18544

- Source file: pages/18544.html

DTC U0131: F-CAN Malfunction (ECM/PCM-EPS Control Unit) ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. DTC check:

- 1. Turn the ignition switch to ON (II).
- 1. Check for Pending or Confirmed DTCs with the HDS.
Are DTC U0028 and U0131 indicated at the same time?
YES
Go to the troubleshooting for DTC U0029 - Refer to: DTC Troubleshooting: U0029 (ECM/PCM (K24Z7)) , DTC Troubleshooting: U0029 (ECM/PCM (Except K24Z7)) .
NO
Go to step 2.

2. Problem verification:

- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0131 indicated?
YES
The failure is duplicated. Go to step 3.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the EPS control unit and the ECM/PCM.

3. EPS system check:

- 1. Check that the HDS communicates with the EPS system.
Does the HDS communicate with the EPS control unit?
YES
Go to step 4.
NO
Go to the symptom troubleshooting for an EPS indicator does not go off, and no DTCs are stored - Refer to: EPS Symptom Troubleshooting EPS indicator does not come on , EPS Symptom Troubleshooting Steering pull , EPS Symptom Troubleshooting EPS indicator does not go off, and no DTCs are stored (A/T, M/T) .

4. Open wire check (F_CAN-H line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector A (49P)
| EPS control unit connect A (11P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | EPS control unit connect A (11P): disconnected
| Test circuit | F_CAN-H
| Test point 1 | ECM/PCM connector A (49P) No. 3 (WHT)
| Test point 2 | EPS control unit connector A (11P) No. 10 (WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-H wire is OK. Go to step 5.
NO
Repair an open in the F_CAN-H wire between the ECM/PCM (A3) and the EPS control unit, then do the repair verification.

5. Open wire check (F_CAN-L line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | EPS control unit connect A (11P): disconnected
| Test circuit | F_CAN-L
| Test point 1 | ECM/PCM connector A (49P) No. 4 (RED)
| Test point 2 | EPS control unit connector A (11P) No. 11 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-L wire is OK. Update the EPS control unit - Refer to: EPS Control Unit Update if it does not have the latest software, or substitute a known-good EPS control unit - Refer to: EPS Control Unit Removal and Installation , then do the repair verification. If DTC U0131 is not indicated after substitution, replace the original EPS control unit , then do the repair verification.
NO
Repair an open in the F_CAN-L wire between the ECM/PCM (A4) and the EPS control unit, then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0131 indicated?
YES
Check for poor connections or loose terminals at the EPS control unit and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## U0155 — page 18546

- Source file: pages/18546.html

DTC U0155: F-CAN Malfunction (ECM/PCM-Gauge Control Module) ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0155 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the gauge control module (tach) and the ECM/PCM.

2. Body electrical system DTC check:

- 1. Check for body electrical DTCs with the HDS.
Is DTC B1168, B1169, and/or B1178 indicated?
YES
Go to step 3.
NO
Do the gauge control module (tach) input test - Refer to: Gauge Control Module (Speedo) Input Test , Gauge Control Module (Tach) Input Test .

3. Open wire check (F_CAN-H line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connector A (49P)
| Gauge control module (tach) 32P connector
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | Gauge control module (tach) 32P connector: disconnected
| Test circuit | F_CAN-H
| Test point 1 | ECM/PCM connector A (49P) No. 3 (WHT)
| Test point 2 | Gauge control module (tach) 32P connector No. 19 (WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-H wire is OK. Go to step 4.
NO
Repair an open in the F_CAN-H wire between the ECM/PCM (A3) and the gauge control module (tach), then do the repair verification.

4. Open wire check (F_CAN-L line):

- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector A (49P): disconnected
| | Gauge control module (tach) 32P connector: disconnected
| Test circuit | F_CAN-L
| Test point 1 | ECM/PCM connector A (49P) No. 4 (RED)
| Test point 2 | Gauge control module (tach) 32P connector No. 20 (RED)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The F_CAN-L wire is not open. Substitute a known-good gauge control module (tach) - Refer to: Gauge Control Module (Speedo) Removal and Installation ('12) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module('12) , Gauge Control Module (Tach) Removal and Installation ('12) , Gauge Control Module (Speedo) Removal and Installation (Gasoline) (2013) , Gauge Control Module (Tach) Removal and Installation (Gasoline) (2013) , Rewriting the ODO Data and Transferring the Maintenance Minder™ Data to a New Gauge Control Module (Gasoline) (2013) , then do the repair verification. If DTC U0155 is not indicated after substitution, replace the original gauge control module (tach) , then do the repair verification.
NO
Repair an open in the F_CAN-L wire between the ECM/PCM (A4) and the gauge control module (tach), then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC U0155 indicated?
YES
Check for poor connections or loose terminals at the gauge control module (tach) and the ECM/PCM, then go to step 1.
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0651 — page 18550

- Source file: pages/18550.html

DTC P0651: Sensor Reference Voltage B Malfunction ('12-'13: R18Z1)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0651 indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the ECM/PCM and these parts:
EGR valve
Input shaft (mainshaft) speed sensor
APP sensor
A/C pressure sensor
FTP sensor

2. Determine possible failure area (VCC lines, others):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the connectors from these parts:
EGR valve
Input shaft (mainshaft) speed sensor
APP sensor
A/C pressure sensor
FTP sensor
- 1. Turn the ignition switch to ON (II).
- 1. Clear the DTC with the HDS.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0651 indicated?
YES
Go to step 4.
NO
Go to step 3.

3. Shorted part check:

- 1. Check for Pending or Confirmed DTCs with the HDS while reconnecting these parts, one at a time:
EGR valve
Input shaft (mainshaft) speed sensor
APP sensor
A/C pressure sensor
FTP sensor
Is DTC P0651 indicated?
YES
Replace the part that caused the DTC when it was reconnected, then do the repair verification.
NO
Check for poor connections or loose terminals at the ECM/PCM and the following parts, then go to step 1.
EGR valve
Input shaft (mainshaft) speed sensor
APP sensor
A/C pressure sensor
FTP sensor

4. Shorted wire check (VCC lines):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connectors.
| ECM/PCM connectors A (49P)
| ECM/PCM connectors B (49P)
- 1. Check for continuity between these test points and body ground individually.
| Test condition | IG LOCK (0)
| | EGR valve 5P connector: disconnected
| | Input shaft (mainshaft) speed sensor 3P connector: disconnected
| | APP sensor 6P connector: disconnected
| | A/C pressure sensor 3P coonector: disconnected
| | FTP sensor 3P connector: disconnected
| | ECM/PCM connector A (49P): disconnected
| | ECM/PCM connector B (49P): disconnected
| Connector | Test circuit | Terminal
| ECM/PCM connector A (49P) | VCC6 | No. 30 (RED)
| VCC5 | No. 35 (GRY)
| ECM/PCM connector B (49P) | VCC2 | No. 29 (YEL/BLU)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the VCC wire between the ECM/PCM (A30, A35, or B29) and each part that was disconnected in step 2, then do the repair verification.
NO
Update the ECM/PCM, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) then do the repair verification.
Repair verification:
- 1. Turn the ignition switch to LOCK (0).
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0651 indicated?
YES
Check for poor connections or loose terminals at the ECM/PCM and the following parts, then go to step 1.
EGR valve
Input shaft (mainshaft) speed sensor
APP sensor
A/C pressure sensor
FTP sensor
NO
Troubleshooting is complete. If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P065E — page 18554

- Source file: pages/18554.html

DTC P065E: IMT Actuator Circuit Malfunction ('12-'13: except K24Z7)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .

1. Problem verification:

- 1. Turn the ignition switch ON (II).
- 1. Clear the DTC with the HDS.
- 1. Turn the ignition switch OFF (0).
- 1. Turn the ignition switch ON (II).
- 1. Select the IMT (IMRC) TEST in the INSPECTION MENU with the HDS, and do the IMT (IMRC) ACTIVATION for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P065E indicated?
YES
The failure is duplicated. Go to step 2.
NO
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the IMT actuator and the ECM/PCM.

2. Shorted wire check (SVSP line to power):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM/PCM connector B (49P)
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SVSP
| Test point 1 | ECM/PCM connector B (49P) No. 12 (BRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 0.1 V or less?
YES
The SVSP wire is not shorted to power. Go to step 3.
NO
Repair a short to power in the SVSP wire between the ECM/PCM (B12) and the IMT actuator, then do the repair verification.

3. Shorted wire check (SVSM line to power):

- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SVSM
| Test point 1 | ECM/PCM connector B (49P) No. 13 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 0.1 V or less?
YES
The SVSM wire is not shorted to power. Go to step 4.
NO
Repair a short to power in the SVSM wire between the ECM/PCM (B13) and the IMT actuator, then do the repair verification.

4. Shorted wire check (SVSP line to ground):

- 1. Turn the ignition switch to LOCK (0).
- 1. Measure the Ω between test points 1 and 2.
| Test condition | IG OFF (0)
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SVSP
| Test point 1 | ECM/PCM connector B (49P) No. 12 (BRN)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 1.0 MΩ or more?
YES
The SVSP wire is not shorted to ground. Go to step 5.
NO
Repair a short to ground in the SVSP wire between the ECM/PCM (B12) and the IMT actuator, then do the repair verification.

5. Shorted wire check (SVSM line to ground):

- 1. Turn the ignition switch to LOCK (0).
- 1. Measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SVSM
| Test point 1 | ECM/PCM connector B (49P) No. 13 (WHT/BLK)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 1.0 MΩ or more?
YES
The SVSM wire is not shorted to ground. Go to step 6.
NO
Repair a short to ground in the SVSM wire between the ECM/PCM (B13) and the IMT actuator, then do the repair verification.

6. Determination of possible failure area (ECM/PCM, IMT circuit shorted, IMT circuit open):

- 1. Measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector B (49P): disconnected
| Test circuit | SVSP, SVSM
| Test point 1 | ECM/PCM connector B (49P) No. 12 (BRN)
| Test point 2 | ECM/PCM connector B (49P) No. 13 (WHT/BLK)
| Courtesy of HONDA, U.S.A., INC.
Is there about 15-30 --
15 Ω or less
The IMT circuit is shorted. Go to step 7.
15-30 -
The SVSP/SVSM wire are OK. Update the ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
30 Ω or more
The IMT circuit is open. Go to step 8.

7. IMT circuit internal Ω check:

- 1. Disconnect the following connector.
| IMT actuator 2P connector
- 1. At the IMT actuator side, measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector B (49P): disconnected
| | IMT actuator 2P connector: disconnected
| Test circuit | SVSP, SVSM
| Test point 1 | IMT actuator 2P connector No. 1 (IMT actuator side)
| Test point 2 | IMT actuator 2P connector No. 2 (IMT actuator side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 15-30 --
YES
The IMA actuator is OK. Repair a short in the SVSP wire to the SVSM wire between the ECM/PCM (B12, B13) and the IMT actuator, then do the repair verification.
NO
Replace the IMT actuator , then do the repair verification.

8. IMT circuit internal Ω check:

- 1. Disconnect the following connector.
| IMT actuator 2P connector
- 1. At the IMT actuator side, measure the Ω between test points 1 and 2.
| Test condition | IG LOCK (0)
| | ECM/PCM connector B (49P): disconnected
| | IMT actuator 2P connector: disconnected
| Test circuit | SVSP, SVSM
| Test point 1 | IMT actuator 2P connector No. 1 (IMT actuator side)
| Test point 2 | IMT actuator 2P connector No. 2 (IMT actuator side)
| Courtesy of HONDA, U.S.A., INC.
Is there about 15-30 --
YES
Repair an open in the SVSP/SVSM wires between the ECM/PCM (B12, B13) and the IMT actuator, then do the repair verification.
NO
Replace the IMT actuator, then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM/PCM with the HDS - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) .
- 1. Do the ECM/PCM idle learn procedure - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7) .
- 1. Turn the ignition switch to LOCK (0).
- 1. Turn the ignition switch to ON (II).
- 1. Select the IMT (IMRC) TEST in the INSPECTION MENU with the HDS, and do the IMT (IMRC) ACTIVATION for 10 seconds.
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P065E indicated?
YES
Check for poor connections or loose terminals at the IMT actuator and the ECM/PCM. If the ECM/PCM was updated, substitute a known-good ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM/PCM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM/PCM was substituted, replace the original ECM/PCM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs are indicated, go to the indicated DTC's troubleshooting.

## P0720 — page 18577

- Source file: pages/18577.html

DTC P0720: Output Shaft (Countershaft) Speed Sensor Circuit Malfunction (R18Z1 M/T)
NOTE: Before you troubleshoot, record all freeze data and any on-board snapshot, and review the general troubleshooting information - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15) .

1. Problem verification:

- 1. Start the engine. Hold the engine speed at 3, 000 RPM without load (in neutral) until the radiator fan comes on, then let it idle.
- 1. Test-drive several minutes.
- 1. Check the C SHAFT SPD in the DATA LIST with the HDS.
Is any vehicle speed indicated?
YES
Intermittent failure, the system is OK at this time. Check for poor connections or loose terminals at the output shaft (countershaft) speed sensor and the ECM.
NO
Go to step 2.

2. Open wire check (VCC1 line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Disconnect the following connector.
| Output shaft (countershaft) speed sensor 3P connector
- 1. Turn the ignition switch to ON (II).
- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| Test circuit | VCC1
| Test point 1 | Output shaft (countershaft) speed sensor 3P connector No. 3 (YEL/RED)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
The VCC1 wire is OK. Go to step 3.
NO
Repair an open in the VCC1 wire between the output shaft (countershaft) speed sensor 3P connector and the ECM (C23), then do the repair verification.

3. Determine possible failure area (NC line, others):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| Test circuit | NC
| Test point 1 | Output shaft (countershaft) speed sensor 3P connector No. 2 (BLK/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
Go to step 4.
NO
Go to step 5.

4. Open wire check (SG1 line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG ON (II)
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| Test circuit | VCC1, SG1
| Test point 1 | Output shaft (countershaft) speed sensor 3P connector No. 1 (GRN/WHT)
| Test point 2 | Output shaft (countershaft) speed sensor 3P connector No. 3 (YEL/RED)
| Courtesy of HONDA, U.S.A., INC.
Is there about 5.0 V?
YES
The SG1 wire is OK. Replace the output shaft (countershaft) speed sensor - Refer to: Output Shaft (Countershaft) Speed Sensor Removal and Installation (K24Z7) , Output Shaft (Countershaft) Speed Sensor Removal and Installation (R18Z1 M/T) , then do the repair verification.
NO
Repair open in the SG1 wire between the ECM (C24) and the output shaft (countershaft) speed sensor, then do the repair verification.

5. Shorted wire check (NC line):

- 1. Turn the ignition switch to LOCK (0).
- 1. Jump the SCS line with the HDS.
- 1. Disconnect the following connector.
| ECM connector B (49P)
- 1. Check for continuity between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| | ECM connector B (49P): disconnected
| Test circuit | NC
| Test point 1 | ECM connector B (49P) No. 48 (BLK/WHT)
| Test point 2 | Body ground
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
Repair a short in the NC wire between the rocker arm oil pressure switch and the ECM (B48), then do the repair verification.
NO
The NC wire is not shorted. Go to step 6.

6. Open wire check (NC line):

- 1. Measure the voltage between test points 1 and 2.
| Test condition | IG LOCK (0)
| | Output shaft (countershaft) speed sensor 3P connector: disconnected
| | ECM connector B (49P): disconnected
| Test circuit | NC
| Test point 1 | Output shaft (countershaft) speed sensor 3P connector No. 2 (BLK/WHT)
| Test point 2 | ECM connector B (49P) No. 48 (BLK/WHT)
| Courtesy of HONDA, U.S.A., INC.
Is there continuity?
YES
The NC wire is OK. Update the ECM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) if it does not have the latest software, or substitute a known-good ECM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then do the repair verification.
NO
Repair open in the NC wire between the ECM (B48) and the output shaft (countershaft) speed sensor, then do the repair verification.
Repair verification:
- 1. Reconnect all connectors.
- 1. Turn the ignition switch to ON (II).
- 1. Reset the ECM with the HDS. - Refer to: How to Troubleshoot the Fuel and Emissions Systems (K24Z7) , How to Troubleshoot the Fuel and Emissions Systems (R18Z1) , How to Troubleshoot the A/T System (A/T) , How to Troubleshoot the Fuel and Emissions Systems (R18A9) , How to Troubleshoot the CVT System (CVT) ('14 '15)
- 1. Do the ECM idle learn procedure. - Refer to: ECM Idle Learn Procedure (K24Z7) , ECM/PCM Idle Learn Procedure (Except K24Z7)
- 1. Test-drive under these conditions:
Engine coolant temperature (ECT SENSOR 1) above 176 °F (80 °C)
Transmission in 5th
Engine speed between 2, 000-3, 000 RPM
Drive for several minutes, then decelerate (with the throttle fully closed) for 8 seconds
- 1. Check for Pending or Confirmed DTCs with the HDS.
Is DTC P0720 indicated?
YES
Check for poor connections or loose terminals at the output shaft (countershaft) speed sensor and the ECM. If the ECM was updated, substitute a known-good ECM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) , then recheck. If the ECM was substituted, go to step 1.
NO
Troubleshooting is complete. If the ECM was substituted, replace the original ECM - Refer to: ECM/PCM Update , ECM Removal and Installation (K24Z7) , ECM/PCM Removal and Installation (Except K24Z7 and R18Z1 (CVT)) . If any other Pending or Confirmed DTCs were indicated, go to the indicated DTC's troubleshooting.
