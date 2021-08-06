cmake_minimum_required(VERSION 3.10)
project(saiblo-logic-cpp-template)

set(CMAKE_CXX_STANDARD 14)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(main main.cpp jsoncpp/jsoncpp.cpp sdk/BaseLogic.cpp sdk/BaseLogic.h example/RspLogic.cpp example/RspLogic.h)
