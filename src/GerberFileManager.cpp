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

QStringList GerberFileManager::extractTestPoints() const
{
    QStringList testPoints;

    // Example logic to extract test points from Gerber data
    // In a real implementation, parsing should follow Gerber format specifications
    foreach (const QString &line, gerberData.split("\n")) {
        if (line.startsWith("X") && line.contains("Y")) {
            testPoints.append(line.trimmed());
        }
    }

    return testPoints;
}

QString GerberFileManager::generateGCodeForTestPoints(const QStringList &testPoints) const
{
    QString gCode;

    gCode.append("G21 ; Set units to millimeters\n");
    gCode.append("G90 ; Absolute positioning\n");

    for (const QString &point : testPoints) {
        QString xCoord = point.section('X', 1, 1).section('Y', 0, 0).trimmed();
        QString yCoord = point.section('Y', 1, 1).trimmed();

        gCode.append(QString("G1 X%1 Y%2 F500 ; Move to test point\n").arg(xCoord).arg(yCoord));
    }

    gCode.append("M30 ; End of program\n");
    return gCode;
}

bool GerberFileManager::saveGCodeFile(const QString &filePath, const QString &gCodeContent)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to save G-code file:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out << gCodeContent;
    file.close();
    return true;
}
