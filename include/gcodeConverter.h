#ifndef GCODECONVERTER_H
#define GCODECONVERTER_H


#include <QString>
#include <QList>
#include <QMap>
#include <cmath>
#include <QPointF>


struct TestPoint{
    double x;
    double y;
    QString source;
    QString type;
    QString net;
    int pin;

};

struct Trace{
    double startX;
    double startY;
    double endX;
    double endY;
    QString net;

};


class GerberManager;

class GCodeConverter
{
public:
    explicit GCodeConverter(GerberManager* gerberManager);

    bool loadCSVFile(const QString &filePath);
    bool saveGCodeToFile(const QString &filePath, const QString &gCodeContent);
    QList<TestPoint> getTestPointsCSV();
    QList<TestPoint> getTestPointsGerber();
    QMap<QString, QList<TestPoint>> groupByNet(const QList<TestPoint> &testPoints) const;
    QString generateGCode(const QMap<QString, QList<TestPoint>> &groupedTestPoints) const;
    QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>>divideTestPointsForProbes(const QMap<QString, QList<TestPoint>>& groupedTestPoints) const;
    QMap<QString, QList<TestPoint>>prioritizeEdgesAndSingleTracePoints(const QList<TestPoint> &testPoints) const;
    int calculateTotalDistance(const TestPoint& lowerProbePosition,
                               const TestPoint& upperProbePosition,
                               const QList<TestPoint>& netTestPoints) const;
    QString selectNextNet( const QMap<QString, QPair<QList<TestPoint>, QList<TestPoint>>>& dividedTestPoints,
                           const QList<QString>& remainingNets,
                           const QPointF& lowerProbePos,
                           const QPointF& upperProbePos) const;


private:
    QList<TestPoint> testPointsCSV;
    QList<TestPoint> testPointsGerber;
    QList<Trace> Traces;
    GerberManager* gerberManager;
};

#endif // GCODECONVERTER_H
