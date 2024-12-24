#include "include/gerbermanager.h"

#include <QDebug>
#include <QFile>
#include <QPainter>
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
        path.append("C:/Users/ahmed/OneDrive/Desktop/Projects/PCB2Gcode/python"); // 'gerber_wrapper.py' directory

        // Log modified Python path
        qDebug() << "Python path after appending:" << QString::fromStdString(py::str(path).cast<std::string>());

        // Import the custom gerber_wrapper module
        py::module_ gerberWrapper = py::module_::import("gerber_wrapper");

        // Instantiate the GerberWrapper class
        gerberStack  = gerberWrapper.attr("GerberWrapper")();

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
        py::finalize_interpreter();
    }
    catch (...) {
        qDebug() << "Error finalizing Python interpreter.";
    }
}

bool GerberManager::loadGerberFiles(const QStringList& filePaths) {
    try {
        py::gil_scoped_acquire acquire;
        py::list py_file_paths;
        for (const QString& path : filePaths) {
            py_file_paths.append(path.toStdString());
        }
        gerberStack.attr("loadGerberFiles")(py_file_paths);
        qDebug() << "Gerber files loaded successfully.";
        return true;
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error while loading Gerber files:" << QString::fromStdString(e.what());
        return false;
    }
}

void GerberManager::clearGerberFiles() {
    try {
        gerberStack.attr("clearGerberFiles")();
        qDebug() << "Cleared all loaded Gerber files.";
    } catch (const py::error_already_set& e) {
        qDebug() << "Python error while clearing Gerber files:" << QString::fromStdString(e.what());
    }
}

QPixmap GerberManager::renderGerber(const QString& outputPath, int dpmm) {
    try {
        py::gil_scoped_acquire acquire;
        gerberStack.attr("renderToPng")(outputPath.toStdString(), dpmm);

        QPixmap pixmap;
        if (pixmap.load(outputPath)) {
            qDebug() << "Successfully loaded rendered PNG into QPixmap.";
            return pixmap;
        } else {
            qDebug() << "Failed to load rendered PNG into QPixmap.";
            return QPixmap();
        }
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error during rendering:" << QString::fromStdString(e.what());
        return QPixmap();
    }
}
