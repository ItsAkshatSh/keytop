cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEYPHYR_BASE})

project(xiao_kb)

add_subdirectory(drivers/input)