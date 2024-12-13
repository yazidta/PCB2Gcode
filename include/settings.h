#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
public:
    Settings();
    void setGerberLayerPath(const QString &path);
    QString getGerberLayerPath() const;

    void setDrillFilePath(const QString &path);
    QString getDrillFilePath() const;

    // Add more settings methods as needed

private:
    QString gerberLayerPath;
    QString drillFilePath;
};

#endif // SETTINGS_H
