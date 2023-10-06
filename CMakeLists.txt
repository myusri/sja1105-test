cmake_minimum_required(VERSION 3.10)

project(SJA1105_Test)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(sj1105-config sja1105-config.c mcp2210_api.c)
