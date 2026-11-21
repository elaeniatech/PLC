# Arduino Opta PLC — Compressor Alternation

Arduino Opta WiFi project for alternating three compressors, monitoring motor states and cycle counters in Arduino Cloud, and simulating air consumption and tank pressure.

The control sequence runs in Arduino PLC IDE using Function Block Diagram (FBD). The embedded Arduino sketch exchanges shared variables with the PLC, connects to Arduino Cloud, and runs the pressure simulation.

## Project files

| Path | Purpose |
| --- | --- |
| [`motor_alternation/motor_alternation.plcprj`](motor_alternation/motor_alternation.plcprj) | PLC IDE project, including FBD programs, shared variables, and the embedded sketch. |
| [`motor_alternation/LLSketch/LLSketch.ino`](motor_alternation/LLSketch/LLSketch.ino) | Generated Arduino sketch with PLC runtime integration. |
| [`motor_alternation/LLSketch/sketch.yaml`](motor_alternation/LLSketch/sketch.yaml) | Arduino CLI build profiles and library versions. |
| `motor_alternation/Build/`, `Download/`, `LLSketch_build/` | Generated PLC and Arduino build artifacts. |
| `motor_alternation/PreviousVersions/`, `ChangeTargetBackup/`, `backups/` | Project snapshots and backups. |
| [`motor_alternation/assets/scada/`](motor_alternation/assets/scada/) | Compressor dashboard artwork. |

**Make persistent sketch changes in PLC IDE's embedded sketch editor.** PLC IDE regenerates `LLSketch.ino`; editing that generated file alone can result in changes being overwritten. The `main` and `initialize` programs contain the PLC logic.

## Hardware and build configuration

- Arduino Opta WiFi.
- Arduino PLC IDE with the Opta target installed.
- Arduino Cloud account, a Thing associated with the Opta, and a dashboard.
- A compatible 2.4 GHz Wi-Fi network.

The checked-in `opta_1.3.0` profile specifies:

| Component | Version |
| --- | --- |
| Arduino Mbed OS Opta Boards (`arduino:mbed_opta`) | 4.5.0 |
| `AlPlc_Opta` | 1.3.2 |
| `ArduinoIoTCloud` | 2.10.0 |
| `Arduino_ConnectionHandler` | 1.3.0 |

The complete dependency list is in `LLSketch/sketch.yaml` and the project's Sketch → Libraries configuration. The profile name `opta_1.3.0` and the `AlPlc_Opta` library version are separate settings.

## PLC and sketch interface

Shared inputs carry sketch commands into the PLC. Shared outputs carry PLC monitoring values back to the sketch.

| Shared variable | Direction | Sketch / Cloud variable |
| --- | --- | --- |
| `PLCIn.start_sh` | Sketch → PLC | `Start` |
| `PLCIn.stop_sh` | Sketch → PLC | `Stop` |
| `PLCIn.C1_res_sh`, `C2_res_sh`, `C3_res_sh` | Sketch → PLC | `C1_res`, `C2_res`, `C3_res` |
| `PLCOut.B1_sh`, `B2_sh`, `B3_sh` | PLC → sketch | `Motor_1`, `Motor_2`, `Motor_3` |
| `PLCOut.C1_sh`, `C2_sh`, `C3_sh` | PLC → sketch | `Counter_M1`, `Counter_M2`, `Counter_M3` |

The motor signals represent PLC output commands; the simulator treats an active signal as a compressor supplying air. They are not independent feedback from physical motors.

## Pressure simulation

`updatePressureSimulation()` runs in the Opta sketch. Arduino Cloud supplies the consumption setting and displays the results.

| Setting | Current value |
| --- | --- |
| Initial pressure | 7 bar gauge |
| Tank volume | 500 L |
| Capacity per running compressor | 30 CFM |
| Adjustable consumption | 0–30 CFM |
| Update interval | Approximately 1 second |
| Low pressure threshold | 6 bar |
| High pressure threshold | 8 bar |

The sketch integrates the difference between compressor supply and consumption:

```text
supply_CFM = number_of_running_compressors × 30

pressure_change_bar = (0.478 / tank_volume_liters)
                   × (supply_CFM - consumption_CFM)
                   × elapsed_seconds
```

