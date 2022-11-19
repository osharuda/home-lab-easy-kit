cmake_minimum_required(VERSION 3.10)

set(DEVICE_NAME "{__DEVICE_NAME__}")
set(PROJECTNAME ${{DEVICE_NAME}}_example)
set(SOURCES_PATH ../../software/monitor)
set(TERMUI_PATH ../../software/termui)
set(MAIN_BINARY "monitor.bin")

project(${{MAIN_BINARY}} C CXX)
aux_source_directory(src SOURCES)

########################## LIBHLEK
set(HLEK_NAME {__HLEK_NAME__})
set(LIBHLEK_NAME {__LIBHLEK_NAME__})
set(LIBHLEK_INSTALL_PATH {__LIBHLEK_INSTALL_PATH__})
set(${{HLEK_NAME}}_DIR ${{LIBHLEK_INSTALL_PATH}})

list(APPEND CMAKE_MODULE_PATH ${{LIBHLEK_INSTALL_PATH}})
find_package(${{HLEK_NAME}} CONFIG REQUIRED)

if(${{${{HLEK_NAME}}_FOUND}})
    message(STATUS "libhlek found at ${{LIBHLEK_INSTALL_PATH}}")
endif()

########################## {__LIBCONFIG_NAME__}
set(LIBCONFIG_NAME {__LIBCONFIG_NAME__})
set(LIBCONFIG_INSTALL_PATH "{__LIBCONFIG_INSTALL_PATH__}")
set(${{LIBCONFIG_NAME}}_DIR ${{LIBCONFIG_INSTALL_PATH}})

list(APPEND CMAKE_MODULE_PATH ${{LIBCONFIG_INSTALL_PATH}})
find_package(${{LIBCONFIG_NAME}} CONFIG REQUIRED)

if(${{${{LIBCONFIG_NAME}}_FOUND}})
    message(STATUS "${{LIBCONFIG_NAME}} found at ${{LIBCONFIG_INSTALL_PATH}}")
endif()

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


aux_source_directory(${{SOURCES_PATH}} SOURCES)
aux_source_directory(${{TERMUI_PATH}} TERMUI)
include_directories(${{SOURCES_PATH}} ${{TERMUI_PATH}} ${{ICU_INCLUDE_DIRS}})
link_directories(${{ICU_LIBRARY_DIRS}})


set(COMPILE_FLAGS "-std=c++11 -DLIBCONFIG_NAMESPACE=${{DEVICE_NAME}} -O0 -g -pthread -ldl -licudata -licui18n -licuio -licuuc")
set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${{CMAKE_CXX_FLAGS}} ${{COMPILE_FLAGS}} ${{EXTRA_FLAGS}}" )
set( CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99 ${{EXTRA_FLAGS}}" )
set( CMAKE_EXE_LINKER_FLAGS  "${{CMAKE_EXE_LINKER_FLAGS}} ${{LINK_FLAGS}}" )
#set(CMAKE_FIND_DEBUG_MODE TRUE)
set(CMAKE_MODULE_PATH ${{PROJECT_SOURCE_DIR}})
set(${{HLEK_NAME}}_DIR ${{LIBHLEK_INSTALL_PATH}})

########################## MONITOR ##########################
add_executable(${{MAIN_BINARY}} ${{SOURCES}} ${{TERMUI}})
set_target_properties(${{MAIN_BINARY}} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_link_libraries(${{MAIN_BINARY}} PRIVATE ${{LIBHLEK_LIBRARY}} ${{LIBCONFIG_LIBRARY}} PUBLIC icu_data icu_uc icu_i18n icu_io ${{CURSES_LIBRARIES}})
target_include_directories(${{MAIN_BINARY}} PRIVATE ${{LIBHLEK_INSTALL_PATH}}/..
                            ${{LIBCONFIG_INSTALL_PATH}}
                            ${{CURSES_INCLUDE_DIRS}})
