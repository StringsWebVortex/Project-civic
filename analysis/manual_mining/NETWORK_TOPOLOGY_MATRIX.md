# 2013 Civic network topology and interface requirements

Vehicle target: 2013 9th-gen Civic Pakistan RHD, R18Z1. Reference source is the selected 2013 non-hybrid service-information variant.

| Network | Manual evidence | Access / nominal rate | Project interface status | Confidence / gap |
|---|---|---|---|---|
| F-CAN | PGM-FI OBD page 4802; network topology page 775; diagnostic mode pages 1684/1690 | DLC pin 6 F-CAN_H, pin 14 F-CAN_L; 500 kbit/s | STM32 CAN1 is assigned PB8/PB9 AF9, nominal 500 kbit/s | Strong reference for PGM-FI access |
| B-CAN | Non-hybrid Computer Data Lines page 32907 Fig 1; Instrument Cluster page 32919 Figs 1–2; diagnostic mode page 1690 | 125 kbit/s; not listed as a direct DLC terminal | STM32 CAN2 assigned PB5/PB6 AF9, preferred 125 kbit/s | Network exists in selected non-hybrid diagram |
| K-Line | PGM-FI OBD page 4802 | DLC pin 7; used for communications other than PGM-FI | No K-Line transceiver/interface in current hardware |
| S-NET | Non-hybrid Computer Data Lines page 32907 Fig 1 | Shown separately in the network diagram |
| GA-NET audio | Page 32907 Fig 7 (except hybrid) | Separate GA-NET bus/audio circuit |
| SCS | PGM-FI OBD page 4802 | DLC pin 9 | No connection planned |

## Hardware implications

- Current STM32 pin-map comments document MB997 pull-up conflicts on PB6/PB9 with the plain 5 V TJA1051 interface.
