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
                    lowerProbePoints.append(lowerProbePoints.last());
                } else if (lowerProbePoints.last().y > lastPoint.y) {
                    lowerProbePoints.append(lastPoint);
                    upperProbePoints.append(upperProbePoints.last());
                } else if (upperProbePoints.last().y > lastPoint.y && lowerDist < upperDist) {
                    lowerProbePoints.append(lastPoint);
                    upperProbePoints.append(upperProbePoints.last());
                } else if (upperProbePoints.last().y > lastPoint.y && lowerDist > upperDist) {
                    upperProbePoints.append(lastPoint);
                    lowerProbePoints.append(lowerProbePoints.last());
                }
            }
            //Divide remaining test points
            if(remainingPoints.size() % 2 == 0) {
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
int GCodeConverter::calculateTotalDistance(const TestPoint& lowerProbePosition,
                                           const TestPoint& upperProbePosition,
                                           const QList<TestPoint>& netTestPoints) const {
    if (netTestPoints.size() < 2) {
        return std::numeric_limits<int>::max(); // Ignore nets with fewer than 2 points
    }

    int distanceLower = std::abs(lowerProbePosition.x - netTestPoints[0].x) +
                        std::abs(lowerProbePosition.y - netTestPoints[0].y);
    int distanceUpper = std::abs(upperProbePosition.x - netTestPoints[1].x) +
                        std::abs(upperProbePosition.y - netTestPoints[1].y);

    return distanceLower + distanceUpper; // Sum of distances
}


QString GCodeConverter::selectNextNet(
    const QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>>& dividedTestPoints,
    const QList<QString>& remainingNets,
    const QPointF& lowerProbePos,
    const QPointF& upperProbePos) const
{
    QString selectedNet;
    qreal minTotalDistance = std::numeric_limits<qreal>::max();

    for (const QString &net : remainingNets) {
        // Retrieve the lists for each probe.
        // (Here, the first list is for the upper probe and the second is for the lower probe.)
        const QList<TestPoint>& upperProbePoints = dividedTestPoints[net].first;
        const QList<TestPoint>& lowerProbePoints = dividedTestPoints[net].second;

        // Skip nets that do not have at least one test point per probe.
        if (upperProbePoints.isEmpty() || lowerProbePoints.isEmpty()) {
            continue;
        }

        // Calculate distances from current probe positions to the first test points for the net.
        qreal upperDist = qHypot(upperProbePoints.first().x - upperProbePos.x(),
                                 upperProbePoints.first().y - upperProbePos.y());
        qreal lowerDist = qHypot(lowerProbePoints.first().x - lowerProbePos.x(),
                                 lowerProbePoints.first().y - lowerProbePos.y());
        qreal totalDist = upperDist + lowerDist;

        // Select the net with the least total movement cost.
        if (totalDist < minTotalDistance) {
            minTotalDistance = totalDist;
            selectedNet = net;
        }
    }

    return selectedNet;
}

QString GCodeConverter::generateGCode(const QMap<QString, QList<TestPoint>>& groupedTestPoints) const
{
    // Calculate PCB dimensions from the Gerber data.
    double pcbbWidthMM = gerberManager->maxX - gerberManager->minX;
    double pcbHeightMM = gerberManager->maxY - gerberManager->minY;
    QString gCode;

    // Start by outputting a header comment with PCB dimensions.
    gCode += QString("; G54 X%1 Y%2\n\n\n\n").arg(pcbbWidthMM).arg(pcbHeightMM);

    // Divide the test points for each net between the upper and lower probes.
    // (Assumes that divideTestPointsForProbes() returns a QMap where the key is the net name and
    //  the value is a QPair: [upperProbePoints, lowerProbePoints].)
    QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>> dividedTestPoints = divideTestPointsForProbes(groupedTestPoints);

    // Prepare a list of nets that still need to be processed.
    QList<QString> remainingNets = dividedTestPoints.keys();

    // Initialize current probe positions. (These might be updated after processing each net.)
    QPointF lowerProbePos(0, 0);
    QPointF upperProbePos(0, 0);

    // Process nets until there are none left.
    while (!remainingNets.isEmpty()) {
        // Use our helper function to choose the next net based on current probe positions.
        QString nextNet = selectNextNet(dividedTestPoints, remainingNets, lowerProbePos, upperProbePos);
        if (nextNet.isEmpty()) {
            break;  // No valid nets remaining.
        }

        // Output a comment for the selected net.
        gCode += QString("; Net: %1\n\n\n\n").arg(nextNet);

        // Retrieve the test point lists for the selected net.
        // (upperProbePoints = dividedTestPoints[nextNet].first,
        //  lowerProbePoints = dividedTestPoints[nextNet].second)
        const QPair<QList<TestPoint>, QList<TestPoint>>& probePoints = dividedTestPoints[nextNet];

        // For each pair of test points, generate the G‑code commands.
        // Here we assume that the lower probe commands (P1) come from probePoints.second and
        // the upper probe commands (P2) come from probePoints.first.
        int lowerIndex = 0;
        int upperIndex = 0;
        while (lowerIndex < probePoints.second.size() || upperIndex < probePoints.first.size()) {
            // If there is a lower probe point left, print its move.
            if (lowerIndex < probePoints.second.size()) {
                gCode += QString("G0 P1 X%1 Y%2;\n\n\n\n")
                .arg(probePoints.second[lowerIndex].x)
                    .arg(probePoints.second[lowerIndex].y);
                lowerIndex++;
            }
            // If there is an upper probe point left, print its move.
            if (upperIndex < probePoints.first.size()) {
                gCode += QString("G0 P2 X%1 Y%2;\n\n\n\n")
                .arg(probePoints.first[upperIndex].x)
                    .arg(probePoints.first[upperIndex].y);
                upperIndex++;
            }

            // Execute the test command and retract both probes.
            gCode += "T1 ;\n\n\n\n";
        }

        // Update the current probe positions using the last test point of the processed net.
        if (!probePoints.second.isEmpty()) {
            lowerProbePos = QPointF(probePoints.second.last().x, probePoints.second.last().y);
        }
        if (!probePoints.first.isEmpty()) {
            upperProbePos = QPointF(probePoints.first.last().x, probePoints.first.last().y);
        }

        // Remove the processed net from the remaining list.
        remainingNets.removeOne(nextNet);
    }

    // End of program command.
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

