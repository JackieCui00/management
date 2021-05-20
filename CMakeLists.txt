cmake_minimum_required(VERSION 3.8.2)

project(hands_on_qt)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -g")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)
find_package(KF5WidgetsAddons REQUIRED)

add_executable(management main.cpp mainwindow.cpp mainwindow.ui resources.qrc)
target_link_libraries(management Qt5::Widgets Qt5::Core Qt5::Sql KF5WidgetsAddons)
