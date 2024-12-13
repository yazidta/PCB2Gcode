#ifndef PCB2GCODE_H
#define PCB2GCODE_H

#include <QWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "include/GerberFileManager.h"
#include "include/settings.h"

namespace Ui {
class PCB2Gcode;
}

class PCB2Gcode : public QWidget
{
    Q_OBJECT

public:
    explicit PCB2Gcode(QWidget *parent = nullptr);
    ~PCB2Gcode();

private slots:
    void onBrowseButtonLayerClicked();
    void onBrowseButtonDrillClicked();
    void onGenerate();
    void onPreview();

private:
    Ui::PCB2Gcode *ui;
    GCodeFileManager gcodeManager;
    Settings appSettings;
};

#endif // PCB2GCODE_H
