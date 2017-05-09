QT       += core gui widgets

CONFIG += c++11

TARGET = QDataflowCanvas
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qdataflowcanvas.cpp \
    qdataflowmodel.cpp

HEADERS  += mainwindow.h \
    qdataflowcanvas.h \
    qdataflowmodel.h

FORMS += \
    mainwindow.ui
