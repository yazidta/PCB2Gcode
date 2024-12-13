#include "include/pcb2gcode.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <QApplication>
#include "include/pcb2gcode.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PCB2Gcode w;
    w.show();
    return a.exec();
}


