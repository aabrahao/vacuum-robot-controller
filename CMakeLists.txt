cmake_minimum_required(VERSION 3.16)

project(vacuum-robot-controller VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)

qt_add_executable(vacuum-robot-controller
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/AbstractVideo.cpp
    src/AbstractVideo.h
    src/CameraVideo.cpp
    src/CameraVideo.h
    src/MediaVideo.cpp
    src/MediaVideo.h
    src/VideoPlayer.cpp
    src/VideoPlayer.h
    src/VideoToolbar.cpp
    src/VideoToolbar.h
    src/WorkspaceView.cpp
    src/WorkspaceView.h
    src/Settings.cpp
    src/Settings.h
    resources/settings.qrc
)

target_link_libraries(vacuum-robot-controller PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

install(TARGETS vacuum-robot-controller
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
