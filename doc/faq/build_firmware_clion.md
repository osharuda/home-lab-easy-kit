# <p align="center">Loading firmware project in CLion</p>
<p align="center"><img src="../images/hlek.svg"></p>

Integrated Developer Environments (IDE) are very helpful. I use CLion on Linux. Configuring CLion to build Firmware may be somewhat little bit confusing.

# 1. Add a toolchain to CLion
   
   Open `Settings`->`Build, Execution, Deployment`->`Toolchains`
   
   Add new toolchain for ARM toolchain. Typically values are:
   
   | Name                  | Value |
   |:----------------------|:------------------|
   | `"Name"`              | ARM |
   | `"Build tool"`        | /usr/bin/cmake |
   | `"C Complier"`        | /usr/bin/arm-none-eabi-gcc |
   | `"C++ Complier"`      | /usr/bin/arm-none-eabi-g++ |
   | `"Debugger"`          | Bundled GDB |

# 2. Add one or more Profiles

   Open `Settings`->`Build, Execution, Deployment`->`CMake`

   | Name                  | Value |
   |:----------------------|:------------------|
   | `"Build type"`        | `Debug` or `Release` |
   | `"Toolchain"`         | ARM |
   | `"Generator"`         | Unix Makefiles |
   | `"CMake options"`     | Project options, see below ... |
   
CMake requires the following CMake options to be specified:
   | Option                             | Value |
   |:-----------------------------------|:------|
   | `"-DCMAKE_BUILD_TYPE"`             | `Debug` or `Release` |
   | `"-DHLEK_ROOT"`                    | Path to the home-lab-easy-kit directory |
   | `"-DHLEK_CONFIG"`                  | Name of the directory created for particular json configuration (configuration json file name without extension) |
   | `"-DCMAKE_DEPENDS_USE_COMPILER"`   | FALSE

Typical value of the `"CMake options"` value (single line):
```
-DCMAKE_BUILD_TYPE=Debug -DHLEK_ROOT="/mnt/SHARE/home-lab-easy-kit" -DHLEK_CONFIG="pacemakerdev" -DCMAKE_DEPENDS_USE_COMPILER=FALSE -DCMAKE_TOOLCHAIN_FILE:PATH="../firmware/toolchain.cmake"
```
# 3. Regenerate CMake
`File`->`Reload CMake Project`
# 4. Select required configuration
# 5. Build
`Build`->`Rebuild Project`