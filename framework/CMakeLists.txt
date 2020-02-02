#######################################################
#
#              Cmakelist Simulator
#
#######################################################
cmake_minimum_required(VERSION 3.4)

project(test_project)

include_directories( .
)

list(APPEND framework_inc
    "inc/clock.hpp"
)

list(APPEND framework_src
    "src/clock.cpp"
)

#add_executable(MyExec ${hello_src})
add_library(frameworkLib STATIC ${framework_src} ${framework_inc})
