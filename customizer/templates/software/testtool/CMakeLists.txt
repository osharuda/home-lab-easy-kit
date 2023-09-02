cmake_minimum_required(VERSION 3.20)
set(PROJECTNAME testtool)
set(MAIN_BINARY "${{PROJECTNAME}}.bin")
project(${{PROJECTNAME}} C CXX)

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

########################## TESTTOOL
add_executable(${{MAIN_BINARY}} testtool.cpp circbuffer.c utools.c)
set_target_properties(${{MAIN_BINARY}} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_compile_definitions(${{MAIN_BINARY}} PUBLIC DISABLE_NOT_TESTABLE_CODE)
target_link_libraries(${{MAIN_BINARY}} PRIVATE ${{LIBHLEK_LIBRARY}} PUBLIC icu_data icu_uc icu_i18n icu_io)
target_include_directories(${{MAIN_BINARY}} PRIVATE ${{LIBHLEK_INSTALL_PATH}}
                ${{CURSES_INCLUDE_DIRS}}
                .)
