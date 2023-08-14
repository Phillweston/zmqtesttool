# Copyright (C) 2023 The JOUAV Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(zmqtesttool LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/simpleclient")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Mqtt Network Widgets)

qt_add_executable(zmqtesttool
    src/main.cpp
    src/mainwindow.cpp
    src/aboutdialog.cpp
    include/mainwindow.h
    include/aboutdialog.h
    ui/mainwindow.ui
    ui/aboutdialog.ui
)

set_target_properties(zmqtesttool PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_compile_definitions(zmqtesttool PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(zmqtesttool PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
)

target_link_libraries(zmqtesttool PUBLIC Qt::Widgets)

if((QT_MAJOR_VERSION GREATER 4))
    target_link_libraries(zmqtesttool PUBLIC
        Qt::Widgets
    )
endif()

install(TARGETS zmqtesttool
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)