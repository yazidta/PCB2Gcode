#ifndef GERBERMANAGER_H
#define GERBERMANAGER_H
#undef slots
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <QTemporaryFile>
#include <QDir>
#include <QDebug>
#include <QPainter>
#include <QFile>
#include <QByteArray>
#include <QBuffer>
#include <QSvgRenderer>
#include <QString>
#include <string>


namespace py = pybind11;

class GerberManager {
public:
    GerberManager();
    ~GerberManager();

    bool loadGerberFile(const QString& filePath);
    void clearGerberFiles();

private:
    py::module_ gerberWrapper;
    py::object wrapper;
    std::string tempImagePath;
};

#endif // GERBERMANAGER_H
