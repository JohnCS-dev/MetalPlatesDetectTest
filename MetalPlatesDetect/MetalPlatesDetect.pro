#-------------------------------------------------
#
# Project created by QtCreator 2020-03-06T11:57:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MetalPlatesDetect
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        opencvtoolwidgets.cpp \
        testmetaldetectwindow.cpp

HEADERS += \
        opencvtoolwidgets.h \
        testmetaldetectwindow.h

FORMS += \
        testmetaldetectwindow.ui

win32 {
    INCLUDEPATH += C:/OpenCV_401/include/
    LIBS += -LC:/OpenCV_401/x64/vc14/bin/
    LIBS += -LC:/OpenCV_401/x64/vc14/lib/
    OPENCV_VER = 410d
}

linux-g++ {
    INCLUDEPATH += $$(HOME)/OpenCV/include/
    LIBS += -L$$(HOME)/OpenCV/lib/
}

LIBS += -lopencv_core$${OPENCV_VER} \
        -lopencv_imgproc$${OPENCV_VER} \
        -lopencv_imgcodecs$${OPENCV_VER} \
        -lopencv_highgui$${OPENCV_VER} \
        -lopencv_flann$${OPENCV_VER} \
        -lopencv_calib3d$${OPENCV_VER} \
        -lopencv_features2d$${OPENCV_VER} \
        -lopencv_xfeatures2d$${OPENCV_VER} \
        -lopencv_bgsegm$${OPENCV_VER} \
        -lopencv_video$${OPENCV_VER} \
        -lopencv_videoio$${OPENCV_VER}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
