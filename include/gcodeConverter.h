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
    int numTraces;
};

struct PadInfo {
    double x;
    double y;
    std::string aperture;
    QString net;
};
struct TraceInfo {
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    std::string aperture;
};

class GerberManager;

class GCodeConverter
{
public:
    explicit GCodeConverter(GerberManager* gerberManager);

    bool loadCSVFile(const QString &filePath);
    QList<TestPoint> filterTopSidePoints() const;
    QMap<QString, QList<TestPoint>> groupByNet(const QList<TestPoint> &testPoints) const;
    QString generateGCodeFromCSV(const QMap<QString, QList<TestPoint>> &groupedTestPoints) const;
    std::vector<int> countTracesConnectedToPads() const;
    bool saveGCodeToFile(const QString &filePath, const QString &gCodeContent);
    bool extractPadAndTraceCoords();
    QMap<QString, QList<TestPoint>>DeletePoints(const QList<TestPoint> &testPoints) const;

    QString generateGcodeFromGerber();

private:
    QList<TestPoint> testPoints;
    std::vector<PadInfo> padCoords;
    GerberManager* gerberManager;
    std::vector<TraceInfo> TraceCoords;
};

#endif // GCODECONVERTER_H
