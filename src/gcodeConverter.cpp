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

    for (auto it = groupedTestPoints.begin(); it != groupedTestPoints.end(); ++it) {
        const QList<TestPoint>& netTestPoints = it.value();

        QList<TestPoint> upperProbePoints;
        QList<TestPoint> lowerProbePoints;

        if (netTestPoints.size() < 2) {
            continue;
        }

        // Prioritize the farthest points within the current net
        QList<TestPoint> edgePoints = prioritizeEdgesAndSingleTracePoints(netTestPoints).value(it.key());
        if (edgePoints.size() >= 2) {
            if (edgePoints[0].y > edgePoints[1].y) {
                upperProbePoints.append(edgePoints[0]);
                lowerProbePoints.append(edgePoints[1]);
            } else {
                upperProbePoints.append(edgePoints[1]);
                lowerProbePoints.append(edgePoints[0]);
            }
        }

        // Collect remaining points excluding the edge points
        QList<TestPoint> remainingPoints;
        for (const TestPoint& tp : netTestPoints) {
            if ((tp.x != edgePoints[0].x || tp.y != edgePoints[0].y) &&
                (tp.x != edgePoints[1].x || tp.y != edgePoints[1].y)) {
                remainingPoints.append(tp);
                qDebug() << tp.x << "     A      " << tp.y;
            }
        }

        // Process remaining points only if there are any
        if (!remainingPoints.isEmpty()) {
            TestPoint lastPoint = remainingPoints.last();
            int upperY = std::abs(pow((upperProbePoints.last().y - lastPoint.y), 2));
            int lowerY = std::abs(pow(lowerProbePoints.last().y - lastPoint.y, 2));
            int upperDist = std::abs(sqrt(upperY + pow((upperProbePoints.last().x - lastPoint.x), 2)));
            int lowerDist = std::abs(sqrt(lowerY + pow((lowerProbePoints.last().x - lastPoint.x), 2)));



            // Handle odd number of points: check closest probe
            if (remainingPoints.size() % 2 != 0) {
                if (upperProbePoints.last().y < lastPoint.y) {
                    upperProbePoints.append(lastPoint);
                } else if (lowerProbePoints.last().y > lastPoint.y) {
                    lowerProbePoints.append(lastPoint);
                } else if (upperProbePoints.last().y > lastPoint.y && lowerDist < upperDist) {
                    lowerProbePoints.append(lastPoint);
                } else if (upperProbePoints.last().y > lastPoint.y && lowerDist > upperDist) {
                    upperProbePoints.append(lastPoint);
                }
            }
            //Divide remaining test points
/*            for (int i = 0; i < remainingPoints.size(); ++i) {
                const TestPoint& tp = remainingPoints[i];
                if (i % 2 == 0)*/ if(remainingPoints.size() % 2 == 0) {
                for (int i = 0; i < remainingPoints.size(); ++i) {
                    const TestPoint& tp = remainingPoints[i];
                    if (upperProbePoints.last().y < tp.y) {
                        upperProbePoints.append(tp);
                    } else if (lowerProbePoints.last().y > tp.y) {
                        lowerProbePoints.append(tp);
                    } else if (upperProbePoints.last().y > tp.y && lowerDist < upperDist) {
                        lowerProbePoints.append(tp);
                    } else if (upperProbePoints.last().y > tp.y && lowerDist > upperDist) {
                        upperProbePoints.append(tp);
                    }
                }
            }
        }

        dividedTestPoints[it.key()] = qMakePair(upperProbePoints, lowerProbePoints);
    }

    return dividedTestPoints;
}















QString GCodeConverter::generateGCode(const QMap<QString, QList<TestPoint>>& groupedTestPoints) const {
    QString gCode;
    gCode += "G21 ; Set units to millimeters\n";
    gCode += "G90 ; Absolute positioning\n";

    // Divide test points between upper and lower probes
    QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>> dividedTestPoints = divideTestPointsForProbes(groupedTestPoints);

    for (auto it = dividedTestPoints.begin(); it != dividedTestPoints.end(); ++it) {
        const QString& net = it.key();
        const QList<TestPoint>& lowerProbePoints = it.value().second;
        const QList<TestPoint>& upperProbePoints = it.value().first;

        gCode += QString("; Net: %1\n").arg(net);

        // Alternate between lower and upper probe movements
        int lowerIndex = 0;
        int upperIndex = 0;
        int maxPoints = std::max(lowerProbePoints.size(), upperProbePoints.size());

        for (int i = 0; i < maxPoints; ++i) {
            if (lowerIndex < lowerProbePoints.size()) {
                const TestPoint& tp = lowerProbePoints[lowerIndex++];
                gCode += QString("G01 X%1 Y%2 ; Move to test point (lower probe)\n").arg(tp.x).arg(tp.y);
                gCode += "G1 Z-1 ; Lower probe\n";
                gCode += "G1 Z1 ; Raise probe\n";
                //gCode += "G01 ; Lower probe movement\n";
            }

            if (upperIndex < upperProbePoints.size()) {
                const TestPoint& tp = upperProbePoints[upperIndex++];
                gCode += QString("G02 X%1 Y%2 ; Move to test point (upper probe)\n").arg(tp.x).arg(tp.y);
                gCode += "G1 Z-1 ; Lower probe\n";
                gCode += "G1 Z1 ; Raise probe\n";
               // gCode += "G02 ; Upper probe movement\n";
            }
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

