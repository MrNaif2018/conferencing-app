TEMPLATE = app
TARGET = Server
CONFIG += ccache
CONFIG += precompile_header
#CONFIG += debug

QT += gui widgets network multimedia concurrent
CONFIG += c++14

LIBS+=-lopencv_core -lopencv_imgproc -lopencv_imgcodecs
INCLUDEPATH+=/usr/include/opencv4

LIBS+= -luvgrtp

win32: QT += winextras
unix:!macx: LIBS += -lX11 -lXext -lXfixes

SOURCES += cvmatandqimage.cpp udpplayer.cpp screenrecorder.cpp Server.cpp
HEADERS += cvmatandqimage.h udpplayer.h imageutil.h screenrecorder.h zoomui.h workerthread.h mainwindow.h

