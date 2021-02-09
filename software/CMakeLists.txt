cmake_minimum_required(VERSION 3.10)

set(SW_NAME hlek)
project(${SW_NAME}sw C CXX)

aux_source_directory(src SOURCES)
aux_source_directory(termui TERMUI)
include_directories(inc termui ${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

set(COMPILE_FLAGS "-std=c++11 -O0 -g -pthread -ldl -licudata -licui18n -licuio -licuuc")
set(EXTRA_FLAGS "-DBLUE_PILL_SOFTWARE")

set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} ${EXTRA_FLAGS}" )
set( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99 ${EXTRA_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}" )


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

########################## LIBRARY ##########################
set(SW_LIB ${SW_NAME})
add_library(${SW_LIB} STATIC ${SOURCES})
set_target_properties(${SW_LIB} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)

########################## TESTTOOL ##########################
set(SW_TEST ${SW_NAME}test.out)
add_executable(${SW_TEST} testtool/testtool.cpp testtool/circbuffer.c testtool/utools.c)
add_dependencies(${SW_TEST} ${SW_LIB})
set_target_properties(${SW_TEST} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_compile_definitions(${SW_TEST} PUBLIC DISABLE_NOT_TESTABLE_CODE)
target_link_libraries(${SW_TEST} PRIVATE ${SW_LIB} PUBLIC icu_data icu_uc icu_i18n icu_io )

########################## MONITOR ##########################
set(SW_MONITOR ${SW_NAME}mon.out)
add_executable(${SW_MONITOR} monitor/monitor.cpp monitor/handlers.cpp ${TERMUI})
add_dependencies(${SW_MONITOR} ${SW_LIB})
set_target_properties(${SW_MONITOR} PROPERTIES
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)
target_link_libraries(${SW_MONITOR} PRIVATE ${SW_LIB} PUBLIC icu_data icu_uc icu_i18n icu_io ${CURSES_LIBRARIES})
target_include_directories(${SW_MONITOR} PRIVATE ${CURSES_INCLUDE_DIRS})

