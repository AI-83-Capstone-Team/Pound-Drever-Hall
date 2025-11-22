cmake_minimum_required(VERSION 3.14)
project(PDH_EXECUTABLES)


set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)
set(C_STANDARD 11)


if(NOT DEFINED DEBUG_REG)
  set(DEBUG_REG OFF)
endif()


if(NOT DEFINED VERSION)
  set(VERSION 0.00-0000)
endif()

if(NOT DEFINED REVISION)
  set(REVISION devbuild)
endif()


message(STATUS "VERSION=${VERSION}")
message(STATUS "REVISION=${REVISION}")
message(STATUS "Install path ${INSTALL_DIR}")

message(STATUS "Compiler C path: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler C ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler C version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler C is part: ${CMAKE_COMPILER_IS_GNUC}")

message(STATUS "Compiler C++ path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler C++ ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler C++version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler C++ is part: ${CMAKE_COMPILER_IS_GNUCXX}")



set(CMAKE_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/"api")
set(CMAKE_CONTROL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/"control")
set(CMAKE_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/"out")


include_directories(${CMAKE_API_DIR}/"inc")
include_directories(${CMAKE_CONTROL_DIR}/"inc")


file(GLOB src_files
    "${CMAKE_CONTROL_DIR}/src/*.c"
    "${CMAKE_API_DIR}/src/*.c"
)


add_compile_options(-mcpu=cortex-a9 -mfpu=neon-fp16 -fPIC)
add_compile_definitions(ARCH_ARM)
add_compile_options(-Wall -Wextra -Wno-unused-parameter -DVERSION=${VERSION} -DREVISION=${REVISION} $<$<CONFIG:Debug>:-g3> $<$<CONFIG:Debug>:-DTRACE_ENABLE> $<$<CONFIG:Release>:-Os> -ffunction-sections -fdata-sections)
if(DEBUG_REG)
add_compile_options(-DDEBUG_REG)
endif()


