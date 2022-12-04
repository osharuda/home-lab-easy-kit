cmake_minimum_required(VERSION 3.6)

set(FW_NAME hlekfw)
project(${{FW_NAME}} C ASM)

set(EXTRA_FLAGS "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_COMMON_DIR ${{PROJECT_SOURCE_DIR}}/../../firmware)
string(TOLOWER ${{CMAKE_BUILD_TYPE}} BUILD_TYPE)

if (${{BUILD_TYPE}} STREQUAL "release")
    message(STATUS "* Release build is selected")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -mcpu=cortex-m3 -mthumb -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -DRELEASE -DIGNORE_FAILED_ASSERTS -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DHLEK_FIRMWARE")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -T mem.ld -T libs.ld -T sections.ld -Xlinker --gc-sections -L\"${{PROJECT_COMMON_DIR}}/ldscripts\"")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -Wl,-Map,\"${{FW_NAME}}.map\" --specs=nano.specs -o \"${{FW_NAME}}.elf\"")
    set(CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99" )
else()
    message(STATUS "* Debug build is selected")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -DHLEK_FIRMWARE")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -T mem.ld -T libs.ld -T sections.ld -Xlinker --gc-sections -L\"${{PROJECT_COMMON_DIR}}/ldscripts\"")
    set(CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} -Wl,-Map,\"${{FW_NAME}}.map\" --specs=nano.specs -o \"${{FW_NAME}}.elf\"")
    set(CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99" )
endif()

set(CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} ${{EXTRA_FLAGS}}" )

aux_source_directory(${{PROJECT_COMMON_DIR}}/src FW_SRC)

set(STDPERIF_SRC ${{STDPERIF_PATH}}/Libraries/STM32F10x_StdPeriph_Driver/src)
set(STM32F10x_SRC ${{STDPERIF_PATH}}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x)
set(CM3_CoreSupport_SRC ${{STDPERIF_PATH}}/Libraries/CMSIS/CM3/CoreSupport)

include_directories(    ${{STDPERIF_PATH}}/Libraries/CMSIS/CM3/CoreSupport
                        ${{STDPERIF_PATH}}/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/
                        ${{STDPERIF_PATH}}/Libraries/STM32F10x_StdPeriph_Driver/inc
                        ${{PROJECT_COMMON_DIR}}/inc
                        inc)

add_executable( ${{FW_NAME}}.elf
                ${{FW_SRC}}
                ${{STDPERIF_SRC}}/stm32f10x_gpio.c
                ${{STDPERIF_SRC}}/stm32f10x_i2c.c
                ${{STDPERIF_SRC}}/stm32f10x_rtc.c
                ${{STDPERIF_SRC}}/stm32f10x_usart.c
                ${{STDPERIF_SRC}}/stm32f10x_pwr.c
                ${{STDPERIF_SRC}}/stm32f10x_rcc.c
                ${{STDPERIF_SRC}}/stm32f10x_bkp.c
                ${{STDPERIF_SRC}}/stm32f10x_tim.c
                ${{STDPERIF_SRC}}/stm32f10x_adc.c
                ${{STDPERIF_SRC}}/stm32f10x_dma.c
                ${{STDPERIF_SRC}}/stm32f10x_exti.c
                ${{STDPERIF_SRC}}/stm32f10x_flash.c
                ${{STDPERIF_SRC}}/stm32f10x_can.c
                ${{STDPERIF_SRC}}/stm32f10x_spi.c
                ${{STDPERIF_SRC}}/misc.c
                ${{STM32F10x_SRC}}/system_stm32f10x.c
                ${{STM32F10x_SRC}}/startup/gcc_ride7/startup_stm32f10x_md.s)

add_custom_command(TARGET ${{FW_NAME}}.elf POST_BUILD
    COMMAND ${{tools}}/bin/arm-none-eabi-objcopy -O ihex ./${{FW_NAME}}.elf ./${{FW_NAME}}.hex
    COMMAND ${{CMAKE_PRINT_SIZE}} --format=berkeley ${{FW_NAME}}.elf
    DEPENDS ${{FW_NAME}}.elf)

# Uncomment to leave temporary files for troubleshooting.
#set_target_properties(${{FW_NAME}}.elf PROPERTIES COMPILE_FLAGS "-save-temps")
