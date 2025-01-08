#ifndef GERBERMANAGER_H
#define GERBERMANAGER_H
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <QTemporaryFile>
#include <QPixmap>
#include <QSvgRenderer>
#include <QString>
#include <string>
#include "include/gcodeConverter.h"


namespace py = pybind11;



class GCodeConverter;

class GerberManager {
public:
    GerberManager();
    ~GerberManager();
    // PCB Bounding Box
    double minX;
    double minY;
    double maxX;
    double maxY;
    void getBoundingBox();
    bool loadGerberFiles(const QStringList& filePaths);
    void clearGerberFiles();
    QPixmap renderGerber(int dpmm=40);
    QList<TestPoint> getPadInfo();
    QList<Trace> getTraceCoords();
    QPixmap overlayTestPoints(const QPixmap& baseImage, const QList<TestPoint>& points);



private:
    py::object GerberWrapper;
    std::string tempImagePath;
    GCodeConverter* gcodeConverter;
    // Bounding Box coords


};

#endif // GERBERMANAGER_H
