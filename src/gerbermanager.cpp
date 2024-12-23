#include "include/gerbermanager.h"

#include <Python.h>

namespace py = pybind11;

GerberManager::GerberManager() {
    try {
        py::initialize_interpreter();

        PyGILState_STATE gstate = PyGILState_Ensure();

        py::module_ sys = py::module_::import("sys");

        // Log the Python executable being used
        qDebug() << "Python executable:" << QString::fromStdString(sys.attr("executable").cast<std::string>());

        // Set the PYTHONPATH environment in Python
        py::list path = sys.attr("path");
        path.append("C:/msys64/mingw64/bin");
        path.append("C:/msys64/mingw64/lib/python3.12");
        path.append("C:/msys64/mingw64/lib/python3.12/site-packages");
        path.append("C:/msys64/mingw64/lib/python3.12/lib-dynload");
        path.append("C:/Users/ahmed/OneDrive/Desktop/Projects/PCB2Gcode/python"); // 'gerber_wrapper.py' directory

        // Log modified Python path
        qDebug() << "Python path after appending:" << QString::fromStdString(py::str(path).cast<std::string>());

        // Import the custom gerber_wrapper module
        py::module_ gerberWrapper = py::module_::import("gerber_wrapper");

        // Instantiate the GerberWrapper class
        wrapper = gerberWrapper.attr("GerberWrapper")();

        // Create a temporary file path for rendered SVG images
        QTemporaryFile temp;
        temp.open();
        tempImagePath = temp.fileName().toStdString() + ".svg";
        temp.close();

        // Release the GIL
        PyGILState_Release(gstate);
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python initialization error:" << QString::fromStdString(e.what());
    }
}

GerberManager::~GerberManager() {
    try {
        // Remove the temporary file if it exists
        QFile::remove(QString::fromStdString(tempImagePath));
        // Finalize Python interpreter
        py::finalize_interpreter();
    }
    catch (...) {
        qDebug() << "Error finalizing Python interpreter.";
    }
}

bool GerberManager::loadGerberFile(const QString& filePath) {
    try {
        // Call the Python method to load the Gerber file
        bool success = wrapper.attr("load_gerber_file")(filePath.toStdString()).cast<bool>();

        if (success) {
            qDebug() << "Successfully loaded Gerber file:" << filePath;
        }

        return success;
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error while loading Gerber file:" << QString::fromStdString(e.what());
        return false;
    }
}




void GerberManager::clearGerberFiles() {
    try {
        wrapper.attr("clear_gerber_files")();
        qDebug() << "Cleared all loaded Gerber files.";
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error while clearing Gerber files:" << QString::fromStdString(e.what());
    }
}
