cmake_minimum_required(VERSION 3.15)
project(saiblo-ai-cpp-template)

set(CMAKE_CXX_STANDARD 14)

add_executable(main main.cpp jsoncpp/jsoncpp.cpp sdk/Client.hpp MyAI.hpp)
