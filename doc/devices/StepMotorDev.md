# <p align="center">StepMotorDev</p>
<p align="center"><img src="../images/hlek.svg"></p>


StepMotorDev provides stepper motor drivers support. Currently A4998 and DRV8825 are supported. These drivers are cheap and widely present on the market. A4998 and DRV8825 are very similar, however they have differences like fault in DRV8825 pin that allows to detect driver failure. Also this feature support logic for two endstops (hardware and software), logic for motor driver failure line, micro steps. Each stepper motor has it's own buffer for the stepper motor commands. If several motors are customized for the same virtual device, all these commands will be executed synchronously, thus stepper motor movements will be synchronized.

Supported stepper motor drivers provide a significant amount of signal lines, most of them are optional. In order to make this feature to be as flexible as possible there is just one mandatory signal that requires gpio - `STEP`. All other lines are optional.

Be very careful with stepper motors. These devices consume significant current and may cause burns or fire if incorrectly connected or misused.

```
"StepMotorDevCustomizer": {
    "motor_group_1" : {
        "dev_id" : 9,
        "motors" : {
            "m0" : {
                "buffer_size" : 512,
                "steps_per_revolution" : 200,
                "drive_type" : "a4998",
                "default_speed" : 10000,
                "error_action" : "stop",
                "step"  : {"gpio" :  "PA_0"},
                "dir"   : {"default" : "CW"},
                "reset" : {"gpio" :  "PA_1"},
                "m1"   : {"default" : 0},
                "m2"   : {"default" : 0},
                "m3"   : {"default" : 0},
                "enable": {"default" : "disable"},
                "sleep" : {"default" : "wakeup"},
                "cw_endstop" : {"position_limit" : "20000", "active_level" :  "low", "action": "ignore"},
                "ccw_endstop" : {"position_limit" :  "-20000", "active_level" :  "high", "action": "ignore"}
            }
        },
    "requires" :  { "timer" : "TIM3"}}}
```

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"motor_group_1"` | Virtual device name | String | Yes |
| `"dev_id"` | Device id | Number, [1, 15] | Yes |
| `"motors"` | Object that contains motors to be described | Object | Yes |
| `"m0"` | Object that contains "m0" motor description | Object | Yes |
| `"buffer_size"` |  Motor circular buffer size in bytes | Number | Yes |
| `"steps_per_revolution"` | Motor steps per revolution value. Refer to stepper motor documentation. | Number | Yes |
| `"default_speed"` | Default motor speed. It is a number of microseconds between step pulses. It doesn't take into account micro stepping. | Number | Yes |
| `"drive_type"` | Type of the driver being used. | "drv8825", "a4998", "unknown" | Yes |
| `"error_action"` | Action to be made on error. "stop" will stop just this motor. "stop_all" will stop all motors in the virtual device. | "stop", "stop_all" | Yes |
| `"step"` | Describes gpio pin to be used for `STEP` signal. | Object with `"gpio"` | Yes |
| `"dir"` | Describes stepper motor default `DIR` and signal. [Details here...](#Stepper-motor-direction)| Object | Yes |
| `"reset"` | Describes gpio pin to be used for`reset` signal.| Object with `"gpio"` | No |
| `"m1"`, `"m2"`, `"m3"` | Objects that describe micro step pins and default values. [Details here...](#Micro-stepping)| Object | Yes |
| `"enable"` | Describes enable signal and default enable state. [Details here...](#Enable)| Object | Yes |
| `"sleep"` | Describes sleep signal and default sleep state. [Details here...](#Sleep)| Object | Yes |
| `"fault"` | Object that control fault behavior. Available for DRV8825 and "unknown" drivers. [Details here...](#Fault)| Object | No |
| `"cw_endstop"` | Describes clock-wise end stop. [Details here...](#End-stops)| Object | Yes |
| `"ccw_endstop"` | Describes counter-clock-wise end stop. [Details here...](#End-stops)| Object | Yes |
| `"requires"` | Describes peripherals required by the virtual device. Just one timer is required per virtual device. | Object with `"timer"` | Yes |

#### Stepper motor direction
Stepper motor direction may be clock-wise or counter-clock-wise. Optionally it may specify `DIR` pin for supported stepper motor drivers.


Table below describe key/values to customize stepper motor direction:
| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"default"` | Default stepper motor direction | "CW" for clock-wise, "CCW" for counter-clock-wise | Yes |
| `"gpio"` | Optional GPIO pin dedicated for `DIR` signal. | `"gpio"` | No |

