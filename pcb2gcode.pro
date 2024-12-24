QT += core gui serialport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FPTApplication
TEMPLATE = app

SOURCES += \
    src/gcodeConverter.cpp \
    src/gerbermanager.cpp \
    src/main.cpp \
    src/pcb2gcode.cpp \
    src/settings.cpp \
    src/uart.cpp

HEADERS += \
    include/pcb2gcode.h \
    include/settings.h \
    include/uart.h \
    include/gcodeConverter.h \
    include/gerbermanager.h

# Python Configuration
PYTHON_VERSION = 3.12  # Adjust this to your installed Python version
PYTHON_VERSIONLIB = 312  # Adjust this to your installed Python library version(for anaconda there shouldn't be a " . ")

# Conda Environment Path
CONDA_ENV_PATH = C:/Users/ahmed/.conda/envs/pcb2gcode_env

# Python Include paths
INCLUDEPATH += \
    $$CONDA_ENV_PATH/include \
    $$CONDA_ENV_PATH/include/python$$PYTHON_VERSION \
    $$CONDA_ENV_PATH/Lib/site-packages/include \
    $$CONDA_ENV_PATH/Lib/site-packages/pybind11/include

# Python Library path
LIBS += -L$$CONDA_ENV_PATH/libs/ -lpython$$PYTHON_VERSIONLIB

# Forms
FORMS += \
    ui/PCB2Gcode.ui

# Translations
TRANSLATIONS += \
    translations/PCB2Gcode_en_US.ts

# Python script files
DISTFILES += \
    python/gerber_wrapper.py

RESOURCES += \
    resources.qrc
