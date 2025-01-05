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

struct PadInfo {
    double x;
    double y;
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
    bool saveGCodeToFile(const QString &filePath, const QString &gCodeContent);
    bool extractPadCoords();
    QString generateGcodeFromGerber();

private:
    QList<TestPoint> testPoints;
    std::vector<PadInfo> padCoords;
    GerberManager* gerberManager;
};

#endif // GCODECONVERTER_H
