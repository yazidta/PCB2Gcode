
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Name of the project
TARGET = FPTApplication
TEMPLATE = app

# Source, header, and UI file locations
SOURCES += \
    src/GerberFileManager.cpp \
    src/PCB2Gcode.cpp \
    src/main.cpp \
    src/settings.cpp

HEADERS += \
    include/GerberFileManager.h \
    include/pcb2gcode.h \
    include/settings.h

FORMS += \
    ui/PCB2Gcode.ui

TRANSLATIONS += \
    translations/PCB2Gcode_en_US.ts
