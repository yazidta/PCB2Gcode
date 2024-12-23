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
PYTHON_VERSIONLIB = 3.9  # Adjust this to your installed Python version
PYTHON_VERSION = 3.12  # Adjust this to your installed Python version

PYTHON_PATH = C:/msys64/mingw64/
 # Using Python from MSYS2 environment

# Python Include paths
INCLUDEPATH += \
    $$PYTHON_PATH/include \
    $$PYTHON_PATH/include/python$$PYTHON_VERSION \
    $$PYTHON_PATH/lib/python$$PYTHON_VERSION/site-packages/include \
    $$PYTHON_PATH/lib/python$$PYTHON_VERSION/site-packages/pybind11/include

# Python Library path
LIBS += -L$$PYTHON_PATH/lib/ -lpython$$PYTHON_VERSION

# Build Custom Commands for Python Script
copydata.commands = \
    $(CHK_DIR_EXISTS) "$$shell_path($$OUT_PWD/python)" $(MKDIR) "$$shell_path($$OUT_PWD/python)" && \
    $(COPY_FILE) "$$shell_path($$PWD/python/gerber_wrapper.py)" "$$shell_path($$OUT_PWD/python/gerber_wrapper.py)"

first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)

QMAKE_EXTRA_TARGETS += first copydata

# Forms
FORMS += \
    ui/PCB2Gcode.ui

# Translations
TRANSLATIONS += \
    translations/PCB2Gcode_en_US.ts

# Python script files
DISTFILES += \
    python/gerber_wrapper.py
