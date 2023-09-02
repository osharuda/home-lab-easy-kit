cmake_minimum_required(VERSION 3.20)

set(HLEK_INSTALL_PATH "/opt/radpattern")
set(DEVICE_NAME "radpattern")
set(PROJECTNAME "main")
set(MAIN_BINARY "${PROJECTNAME}.bin")

project(${PROJECTNAME} C CXX)

aux_source_directory(src SOURCES)

set(COMPILE_FLAGS "-std=c++11 -O0 -g -pthread -ldl -licudata -licui18n -licuio -licuuc")
set(EXTRA_FLAGS "-DBLUE_PILL_SOFTWARE")
set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} ${EXTRA_FLAGS}" )
set( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 ${EXTRA_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}" )
set(CMAKE_FIND_DEBUG_MODE TRUE)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

########################## ICU Library
find_package(ICU COMPONENTS data uc i18n io REQUIRED)
add_library(icu_data SHARED IMPORTED)
set_property(TARGET icu_data PROPERTY IMPORTED_LOCATION ${ICU_DATA_LIBRARIES})
add_library(icu_uc SHARED IMPORTED)
set_property(TARGET icu_uc PROPERTY IMPORTED_LOCATION ${ICU_UC_LIBRARIES})
add_library(icu_i18n SHARED IMPORTED)
set_property(TARGET icu_i18n PROPERTY IMPORTED_LOCATION ${ICU_I18N_LIBRARIES})
add_library(icu_io SHARED IMPORTED)
set_property(TARGET icu_io PROPERTY IMPORTED_LOCATION ${ICU_IO_LIBRARIES})
set(ICU_TARGETS "icu_data icu_uc icu_i18n icu_io")

########################## NCURSES Library
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)

########################## Customized HLEK Library
set(HLEK_LIB ${DEVICE_NAME})
list(APPEND CMAKE_MODULE_PATH ${HLEK_INSTALL_PATH}/cmake)
find_package(${HLEK_LIB} CONFIG REQUIRED)

if(${${DEVICE_NAME}_FOUND})
    message(STATUS "HLEK found!")
    message(STATUS "HLEK install path: ${${DEVICE_NAME}_LIBRARY_PATH}")
    message(STATUS "HLEK include dirs: ${HLEK_INCLUDE_DIRS}")
endif()

########################## Main binary ##########################
add_executable(${MAIN_BINARY} src/main.cpp )
set_target_properties(${MAIN_BINARY} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_link_libraries(${MAIN_BINARY} PRIVATE ${HLEK_LIB} PUBLIC icu_data icu_uc icu_i18n icu_io ${CURSES_LIBRARIES})
target_include_directories(${MAIN_BINARY} PRIVATE ${HLEK_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS} inc)
