#-------------------------------------------------
#
# Project created by QtCreator 2016-04-05T16:04:03
#
#-------------------------------------------------

QT       += core gui webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogicAUBS
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    proofwindow.cpp \
    proof.cpp

HEADERS  += mainwindow.h \
    proofwindow.h \
    proof.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
