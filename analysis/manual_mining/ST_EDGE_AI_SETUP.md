# ST Edge AI setup on the Project Civic workstation

## Installed and verified

- STM32Cube AI Studio: Debian package 1.4.0, status install ok installed.
- ST Edge AI Core: 4.1.0-20732 (33a7f37f5).
- CLI: /opt/ST/STEdgeAI/4.1/Utilities/linux/stedgeai
- Environment:
  - STEDGEAI_CORE_DIR=/opt/ST/STEdgeAI/4.1
  - PATH=$STEDGEAI_CORE_DIR/Utilities/linux:$PATH
- stedgeai --version and stedgeai --tools both execute successfully.
- STM32F4 target configuration lists STM32F4xx / Cortex-M4 with single-precision FPU and GCC hard-float flags.
- GCC Cortex-M4 runtime archive is present at Middlewares/ST/AI/Lib/GCC/ARMCortexM4/NetworkRuntime1210_CM4_GCC.a (1,071,400 bytes).

## Important F407 constraints

- STM32F407 is a Cortex-M4F, not an NPU device. Use the standard Cortex-M4 runtime, not Neural-ART.
- ST Edge AI Core 4.1.0 release notes say target validation through the tool is not enabled for Cortex-M4. Compilation/runtime integration is available.
- The 1 MiB Flash / SRAM budget is tight.

## NanoEdge AI

NanoEdge AI Studio is ST's more directly targeted AutoML workflow for anomaly detection / outlier detection / classification / extrapolation and supports STM32. The ST software page currently routes download through a software request/approval process (may take up to 48 hours). It is not part of the installed ST Edge AI Core package and is not installed on this workstation yet.
