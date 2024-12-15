#ifndef GCODECONVERTER_H
#define GCODECONVERTER_H

#include <QString>
#include <QList>
#include <QMap>

struct TestPoint {
    QString sourceRefDes;
    QString sourcePad;
    QString net;
    QString netClass;
    QString side;
    double x;
    double y;
    QString padType;
    QString footprintSide;
};

class GCodeConverter
{
public:
    GCodeConverter();

    bool loadCSVFile(const QString &filePath);
    QList<TestPoint> filterTopSidePoints() const;
    QMap<QString, QList<TestPoint>> groupByNet(const QList<TestPoint> &testPoints) const;
    QString generateGCode(const QMap<QString, QList<TestPoint>> &groupedTestPoints) const;
    bool saveGCodeToFile(const QString &filePath, const QString &gCodeContent);

private:
    QList<TestPoint> testPoints;
};

#endif // GCODECONVERTER_H
