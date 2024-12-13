#include "pcb2gcode.h"
#include "./ui_PCB2Gcode.h"

PCB2Gcode::PCB2Gcode(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FPTApplication)
{
    ui->setupUi(this);
}

PCB2Gcode::~PCB2Gcode()
{
    delete ui;
}
