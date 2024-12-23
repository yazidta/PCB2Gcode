#include "include/pcb2gcode.h"
#include <QApplication>


int main(int argc, char *argv[]) {



    QApplication a(argc, argv);
    PCB2Gcode w;
    w.show();
    return a.exec();



    return 0;
}




