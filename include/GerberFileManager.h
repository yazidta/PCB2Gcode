#ifndef GCODEFILEMANAGER_H
#define GCODEFILEMANAGER_H

#include <QString>

class GCodeFileManager
{
public:
    GCodeFileManager();
    bool loadGCodeFile(const QString &filePath);
    bool saveGCodeFile(const QString &filePath, const QString &gCodeContent);

private:
    QString gCodeData;  // Stores the loaded G-code data
};

#endif // GCODEFILEMANAGER_H
