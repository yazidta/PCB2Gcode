#include "include/settings.h"

Settings::Settings()
    : gerberLayerPath(""), drillFilePath("")
{
}

void Settings::setGerberLayerPath(const QString &path)
{
    gerberLayerPath = path;
}

QString Settings::getGerberLayerPath() const
{
    return gerberLayerPath;
}

void Settings::setDrillFilePath(const QString &path)
{
    drillFilePath = path;
}

QString Settings::getDrillFilePath() const
{
    return drillFilePath;
}
