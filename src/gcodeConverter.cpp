#include "include/gcodeConverter.h"
#include "include/gerbermanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

GCodeConverter::GCodeConverter(GerberManager* gerberManager):gerberManager(gerberManager)
{

}



bool GCodeConverter::loadCSVFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    QString headerLine = in.readLine(); // Skip header

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');

        if (parts.size() != 9) continue;

        TestPoint tp;
        tp.sourceRefDes = parts[0].remove('"');
        tp.sourcePad = parts[1].remove('"');
        tp.net = parts[2].remove('"');
        tp.netClass = parts[3].remove('"');
        tp.side = parts[4].remove('"');
        tp.x = parts[5].toDouble();
        tp.y = parts[6].toDouble();
        tp.padType = parts[7].remove('"');
        tp.footprintSide = parts[8].remove('"');

        testPoints.append(tp);
    }

    file.close();
    return true;
}

QList<TestPoint> GCodeConverter::filterTopSidePoints() const
{
    QList<TestPoint> filtered;
    for (const TestPoint &tp : testPoints) {
        if (tp.side == "TOP") {
            filtered.append(tp);
        }
    }
    return filtered;
}

QMap<QString, QList<TestPoint>> GCodeConverter::groupByNet(const QList<TestPoint> &testPoints) const
{
    QMap<QString, QList<TestPoint>> grouped;
    for (const TestPoint &tp : testPoints) {
        grouped[tp.net].append(tp);
    }
    return grouped;
}
std::vector<int> GCodeConverter::countTracesConnectedToPads() const
{
    auto [pads, traces] = gerberManager->getPadAndTraceCoordinates();
    std::vector<int> padTraceCount;
    qDebug()<<"the function started" <<pads.size();
    // Loop through each pad
    for (size_t padIndex = 0; padIndex < pads.size(); ++padIndex)
    {
        const auto& pad = pads[padIndex];
        int connectedTraces = 0;
       // qDebug()<<"the function started11111";

        // Loop through each trace
        for (const auto& trace : traces)
        {
            //qDebug()<<"the function started99999";

            // Compare the trace start and end points with the pad coordinates
            bool isConnectedToStart = std::abs(trace.start_x - pad.x) == 0 &&
                                      std::abs(trace.start_y - pad.y) == 0;

            bool isConnectedToEnd = std::abs(trace.end_x - pad.x) == 0 &&
                                    std::abs(trace.end_y - pad.y) == 0;

            if (isConnectedToStart || isConnectedToEnd)
            {
                ++connectedTraces;
            }
        }

        // Store the count for this pad
        padTraceCount.emplace_back(connectedTraces);
        qDebug()<< "Pad at (" << pad.x << ", " << pad.y << ") is connected to "
                  << connectedTraces << " traces.\n";
    }

    return padTraceCount;
}

QString GCodeConverter::generateGCodeFromCSV(const QMap<QString, QList<TestPoint>> &groupedTestPoints) const
{
    QString gCode;
    gCode += "G21 ; Set units to millimeters\n";
    gCode += "G90 ; Absolute positioning\n";

    for (const QString &net : groupedTestPoints.keys()) {
        gCode += QString("; Net: %1\n").arg(net);

        for (const TestPoint &tp : groupedTestPoints[net]) {
            gCode += QString("G0 X%1 Y%2 ; Move to test point\n").arg(tp.x).arg(tp.y);
            gCode += "G1 Z-1 ; Lower probe\n";
            gCode += "G1 Z1 ; Raise probe\n";
        }
    }

    gCode += "M30 ; End of program\n";
    return gCode;
}

bool GCodeConverter::extractPadAndTraceCoords()
{
    auto [pads, traces] = gerberManager->getPadAndTraceCoordinates();

    if (pads.empty() && traces.empty()) {
        qWarning() << "No pad or trace coordinates extracted.";
        return false;
    }

    if (!pads.empty()) {
        qDebug() << "Extracted pad coordinates successfully:";
        for (const auto& pad : pads) {
            qDebug() << "Pad - XY: (" << pad.x << ", " << pad.y << "), Aperture: " << QString::fromStdString(pad.aperture);
        }
    } else {
        qDebug() << "No pad coordinates found.";
    }

    if (!traces.empty()) {
        qDebug() << "Extracted trace coordinates successfully:";
        for (const auto& trace : traces) {
            qDebug() << "Trace - Start: (" << trace.start_x << ", " << trace.start_y
                     << "), End: (" << trace.end_x << ", " << trace.end_y;

        }
    } else {
        qDebug() << "No trace coordinates found.";
    }

    return true;
}


QString GCodeConverter::generateGcodeFromGerber()
{
    extractPadAndTraceCoords();
    QString gCode;
    countTracesConnectedToPads();

    gCode += "G21 ; Set units to millimeters\n";
    gCode += "G90 ; Absolute positioning\n";


    for(const auto& coord : padCoords){
        double x = coord.x;
        double y = coord.y;
        gCode += QString("G0 X%1 Y%2 ; Move to test point\n").arg(x).arg(y);
        gCode += "G1 Z-1 ; Lower probe\n";
        gCode += "G1 Z1 ; Raise Probe\n";
    }
    gCode += "M30 ; End of Program\n";
    return gCode;
}



bool GCodeConverter::saveGCodeToFile(const QString &filePath, const QString &gCodeContent)
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

