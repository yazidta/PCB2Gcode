cmake_minimum_required(VERSION 3.5)

project(PCB2Gcode VERSION 0.1 LANGUAGES CXX)

# Qt setup
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 (or Qt5) for Widgets and LinguistTools
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

message(STATUS "Detected Qt Version: ${QT_VERSION}")
message(STATUS "Detected Qt Major Version: ${QT_VERSION_MAJOR}")

# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Debugging outputs
message(STATUS "Python Version: ${Python3_VERSION}")
message(STATUS "Python Include Directory: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python Library: ${Python3_LIBRARIES}")

# Add translation files
set(TS_FILES PCB2Gcode_en_US.ts)

# Add source files
set(PROJECT_SOURCES
    main.cpp
    fptapplication.cpp
    fptapplication.h
    fptapplication.ui
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PCB2Gcode
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        pcb2gcode.pro
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PCB2Gcode SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(PCB2Gcode
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# Link Qt libraries
target_link_libraries(PCB2Gcode PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Svg)



target_include_directories(PCB2Gcode PRIVATE
    ${LIBGERBV_INCLUDE_DIRS}
)

target_link_libraries(PCB2Gcode PRIVATE
    ${LIBGERBV_LIBRARIES}
)

# Link Python
target_include_directories(PCB2Gcode PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(PCB2Gcode PRIVATE ${Python3_LIBRARIES})

# Qt for iOS settings
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PCB2Gcode)
endif()

set_target_properties(PCB2Gcode PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installation settings
include(GNUInstallDirs)
install(TARGETS PCB2Gcode
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize the Qt executable (for Qt6)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PCB2Gcode)
endif()
