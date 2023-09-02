cmake_minimum_required(VERSION 3.20)

set(HLEK_NAME {__HLEK_NAME__})
set(LIBHLEK_NAME {__LIBHLEK_NAME__})
set(LIBHLEK_INSTALL_PATH "{__LIBHLEK_INSTALL_PATH__}")

project(${{LIBHLEK_NAME}} C CXX)

aux_source_directory(src SOURCES)
include_directories(inc ${{ICU_INCLUDE_DIRS}})
link_directories(${{ICU_LIBRARY_DIRS}})

set(COMPILE_FLAGS "-std=c++11 -O0 -g -pthread")

set(LINK_FLAGS "-rdynamic")
set( CMAKE_CXX_FLAGS  "${{CMAKE_CXX_FLAGS}} ${{COMPILE_FLAGS}} ${{EXTRA_FLAGS}}" )
set( CMAKE_C_FLAGS  "${{CMAKE_C_FLAGS}} -std=c99 ${{EXTRA_FLAGS}}" )
set( CMAKE_EXE_LINKER_FLAGS  "${{CMAKE_EXE_LINKER_FLAGS}} ${{LINK_FLAGS}}" )

########################## LIBRARY ##########################
add_library(${{HLEK_NAME}} STATIC ${{SOURCES}})
file(GLOB INSTALL_PUB_HEADERS "inc/*.h" "inc/*.hpp")
set_target_properties(${{HLEK_NAME}} PROPERTIES
                   PUBLIC_HEADER "${{INSTALL_PUB_HEADERS}}"
                   RUNTIME_OUTPUT_DIRECTORY_DEBUG build/debug
                   RUNTIME_OUTPUT_DIRECTORY_RELEASE build/release)

message( STATUS "Library installation path is: ${{LIBHLEK_INSTALL_PATH}}")

file(WRITE ${{HLEK_NAME}}Config.cmake
"add_library(${{HLEK_NAME}} STATIC IMPORTED)
find_library(LIBHLEK_LIBRARY ${{LIBHLEK_NAME}}.a HINTS ${{LIBHLEK_INSTALL_PATH}} )
set(${{HLEK_NAME}}_INCLUDE_DIRS \"${{LIBHLEK_INSTALL_PATH}}\")
set_target_properties(${{HLEK_NAME}} PROPERTIES IMPORTED_LOCATION $\{{${{HLEK_NAME}}_INCLUDE_DIRS\}})
")

install(TARGETS ${{HLEK_NAME}}
        PUBLIC_HEADER
            DESTINATION ${{LIBHLEK_INSTALL_PATH}}
        ARCHIVE
            DESTINATION ${{LIBHLEK_INSTALL_PATH}}
)

install(FILES ${{HLEK_NAME}}Config.cmake
            DESTINATION ${{LIBHLEK_INSTALL_PATH}})

