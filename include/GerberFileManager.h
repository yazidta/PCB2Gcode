#ifndef GCODEFILEMANAGER_H
#define GCODEFILEMANAGER_H

#include <QString>

class GerberFileManager
{
public:
    GerberFileManager();
    bool loadGerberFile(const QString &filePath);
private:
    QString gerberData;

};

#endif // GCODEFILEMANAGER_H
