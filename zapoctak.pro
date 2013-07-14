QT       += core

QT       -= gui

TARGET = zapoctak
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    wave.cpp \
    fft.cpp \
    data_utility.cpp \
    complex.cpp

HEADERS += \
    wave.h \
    fft.h \
    data_utility.h \
    complex.h
