cmake_minimum_required(VERSION 3.20)

set(LIBCONFIG_NAME {__LIBCONFIG_NAME__})
set(LIBCONFIG_LIBNAME lib${{LIBCONFIG_NAME}})
set(LIBCONFIG_INSTALL_PATH "{__LIBCONFIG_INSTALL_PATH__}")

project(${{LIBCONFIG_NAME}} C CXX)

########################## LIBHLEK
set(HLEK_NAME {__HLEK_NAME__})
set(LIBHLEK_NAME {__LIBHLEK_NAME__})
set(LIBHLEK_INSTALL_PATH "{__LIBHLEK_INSTALL_PATH__}")
set(${{HLEK_NAME}}_DIR ${{LIBHLEK_INSTALL_PATH}})

list(APPEND CMAKE_MODULE_PATH ${{LIBHLEK_INSTALL_PATH}})
find_package(${{HLEK_NAME}} CONFIG REQUIRED)

if(${{${{HLEK_NAME}}_FOUND}})
    message(STATUS "libhlek found at ${{LIBHLEK_INSTALL_PATH}}")
endif()

aux_source_directory(src SOURCES)
include_directories(inc ${{LIBHLEK_INSTALL_PATH}}/..)
set(COMPILE_FLAGS "-std=c++11 -O0 -g")

set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${{CMAKE_CXX_FLAGS}} ${{COMPILE_FLAGS}} ${{EXTRA_FLAGS}}" )
set( CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99 ${{EXTRA_FLAGS}}" )
set( CMAKE_EXE_LINKER_FLAGS  "${{CMAKE_EXE_LINKER_FLAGS}} ${{LINK_FLAGS}}" )

########################## LIBRARY ##########################
add_library(${{LIBCONFIG_NAME}} STATIC ${{SOURCES}})
file(GLOB INSTALL_PUB_HEADERS "inc/*.h" "inc/*.hpp")
set_target_properties(${{LIBCONFIG_NAME}} PROPERTIES
                   PUBLIC_HEADER "${{INSTALL_PUB_HEADERS}}"
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)

message( STATUS "Software library installation path is: ${{INSTALL_PATH}}")

file(WRITE ${{LIBCONFIG_NAME}}Config.cmake
"add_library(${{LIBCONFIG_NAME}} STATIC IMPORTED)
find_library(LIBCONFIG_LIBRARY ${{LIBCONFIG_LIBNAME}}.a HINTS ${{LIBCONFIG_INSTALL_PATH}} )
set(${{LIBCONFIG_NAME}}_INCLUDE_DIRS \"${{LIBCONFIG_INSTALL_PATH}}\")
set_target_properties(${{LIBCONFIG_NAME}} PROPERTIES IMPORTED_LOCATION $\{{${{LIBCONFIG_NAME}}_INCLUDE_DIRS\}})
")

install(TARGETS ${{LIBCONFIG_NAME}}
        PUBLIC_HEADER
            DESTINATION ${{LIBCONFIG_INSTALL_PATH}}
        ARCHIVE
            DESTINATION ${{LIBCONFIG_INSTALL_PATH}}
)

install(FILES ${{LIBCONFIG_NAME}}Config.cmake
            DESTINATION ${{LIBCONFIG_INSTALL_PATH}})