cmake_minimum_required(VERSION 3.15)
project(saiblo-logic-cpp-template)

set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp jsoncpp/jsoncpp.cpp sdk/BaseLogic.cpp sdk/BaseLogic.h example/RspLogic.cpp example/RspLogic.h)
