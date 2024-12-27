#ifndef GERBERMANAGER_H
#define GERBERMANAGER_H
#undef slots
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "include/gcodeConverter.h"
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
    QPixmap renderGerber(int dpmm=40);
    QList<TestPoint> loadTestPoints(const QString& csvPath);
    QPixmap overlayTestPoints(const QPixmap& baseImage, const QList<TestPoint>& points);
    void getBoundingBox();




private:
    py::object gerberStack;
    std::string tempImagePath;
    GCodeConverter gcodeConverter;

    // Bounding Box coords
    double minX;
    double minY;
    double maxX;
    double maxY;

};

#endif // GERBERMANAGER_H
