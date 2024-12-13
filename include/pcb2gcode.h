#ifndef PCB2GCODE_H
#define PCB2GCODE_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class PCB2Gcode;
}
QT_END_NAMESPACE

class PCB2Gcode : public QMainWindow
{
    Q_OBJECT

public:
    PCB2Gcode(QWidget *parent = nullptr);
    ~PCB2Gcode();

private:
    Ui::PCB2Gcode *ui;
};
#endif // PCB2GCODE_H
