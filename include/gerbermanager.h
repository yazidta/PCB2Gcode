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

    bool loadGerberFiles(const QStringList& filePaths);
    void clearGerberFiles();
    QPixmap renderGerber(int dpmm=40);
    QList<TestPoint> loadTestPoints(const QString& csvPath);
    QPixmap overlayTestPoints(const QPixmap& baseImage, const QList<TestPoint>& points);
    void getBoundingBox();
    std::pair<std::vector<PadInfo>, std::vector<TraceInfo>>getPadAndTraceCoordinates();





private:
    py::object gerberStack;
    std::string tempImagePath;
    GCodeConverter* gcodeConverter;
    std::vector<TraceInfo> TraceCoordinates;
    std::vector<PadInfo> padCoordinates;
    // Bounding Box coords
    double minX;
    double minY;
    double maxX;
    double maxY;

};

#endif // GERBERMANAGER_H
