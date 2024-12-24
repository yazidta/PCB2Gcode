#ifndef GERBERMANAGER_H
#define GERBERMANAGER_H
#undef slots
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <QTemporaryFile>
#include <QPixmap>
#include <QSvgRenderer>
#include <QString>
#include <string>


namespace py = pybind11;

class GerberManager {
public:
    GerberManager();
    ~GerberManager();

    bool loadGerberFiles(const QStringList& filePaths);
    void clearGerberFiles();
    QPixmap renderGerber(const QString& outputPath, int dpmm=40);


private:
    py::object wrapper;
    std::string tempImagePath;
};

#endif // GERBERMANAGER_H
