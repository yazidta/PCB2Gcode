#include "include/gcodeConverter.h"
#include "include/gerbermanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QQueue>

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
        tp.source = parts[0].remove('"');
        tp.pin = parts[1].toInt();
        tp.net = parts[2].remove('"');
        tp.x = parts[5].toDouble() - 130.4005; // Subtracting boundBox o
        tp.y = parts[6].toDouble() - -134.5995;
        tp.type = parts[7].remove('"');
        testPointsCSV.append(tp);
    }

    file.close();
    return true;
}
QList<TestPoint> GCodeConverter::getTestPointsCSV()
{
    return testPointsCSV;
}
QList<TestPoint> GCodeConverter::getTestPointsGerber(){
    testPointsGerber = gerberManager->getPadInfo();
    return testPointsGerber;
}

QMap<QString, QList<TestPoint>> GCodeConverter::groupByNet(const QList<TestPoint> &testPoints) const
{
    QMap<QString, QList<TestPoint>> grouped;
    for (const TestPoint &tp : testPoints) {
        grouped[tp.net].append(tp);
    }
    return grouped;
}

QMap<QString, QList<TestPoint>> GCodeConverter::prioritizeEdgesAndSingleTracePoints(const QList<TestPoint> &testPoints) const {
    QMap<QString, QList<TestPoint>> groupedTestPoints = groupByNet(testPoints);
    QMap<QString, QList<TestPoint>> optimized;

    auto traces = gerberManager->getTraceCoords(); // Load traces once at the start
    const double TOLERANCE = 1e-4;

    for (auto it = groupedTestPoints.begin(); it != groupedTestPoints.end(); ++it) {
        const QList<TestPoint> &netTestPoints = it.value();
        QMap<int, TestPoint> pointIndexMap;
        for (int i = 0; i < netTestPoints.size(); ++i) {
            pointIndexMap[i] = netTestPoints[i];
        }

        QMap<int, QList<int>> adjacencyList;

        QMap<QPair<double, double>, int> virtualNodeMap;
        int nextVirtualNode = netTestPoints.size();

        for (const auto &trace : traces) {
            QList<int> startMatches, endMatches;

            for (int i = 0; i < netTestPoints.size(); ++i) {
                const auto &tp = netTestPoints[i];
                if (std::abs(tp.x - trace.startX) < TOLERANCE && std::abs(tp.y - trace.startY) < TOLERANCE) {
                    startMatches.append(i);
                }
                if (std::abs(tp.x - trace.endX) < TOLERANCE && std::abs(tp.y - trace.endY) < TOLERANCE) {
                    endMatches.append(i);
                }
            }

            if (startMatches.isEmpty()) {
                QPair<double, double> startCoord = {trace.startX, trace.startY};
                if (!virtualNodeMap.contains(startCoord)) {
                    virtualNodeMap[startCoord] = nextVirtualNode++;
                }
                startMatches.append(virtualNodeMap[startCoord]);
            }

            if (endMatches.isEmpty()) {
                QPair<double, double> endCoord = {trace.endX, trace.endY};
                if (!virtualNodeMap.contains(endCoord)) {
                    virtualNodeMap[endCoord] = nextVirtualNode++;
                }
                endMatches.append(virtualNodeMap[endCoord]);
            }

            for (int startIndex : startMatches) {
                for (int endIndex : endMatches) {
                    if (startIndex != endIndex) {
                        adjacencyList[startIndex].append(endIndex);
                        adjacencyList[endIndex].append(startIndex);
                    }
                }
            }
        }

        if (netTestPoints.isEmpty()) {
            continue;
        }

        auto bfs = [&](int startNode) -> QPair<int, int> {
            QMap<int, int> distances;
            QQueue<int> queue;

            for (int i = 0; i < nextVirtualNode; ++i) {
                distances[i] = -1;
            }
            queue.enqueue(startNode);
            distances[startNode] = 0;

            int farthestNode = startNode;
            while (!queue.isEmpty()) {
                int currentNode = queue.dequeue();

                for (int neighbor : adjacencyList[currentNode]) {
                    if (distances[neighbor] == -1) {
                        distances[neighbor] = distances[currentNode] + 1;
                        queue.enqueue(neighbor);

                        if (distances[neighbor] > distances[farthestNode]) {
                            farthestNode = neighbor;
                        }
                    }
                }
            }
            return {farthestNode, distances[farthestNode]};
        };

        auto [farthestA, _] = bfs(0);
        auto [farthestB, __] = bfs(farthestA);

        TestPoint edgeA = pointIndexMap.contains(farthestA) ? pointIndexMap[farthestA] : TestPoint{};
        TestPoint edgeB = pointIndexMap.contains(farthestB) ? pointIndexMap[farthestB] : TestPoint{};

        QList<TestPoint> singleTracePoints;
        QSet<QPair<double, double>> uniqueCoordinates;

        if (edgeA.x != 0 || edgeA.y != 0) {
            singleTracePoints.append(edgeA);
            uniqueCoordinates.insert({edgeA.x, edgeA.y});
        }
        if (edgeB.x != 0 || edgeB.y != 0) {
            singleTracePoints.append(edgeB);
            uniqueCoordinates.insert({edgeB.x, edgeB.y});
        }

        for (const auto &tp : netTestPoints) {
            QPair<double, double> coord = {tp.x, tp.y};
            if (uniqueCoordinates.contains(coord)) {
                continue;
            }

            int connectedTraces = 0;
            for (const auto &trace : traces) {
                bool isConnectedToStart = std::abs(trace.startX - tp.x) < TOLERANCE &&
                                          std::abs(trace.startY - tp.y) < TOLERANCE;
                bool isConnectedToEnd = std::abs(trace.endX - tp.x) < TOLERANCE &&
                                        std::abs(trace.endY - tp.y) < TOLERANCE;

                if (isConnectedToStart || isConnectedToEnd) {
                    ++connectedTraces;
                }
            }

            if (connectedTraces == 1) {
                singleTracePoints.append(tp);
                uniqueCoordinates.insert(coord);
            }
        }

        optimized[it.key()] = singleTracePoints;
    }

    return optimized;
}
QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>> GCodeConverter::divideTestPointsForProbes(const QMap<QString, QList<TestPoint>>& groupedTestPoints) const {
    QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>> dividedTestPoints;

    // Flatten the grouped test points
    QList<TestPoint> allTestPoints;
    for (const auto& sublist : groupedTestPoints.values()) {
        allTestPoints.append(sublist);
    }

    // Get the prioritized edge points for each net
    QMap<QString, QList<TestPoint>> prioritizedEdges = prioritizeEdgesAndSingleTracePoints(allTestPoints);

    for (auto it = groupedTestPoints.begin(); it != groupedTestPoints.end(); ++it) {
        const QList<TestPoint>& netTestPoints = it.value();
        QList<TestPoint> upperProbePoints;
        QList<TestPoint> lowerProbePoints;

        if (netTestPoints.isEmpty()) {
            continue;
        }

        // Extract the two farthest points for this net
        QList<TestPoint> edgePoints = prioritizedEdges.value(it.key());
        if (edgePoints.size() >= 2) {
            upperProbePoints.append(edgePoints[0]); // Farthest point A
            lowerProbePoints.append(edgePoints[1]); // Farthest point B
        }

        // Divide remaining test points based on Y-coordinate
        for (const TestPoint& tp : netTestPoints) {
            if (tp.x == edgePoints[0].x && tp.y == edgePoints[0].y ||
                tp.x == edgePoints[1].x && tp.y == edgePoints[1].y) {
                continue; // Skip the endpoints already assigned
            }

            if (tp.y >= (edgePoints[0].y + edgePoints[1].y) / 2) {
                upperProbePoints.append(tp);
            } else {
                lowerProbePoints.append(tp);
            }
        }

        dividedTestPoints[it.key()] = qMakePair(upperProbePoints, lowerProbePoints);
    }

    return dividedTestPoints;
}














QString GCodeConverter::generateGCode(const QMap<QString, QList<TestPoint>>& groupedTestPoints) const
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

