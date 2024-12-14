#ifndef GCODEFILEMANAGER_H
#define GCODEFILEMANAGER_H

#include <QString>

class GerberFileManager
{
public:
    GerberFileManager();
    bool loadGerberFile(const QString &filePath);
    bool saveGCodeFile(const QString &filePath, const QString &gCodeContent);
    QStringList extractTestPoints() const;
    QString generateGCodeForTestPoints(const QStringList &testPoints) const;
private:
    QString gerberData;

};

#endif // GCODEFILEMANAGER_H
