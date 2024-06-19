cmake_minimum_required(VERSION 3.10)

set(PROJECTNAME time_track_test)
set(MAIN_BINARY "${PROJECTNAME}.bin")

project(${PROJECTNAME} C CXX)
aux_source_directory(src SOURCES)

########################## LIBHLEK
set(HLEK_NAME hlek)
set(LIBHLEK_NAME libhlek)
set(LIBHLEK_INSTALL_PATH /usr/local/libhlek)
set(${HLEK_NAME}_DIR ${LIBHLEK_INSTALL_PATH})

list(APPEND CMAKE_MODULE_PATH ${LIBHLEK_INSTALL_PATH})
find_package(${HLEK_NAME} CONFIG REQUIRED)

if(${${HLEK_NAME}_FOUND})
    message(STATUS "libhlek found at ${LIBHLEK_INSTALL_PATH}")
endif()

########################## tb_ad9850dev
set(LIBCONFIG_NAME tb_ad9850dev)
set(LIBCONFIG_INSTALL_PATH "/usr/local/libtb_ad9850dev")
set(${LIBCONFIG_NAME}_DIR ${LIBCONFIG_INSTALL_PATH})

list(APPEND CMAKE_MODULE_PATH ${LIBCONFIG_INSTALL_PATH})
find_package(${LIBCONFIG_NAME} CONFIG REQUIRED)

if(${${LIBCONFIG_NAME}_FOUND})
    message(STATUS "${LIBCONFIG_NAME} found at ${LIBCONFIG_INSTALL_PATH}")
endif()
set(AD9850_LIBRARY ${LIBCONFIG_LIBRARY})
set(AD9850_INSTALL_PATH ${LIBCONFIG_INSTALL_PATH})

unset(LIBCONFIG_LIBRARY CACHE)


########################## tb_timetrackerdev
set(LIBCONFIG_NAME tb_timetrackerdev)
set(LIBCONFIG_INSTALL_PATH "/usr/local/libtb_timetrackerdev")
set(${LIBCONFIG_NAME}_DIR ${LIBCONFIG_INSTALL_PATH})

list(APPEND CMAKE_MODULE_PATH ${LIBCONFIG_INSTALL_PATH})
find_package(${LIBCONFIG_NAME} CONFIG REQUIRED)
if(${${LIBCONFIG_NAME}_FOUND})
    message(STATUS "${LIBCONFIG_NAME} found at ${LIBCONFIG_INSTALL_PATH}")
endif()

set(TIME_TRACK_LIBRARY ${LIBCONFIG_LIBRARY})
set(TIME_TRACK_INSTALL_PATH ${LIBCONFIG_INSTALL_PATH})
unset(LIBCONFIG_LIBRARY CACHE)

########################## COMPILE FLAGS
set(COMPILE_FLAGS "-std=c++11 -O0 -g -pthread -ldl -licudata -licui18n -licuio -licuuc")
#set(EXTRA_FLAGS "-DBLUE_PILL_SOFTWARE")
set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} ${EXTRA_FLAGS}" )
set( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 ${EXTRA_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}" )
#set(CMAKE_FIND_DEBUG_MODE TRUE)
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

########################## Main binary ##########################
add_executable(${MAIN_BINARY} src/main.cpp )
set_target_properties(${MAIN_BINARY} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_link_libraries(${MAIN_BINARY} PRIVATE ${LIBHLEK_LIBRARY} ${AD9850_LIBRARY} ${TIME_TRACK_LIBRARY} PUBLIC icu_data icu_uc icu_i18n icu_io)
target_include_directories(${MAIN_BINARY} PRIVATE ${LIBHLEK_INSTALL_PATH}/..
                ${AD9850_INSTALL_PATH}/..
                ${TIME_TRACK_INSTALL_PATH}/..
                ${CURSES_INCLUDE_DIRS}
                inc)
