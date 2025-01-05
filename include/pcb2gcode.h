#ifndef PCB2GCODE_H
#define PCB2GCODE_H

#include "include/gcodeConverter.h"
#include "include/gerbermanager.h"
#include "include/settings.h"
#include "include/uart.h"

#include <QWidget>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QMainWindow>
#include <QTabWidget>
#include <QObject>
#include <QFileDialog>



QT_BEGIN_NAMESPACE
namespace Ui {
class PCB2Gcode;
}
QT_END_NAMESPACE

class PCB2Gcode : public QMainWindow
{
    Q_OBJECT

public:
    explicit PCB2Gcode(QWidget *parent = nullptr);
    ~PCB2Gcode();

private Q_SLOTS:
    // Gcode Converter Tab
    void onBrowseTestPoints();
    void onBrowseCopper();
    void onBrowseMask();
    void onBrowseSilk();
    void onBrowseBoard();
    void onGenerateFromGerber();
    void onGenerateFromCSV();
    void onPreview();
    void onPreviewTestPoints();
    void onZoomIn();
    void onZoomOut();
    void onZoomOriginal();
    void onDrag();
    void onSaveImage();
    void enableToolBar();
    void keyPressEvent(QKeyEvent *event);


private:
    Ui::PCB2Gcode *ui;
    Settings appSettings;
    UART *uart;
    QTabWidget* tabWidget;
    GerberManager *gerberManager;
    GCodeConverter *gcodeConverter;
    QPixmap  saveImageRendered;

    void connectSignals();
    void initUART();
    bool isImageRendered = false;
    bool isDragEnabled = false;


};

#endif // PCB2GCODE_H
