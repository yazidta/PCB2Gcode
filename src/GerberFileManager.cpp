#include "include/GerberFileManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

GerberFileManager::GerberFileManager()
{
}

bool GerberFileManager::loadGerberFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open Gerber file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    gerberData = in.readAll();
    file.close();
    return true;
}

