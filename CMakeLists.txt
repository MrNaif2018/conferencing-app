cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Network Multimedia REQUIRED)
if(WIN32)
    find_package(Qt5 COMPONENTS WinExtras REQUIRED)
endif()

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCE_FILES cvmatandqimage.cpp udpplayer.cpp screenrecorder.cpp Server.cpp)
set(HEADER_FILES cvmatandqimage.h udpplayer.h imageutil.h screenrecorder.h zoomui.h workerthread.h mainwindow.h startwindow.h settingswindow.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} resources.qrc)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Network Qt5::Multimedia ${OpenCV_LIBS})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} Qt5::WinExtras)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} X11 Xext Xfixes uvgrtp)
endif()