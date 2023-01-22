set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Path to the GNU ARM Toolchain. CMAKE_C_COMPILER variable and others should
# point to the correct files.
set(tools "/usr")

# Should point to the CMSIS library. For the latest version of the library
# this path should end with "/STM32F10x_StdPeriph_Lib_V3.6.0"
set(STDPERIF_PATH "/home/pi/STM32F10x_StdPeriph_Lib_V3.6.0")

set(CMAKE_C_COMPILER ${tools}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  ${tools}/bin/arm-none-eabi-gcc)
set(CMAKE_OBJCOPY ${tools}/bin/arm-none-eabi-objcopy)
set(CMAKE_PRINT_SIZE ${tools}/bin/arm-none-eabi-size)


set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
