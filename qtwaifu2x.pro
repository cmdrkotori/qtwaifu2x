#-------------------------------------------------
#
# Project created by QtCreator 2016-03-16T22:19:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtwaifu2x
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        waifuprocess.cpp

HEADERS  += mainwindow.h \
    waifuprocess.h

FORMS    += mainwindow.ui

DISTFILES += \
    .gitignore \
    README.md \
    LICENSE

RESOURCES += \
    resources.qrc
