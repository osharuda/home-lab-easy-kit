cmake_minimum_required(VERSION 3.10)

set(INSTALL_PATH "{__INSTALL_PATH__}/{__INSTALL_PREFIX__}")
set(DEVICE_NAME "{__DEVICE_NAME__}")
set(SW_LIB_PACKAGE "lib{__DEVICE_NAME__}")
set(SW_LIB ${{DEVICE_NAME}})

project(${{DEVICE_NAME}}sw C CXX)

aux_source_directory(src SOURCES)
aux_source_directory(termui TERMUI)
include_directories(inc termui ${{ICU_INCLUDE_DIRS}})
link_directories(${{ICU_LIBRARY_DIRS}})

set(COMPILE_FLAGS "-std=c++11 -O0 -g -pthread -ldl -licudata -licui18n -licuio -licuuc")
set(EXTRA_FLAGS "-DBLUE_PILL_SOFTWARE")

set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${{CMAKE_CXX_FLAGS}} ${{COMPILE_FLAGS}} ${{EXTRA_FLAGS}}" )
set( CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99 ${{EXTRA_FLAGS}}" )
set( CMAKE_EXE_LINKER_FLAGS  "${{CMAKE_EXE_LINKER_FLAGS}} ${{LINK_FLAGS}}" )
#set(CMAKE_MODULE_PATH ${{PROJECT_SOURCE_DIR}})

########################## ICU Library
find_package(ICU COMPONENTS data uc i18n io REQUIRED)

add_library(icu_data SHARED IMPORTED)
set_property(TARGET icu_data PROPERTY IMPORTED_LOCATION ${{ICU_DATA_LIBRARIES}})

add_library(icu_uc SHARED IMPORTED)
set_property(TARGET icu_uc PROPERTY IMPORTED_LOCATION ${{ICU_UC_LIBRARIES}})

add_library(icu_i18n SHARED IMPORTED)
set_property(TARGET icu_i18n PROPERTY IMPORTED_LOCATION ${{ICU_I18N_LIBRARIES}})

add_library(icu_io SHARED IMPORTED)
set_property(TARGET icu_io PROPERTY IMPORTED_LOCATION ${{ICU_IO_LIBRARIES}})

set(ICU_TARGETS "icu_data icu_uc icu_i18n icu_io")

########################## NCURSES Library
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)

########################## LIBRARY ##########################

add_library(${{SW_LIB}} STATIC ${{SOURCES}} ${{TERMUI}})
file(GLOB INSTALL_PUB_HEADERS "inc/*.h" "inc/*.hpp" "termui/*.hpp")
set_target_properties(${{SW_LIB}} PROPERTIES
                   PUBLIC_HEADER "${{INSTALL_PUB_HEADERS}}"
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)

message( STATUS "Library installation path is: ${{INSTALL_PATH}}")

file(WRITE ${{DEVICE_NAME}}Config.cmake
"add_library({__DEVICE_NAME__} STATIC IMPORTED)
find_library({__DEVICE_NAME__}_LIBRARY_PATH lib{__DEVICE_NAME__}.a HINTS {__INSTALL_PATH__}/{__INSTALL_PREFIX__}/lib)
set({__DEVICE_NAME__}_INCLUDE_DIRS \"{__INSTALL_PATH__}/{__INSTALL_PREFIX__}/inc\")
set(HLEK_INCLUDE_DIRS \"{__INSTALL_PATH__}/{__INSTALL_PREFIX__}/inc\")
set_target_properties({__DEVICE_NAME__} PROPERTIES IMPORTED_LOCATION $\{{{__DEVICE_NAME__}_LIBRARY_PATH\}})
")

install(TARGETS ${{SW_LIB}}
        PUBLIC_HEADER
            DESTINATION ${{INSTALL_PATH}}/inc
        ARCHIVE
            DESTINATION ${{INSTALL_PATH}}/lib
)

install(FILES ${{DEVICE_NAME}}Config.cmake
            DESTINATION ${{INSTALL_PATH}}/cmake)

########################## TESTTOOL ##########################
set(SW_TEST test_tool.bin)
add_executable(${{SW_TEST}} testtool/testtool.cpp testtool/circbuffer.c testtool/utools.c)
add_dependencies(${{SW_TEST}} ${{SW_LIB}})
set_target_properties(${{SW_TEST}} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_compile_definitions(${{SW_TEST}} PUBLIC DISABLE_NOT_TESTABLE_CODE)
target_link_libraries(${{SW_TEST}} PRIVATE ${{SW_LIB}} PUBLIC icu_data icu_uc icu_i18n icu_io )

########################## MONITOR ##########################
set(SW_MONITOR ${{DEVICE_NAME}}_mon.bin)
add_executable(${{SW_MONITOR}} monitor/monitor.cpp monitor/handlers.cpp)
add_dependencies(${{SW_MONITOR}} ${{SW_LIB}})
set_target_properties(${{SW_MONITOR}} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_link_libraries(${{SW_MONITOR}} PRIVATE ${{SW_LIB}} PUBLIC icu_data icu_uc icu_i18n icu_io ${{CURSES_LIBRARIES}})
target_include_directories(${{SW_MONITOR}} PRIVATE ${{CURSES_INCLUDE_DIRS}})