Actual direction values are very related, user should define what is "clock-wise" and "counter-clock_wise" for stepper being used.


#### Micro stepping
A4998 and DRV8825 use micro stepping lines in order to do micro steps ( a fraction of single step motor step ). User may either change these lines from software, or pull up or down these pins if micro steps are not required. In any case stepper motor logic implemented in firmware and software must be aware of micro steps being used. It directly impacts on stepper motor position being calculated during movement. Table below describes values to be used by `"m1"`, `"m2"`, `"m3"`.

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"default"` | Value that describe default micro step line value. | 0 or 1| Yes|
| `"gpio"` | Optional GPIO pin dedicated for corresponding stepper motor line. | `"gpio"`| No |

It is possible to use GPIO pins for some of these lines, while others have no GPIO pins assigned.

#### Enable

`ENABLE` line allows to enable/disable stepper motor driver. Table below describes values available for `"enable"` key.

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"default"` | Default enable value | "enable" or "disable"| Yes |
| `"gpio"` | Optional GPIO pin used to control this line | `"gpio"`| No |

Note, that stepper motor left enabled will consume significant current and may overheat. It is not recommended to set stepper motor `"default"` value to "enable".

#### Sleep

`SLEEP` signal may put driver into sleep mode. Table below describes values for `"sleep"` key:

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"default"` | Default sleep pin value | "sleep" or "wakeup"| Yes |
| `"gpio"` | Optional GPIO pin dedicated for the `SLEEP` line. | `"gpio"`| No |

#### End stops

End stops provide possibility to stop stepper motor when it reached some limit. Such limit may be defined by hardware by using end stop switch for example, or by software limitation. In the case of software limitation current motor position calculated by firmware is being used. Table below explains values available for both clock-wise and counter-clock-wise end stop description:

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"action"` | Action that happens when end-stop is triggered. It is possible to "ignore" it, "stop" the motor or "stop_all" motors. | "ignore", "stop", "stop_all" | Yes |
| `"position_limit"` | Mandatory if gpio pin is not dedicated for hardware end stop. Specify stepper motor position where stepper motor software limit will be triggered. | Number | Yes, if `"gpio"` is not used |
| `"gpio"` | Hardware end stop is used. Specifies GPIO pin to be used for this purpose. | `"gpio"` | No |
| `"active_level"` | Active logical level that makes hardware end stop active. | "low" or "high" | Yes, if `"gpio"` is used |

Firmware calculates position for each stepper motor used. Position is being calculated by the following formula: <img src="https://render.githubusercontent.com/render/math?math=P=32\times N">, where <img src="https://render.githubusercontent.com/render/math?math=P"> is current position, <img src="https://render.githubusercontent.com/render/math?math=N"> number of full steps performed, 32 is a minimal fraction of step supported by micro stepping. If motor direction is clock-wise <img src="https://render.githubusercontent.com/render/math?math=P"> increases, if direction is counter-clock-wise decrease. For example, if stepper motor executes two subsequent commands to move by one counter-clock-wise and current micro stepping option will instruct to do half-step, than position will decrease by one.

If `"driver_type"` is "unknown" micro stepping is not supported and motor position will change by full step every `STEP` pulse.

#### Fault

DRV8825 is different to A4998. It has "Fault" line that may be used to detect fault conditions inside driver. Table below explains possible values to use it:

| Key      | Value description | Possible values | Required |
|:---------|:------------------|:----------------|:---------|
| `"gpio"` | Specifies GPIO pin to be used connected to `FAULT` driver line. | `"gpio"` | Yes |
| `"active_level"` | Active logical level that indicates fault. | "low" or "high" | Yes |
| `"action"` | Action to be taken if `FAULT` becomes active.  It is possible to "ignore" it, "stop" the motor or "stop_all" motors. | "ignore", "stop", "stop_all" | Yes |

Note, `"fault"` object is optional, however, if specified all values should be also specified. Also, DRV8825 active `"fault"` signal is `low` (grounded).