The conversion assumes supply and consumption use the same free-air reference conditions. This is a simplified tank model; it does not model temperature changes, compressor performance curves, or pipe losses. Pressure is clamped at zero.

- At or below **6 bar**, the sketch sends one `Start` pulse.
- At or above **8 bar**, after a start request, it sends one `Stop` pulse.
- Each pulse remains true until the next simulation update, approximately one second.
- The sketch waits for the opposite threshold before issuing another command.

At startup, pressure is 7 bar and consumption is zero, so pressure remains steady until consumption is increased or a compressor is running. If one compressor supplies 30 CFM and consumption is also 30 CFM, pressure remains approximately constant. Use a lower consumption to observe pressure rising while that compressor runs.

## Arduino Cloud variables

Create these variables in the Opta's **Thing** with the exact names below. `ArduinoCloud.addProperty(...)` in the local sketch registers the device-side properties; it does not create variables in the Cloud interface.

| Name | Type | Permission | Update policy |
| --- | --- | --- | --- |
| `Consumption_CFM` | Float | Read & Write | On change |
| `Pressure_bar` | Float | Read Only | On change |
| `Start`, `Stop` | Boolean | Read Only | On change |
| `Motor_1`, `Motor_2`, `Motor_3` | Boolean | Read Only | On change |
| `Counter_M1`, `Counter_M2`, `Counter_M3` | Integer | Read Only | On change |
| `C1_res`, `C2_res`, `C3_res` | Boolean | Read & Write | On change |

Suggested dashboard widgets:

- A slider from 0 to 30 for `Consumption_CFM`.
- A pressure gauge and time chart for `Pressure_bar`.
- Three status indicators for the motors and three value displays for the counters.
- Momentary controls for the counter resets. The sketch forwards each reset variable's level directly to the PLC; return it to false after a reset.

`Start` and `Stop` are indicators of the automatic simulation commands in this version, rather than manual dashboard controls.

## Wi-Fi setup

The setup used during this project's connection troubleshooting included fixed network credentials, the Cloud connection handler, and an explicit Wi-Fi startup call:

```cpp
#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[] = "YOUR_NETWORK";
const char PASS[] = "YOUR_PASSWORD";

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
```

Startup included:

```cpp
setDebugMessageLevel(-1);
initProperties();
ArduinoCloud.begin(ArduinoIoTPreferredConnection);
WiFi.begin(SSID, PASS);
```

Keep `ArduinoCloud.update()` in `loop()`. This setup does not use `NetworkConfigurator`, `BLEAgent`, or `SerialAgent` in the application sketch, although related libraries remain in the build profile as dependencies.

**Check the generated sketch as well as the embedded project sketch:** the checked-in generated sketch contains `WiFi.begin(...)`, while the embedded `SketchContent` does not contain that explicit call. PLC IDE adds Wi-Fi startup code during generation. Preserve the working PLC IDE Wi-Fi configuration when changing the simulation.

Cloud debug output is disabled to keep it off the PLC USB communication channel. A hotspot previously connected successfully when the router network did not. Router connectivity and current hardware operation still need to be checked on the actual Opta.

Use local network credentials when configuring the device. Review the project, generated sketch, and backups for embedded credentials before sharing copies.

## Build and run

1. Open `motor_alternation/motor_alternation.plcprj` in Arduino PLC IDE.
2. Review the Opta target, I/O mappings, shared variables, and Wi-Fi configuration.
3. Configure the Cloud Thing and dashboard variables listed above.
4. Build the project in PLC IDE. Its post-build step generates and compiles the Arduino sketch.
5. Connect to the Opta using the correct runtime COM port and download the project.
6. Confirm the Cloud device is online, then increase `Consumption_CFM` to exercise the simulation.

To compile an already-generated sketch from PowerShell, run this from the repository root using the PLC IDE Tools installation:

```powershell
& 'C:\Program Files\Arduino PLC IDE Tools\arduino-cli.exe' `
  --config-file "$env:LOCALAPPDATA\T\arduino-cli.yaml" `
  compile '.\motor_alternation\LLSketch' `
  --profile opta_1.3.0 `
  --build-path '.\motor_alternation\LLSketch_build'
```

This command compiles the generated Arduino sketch only. It does not regenerate the PLC project or upload firmware. A successful compile confirms the build, not Wi-Fi, Cloud connectivity, or physical machine operation.
