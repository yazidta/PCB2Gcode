#ifndef PCB2GCODE_H
#define PCB2GCODE_H

#include <QWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QMainWindow>
#include <QTabWidget>
#include "include/gcodeConverter.h"
#include "include/GerberFileManager.h"
#include "include/settings.h"
#include "include/uart.h"

namespace Ui {
class PCB2Gcode;
}

class PCB2Gcode : public QMainWindow
{
    Q_OBJECT

public:
    explicit PCB2Gcode(QWidget *parent = nullptr);
    ~PCB2Gcode();

private slots:
    // Gcode Converter Tab
    void onBrowseButtonTestPointsClicked();
    void onBrowseButtonLayerClicked();
    void onBrowseButtonDrillClicked();
    void onGenerate();
    void onPreview();



private:
    Ui::PCB2Gcode *ui;
    GerberFileManager gerberManager;
    GCodeConverter gcodeConverter;
    Settings appSettings;
    UART *uart;

    QTabWidget* tabWidget;
    void connectSignals();
    void setupFirstTab();
    void initUART();
};

#endif // PCB2GCODE_H
