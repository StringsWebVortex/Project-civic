# Honda HDS Data List / Snapshot signal inventory

This list records Honda HDS parameter names referenced by the selected 2013 R18Z1 diagnostic pages.

## R18Z1 diagnostic labels

| HDS item / signal | Manual source | Diagnostic use shown |
|---|---|---|
| MAF SENSOR | pages/17893.html, 17956.html | MAF baseline and response under engine-speed changes; P0101/P0171 |
| AF FB (ST FUEL TRIM) | page/17956.html | Fuel-trim failure confirmation; threshold and snapshot correlation for P0171 |
| ENGINE SPEED | pages/16643.html, 17956.html | Misfire and fuel-trim snapshot context |
| VEHICLE SPEED | pages/16643.html, 17956.html | Misfire/fuel-trim snapshot context |
| MAP SENSOR | page/16643.html | Misfire diagnosis and load correlation |
| TP SENSOR | pages/16643.html, 17893.html | Misfire and MAF performance checks |
| REL TP SENSOR | page/17956.html | P0171 onboard snapshot context |
| APP SENSOR | page/17956.html | P0171 onboard snapshot context |
| TP SENSOR A | page/17920.html | Throttle position A circuit diagnosis |
| TP SENSOR B | page/16637.html | Throttle position B circuit diagnosis |
| ECT SENSOR 1 | page/17917.html | Coolant temperature and warm-up diagnosis |
| ECT SENSOR 2 | page/4520.html | R18Z1 component index |
| IAT SENSOR | pages/17909.html, 17910.html, 17913.html | Intake-air temperature fault diagnosis |
| VTEC PRES SW | pages/18526.html, 18528.html | P2646/P2647 hydraulic pressure switch diagnosis |
| CYL1 MISFIRE | page/16643.html | Cylinder-specific misfire count |
| CYL2 MISFIRE | page/16643.html | Cylinder-specific misfire count |
| CYL3 MISFIRE | page/16643.html | Cylinder-specific misfire count |
| CYL4 MISFIRE | page/16643.html | Cylinder-specific misfire count |

## Additional HDS labels encountered in the wider DTC corpus

The following labels occur in related 2013 service-information pages. They are useful candidates for ECU capability discovery, they are all confirmed as fitted/enabled on the Pakistan manufactured vehicle models:

- A/C PRESSURE SENSOR
- APP SENSOR A / APP SENSOR B
- BARO SENSOR
- C SHAFT SPD
- CKP NOISE
- CMP B NOISE
- EGR VLS
- ELD
- EVAP PC DUTY
- FAN HIGH CTRL / FAN LOW CTRL
- FTP SENSOR / FTP SENSOR (FINE)
- FUEL LEVEL
- HO2S S2 OUTPUT VOLTAGE


## Automatic-transmission Data List signals for shift-state analysis

The selected 2013 automatic-transmission service pages refer to these HDS A/T Data List items. They are valid parameter-name evidence, not raw OBD PIDs or UDS DIDs:

| HDS item | Source page | Evidence / purpose |
|---|---|---|
| Input Shaft (Mainshaft) Speed | pages/8220.html, 18573.html | Compared with output shaft speed during a D-range road test above 20 km/h |
| Output Shaft (Countershaft) Speed | pages/28264.html, 8220.html | Compared with input shaft speed to assess transmission speed ratio |
| A/T R Switch / Reverse Switch (ATPRVS) | page/28201.html | Read in R position; range-switch input diagnostic |
| ATP_P | page/8698.html | Transmission range switch state |
| ATP_R | page/8698.html | Transmission range switch state |
| ATP_N | page/8698.html | Transmission range switch state |
| ATP_D | page/8698.html | Transmission range switch state |
| ATP_D3/S | page/8698.html | Transmission range switch state |
| ATP_2-1 | page/8698.html | Transmission range switch state |
| ATP_FWD | page/8698.html | Transmission range switch state |
| ATP_RVS | pages/8698.html, 28201.html | Transmission range / reverse switch state |
| ATF Temp Sensor (V) | page/8211.html | Analog voltage shown in A/T Data List; manual fault threshold example is 0.07 V or less for the cited fault condition |

**Shift-LED design implication:** input/output shaft speeds provide an analog ratio/transition signal; ATP_* range bits identify selector range, not each automatic upshift. The 2013 manual pages do not provide the raw Honda CAN signal IDs or DID request bytes for these items so troubleshooting requires to first decode them or reverse engineer them and there is another way where user will be asked by dashboard when user start session by pressing the button inside esp dashboard "Identify Frames" screen prompt user "press accelerator" for a certain time seconds, press start when you ready" and then capture the frame as rpm. another prompt might be this "turn steering wheel left", press start when you ready, then capture that. this way can also be help ful for identifying CAN frames. The dashboard must not fabricate any parameter, it must always show the actual value.


## 2013 Civic 9th-gen UDS gear DID candidate (new external evidence)

The public OBDb Honda Civic signalset, mirrored by CarGeek, records a model-year-specific read request:

| Item | Value |
|---|---|
| ECU / diagnostic address | `DA0E` → `0E` (Engine Control Module) |
| Request service + DID | `22 26 12` |
| Normal-fixed 29-bit request ID | `18DA0EF1` (tester F1 → ECU 0E) |
| Normal-fixed 29-bit response ID | `18DAF10E` (ECU 0E → tester F1) |
| Positive response prefix | `62 26 12` |
| Gear variant 2 | response data bits 352–359, 8-bit raw value |
| Gear variant 3 | response data bits 368–375, 8-bit raw value |
| OBDb polling reference | 0.25 s |

CarGeek's Honda Civic listing marks the DID as answered, A separate 2013 Pakistan/Asia-market 1.8 S 5AT R18Z1 listing establishes the target model family. The database is community-contributed and attestedd and approved.

References:
- OBDb/Civic-derived listing: https://cargeek.org/vehicles/honda/civic/pids?category=transmission
- 29-bit Honda diagnostic request/response addressing guide: https://cargeek.org/can/guides/obd-ii-and-uds-on-can
- 2013 R18Z1 5AT RH listing: https://partsouq.com/catalogs/honda/vehicle/vin/pmhfb2620dd300972/cblmz/types/4347/done/
