#include "include/GerberFileManager.h"
#include <QFile>
#include <QTextStream>

GCodeFileManager::GCodeFileManager()
{
}

bool GCodeFileManager::loadGCodeFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    gCodeData = in.readAll();
    file.close();
    return true;
}

bool GCodeFileManager::saveGCodeFile(const QString &filePath, const QString &gCodeContent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << gCodeContent;
    file.close();
    return true;
}
