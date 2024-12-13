
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Name of the project
TARGET = FPTApplication
TEMPLATE = app

# Source, header, and UI file locations
SOURCES += \
    src/PCB2Gcode.cpp \
    src/main.cpp

HEADERS += \
    include/pcb2gcode.h

FORMS += \
    ui/PCB2Gcode.ui

TRANSLATIONS += \
    translations/PCB2Gcode_en_US.ts
