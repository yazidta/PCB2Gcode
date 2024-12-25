#include "include/pcb2gcode.h"
#include "ui_PCB2Gcode.h"

#include <QProgressDialog>
#include <QKeyEvent>


PCB2Gcode::PCB2Gcode(QWidget *parent) : QMainWindow(parent), ui(new Ui::PCB2Gcode), uart(new UART(this)), gerberManager()
{
    ui->setupUi(this);

    initUART();
    connectSignals();

}

PCB2Gcode::~PCB2Gcode()
{
    delete ui;
}

void PCB2Gcode::connectSignals()
{

    connect(ui->generateButton, &QPushButton::clicked, this, &PCB2Gcode::onGenerate);

    connect(ui->previewButton, &QPushButton::clicked, this, &PCB2Gcode::onPreview);

    connect(ui->browseButtonCopper, &QPushButton::clicked, this, &PCB2Gcode::onBrowseCopper);

    connect(ui->browseButtonMask, &QPushButton::clicked, this, &PCB2Gcode::onBrowseMask);

    connect(ui->browseButtonSilk, &QPushButton::clicked, this, &PCB2Gcode::onBrowseSilk);

    connect(ui->browseButtonBoard, &QPushButton::clicked, this, &PCB2Gcode::onBrowseBoard);

    connect(ui->zoomInButton, &QPushButton::clicked, this, &PCB2Gcode::onZoomIn);

    connect(ui->zoomOutButton, &QPushButton::clicked, this, &PCB2Gcode::onZoomOut);

    connect(ui->zoomOriginalButton, &QPushButton::clicked, this, &PCB2Gcode::onZoomOriginal);

    connect(ui->dragButton, &QPushButton::clicked, this, &PCB2Gcode::onDrag);

}


void PCB2Gcode::onBrowseTestPoints()
{
    QString TestPointsFile = QFileDialog::getOpenFileName(this, tr("Select Test Points File"));
    if (!TestPointsFile.isEmpty()) {
        ui->testPointsPath->setText(TestPointsFile);
    }
    else{
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invaild file Path, seems to be empty.");
    }
}

void PCB2Gcode::onBrowseCopper()
{
    QString copperFile = QFileDialog::getOpenFileName(this, tr("Select Copper Layer Gerber File"));
    if (!copperFile.isEmpty()) {
        ui->copperLayer->setText(copperFile);
    }
    else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }

}
void PCB2Gcode::onBrowseMask(){
    QString maskFile = QFileDialog::getOpenFileName(this, tr("Select Solder Mask Gerber File"));
    if (!maskFile.isEmpty()) {
        ui->maskPath->setText(maskFile);
    }
    else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }
}

void PCB2Gcode::onBrowseBoard()
{
    QString boardFile = QFileDialog::getOpenFileName(this, tr("Select Board Edge Cuts Gerber File"));
    if (!boardFile.isEmpty()) {
        ui->boardPath->setText(boardFile);
    } else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }
}

void PCB2Gcode::onBrowseSilk()
{
    QString silkFile = QFileDialog::getOpenFileName(this, tr("Select Silkscreen Gerber File"));
    if (!silkFile.isEmpty()) {
        ui->silkPath->setText(silkFile);
    } else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }

}

void PCB2Gcode::onPreview()
{
    // Clear any previously loaded files in GerberManager
    gerberManager.clearGerberFiles();

    QString copperFile = ui->copperLayer->text();
    QString maskFile = ui->maskPath->text();
    QString silkFile = ui->silkPath->text();
    QString boardFile = ui->boardPath->text();

    if (copperFile.isEmpty() || maskFile.isEmpty() || silkFile.isEmpty() || boardFile.isEmpty()) {
        QMessageBox::warning(this, tr("Incomplete Selection"),
                             tr("Please select all Gerber files: Copper Layer, Mask, Silkscreen, and Board Edge Cuts."));
        return;
    }

    QStringList filePaths = { copperFile, maskFile, silkFile, boardFile };

    // Load Gerber files
    if (!gerberManager.loadGerberFiles(filePaths)) {
        QMessageBox::critical(this, tr("Loading Failed"),
                              tr("An error occurred while loading the Gerber files. Please check the logs for details."));
        return;
    }

    // Show progress dialog
    QProgressDialog progressDialog(tr("Rendering Gerber files..."), tr("Cancel"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    // Define output path for rendered image
    QString outputImagePath = QFileDialog::getSaveFileName(this, tr("Save Rendered Image"), "", tr("PNG Images (*.png)"));
    if (outputImagePath.isEmpty()) {
        QMessageBox::information(this, tr("Cancelled"), tr("Rendering cancelled by user."));
        return;
    }

    // Render Gerber files
    QPixmap renderedImage = gerberManager.renderGerber(outputImagePath, 40);

    progressDialog.close();

    if (renderedImage.isNull()) {
        QMessageBox::critical(this, tr("Rendering Failed"),
                              tr("An error occurred during the rendering process. Please check the logs for details."));
        return;
    }

    // Display the rendered image in the UI
    enableToolBar();
    ui->graphicsViewPreview->setScene(new QGraphicsScene(this));
    ui->graphicsViewPreview->scene()->addPixmap(renderedImage);
    ui->graphicsViewPreview->fitInView(ui->graphicsViewPreview->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);

}

void PCB2Gcode::onGenerate(){
    QString testPointsFile = ui->testPointsPath->text();
    if (testPointsFile.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Please select a test points file."));
        return;
    }


    if (!gcodeConverter.loadCSVFile(testPointsFile)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load test points file. Please check the file format."));
        return;
    }

    // Filter and group test points
    QList<TestPoint> topSidePoints = gcodeConverter.filterTopSidePoints();
    if (topSidePoints.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("No top-side test points found in the file."));
        return;
    }

    QMap<QString, QList<TestPoint>> groupedPoints = gcodeConverter.groupByNet(topSidePoints);

    QString gCode = gcodeConverter.generateGCode(groupedPoints);


    QString savePath = QFileDialog::getSaveFileName(this, tr("Save G-Code File"), "", tr("G-Code Files (*.gcode)"));
    if (savePath.isEmpty()) {
        return;
    }

    if (!gcodeConverter.saveGCodeToFile(savePath, gCode)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save G-code file."));
    } else {
        QMessageBox::information(this, tr("Success"), tr("G-code generated and saved successfully."));
    }
}

void PCB2Gcode::enableToolBar(){
    ui->zoomInButton->isEnabled();
    ui->zoomOutButton->isEnabled();
    ui->zoomOriginalButton->isEnabled();
    ui->dragButton->isEnabled();
    ui->saveImageButton->isEnabled();
}

void PCB2Gcode::onZoomIn(){
    ui->graphicsViewPreview->scale(1.1, 1.1);

}
void PCB2Gcode::onZoomOut(){
    ui->graphicsViewPreview->scale(1/1.1, 1/1.1);

}
void PCB2Gcode::onZoomOriginal(){
    ui->graphicsViewPreview->resetTransform();
    if (ui->graphicsViewPreview->scene()) {
        ui->graphicsViewPreview->fitInView(ui->graphicsViewPreview->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}

void PCB2Gcode::onDrag(){
    if (isDragEnabled){
        ui->graphicsViewPreview->setDragMode(QGraphicsView::NoDrag);
        isDragEnabled = false;
    }
    else{
        ui ->graphicsViewPreview->setDragMode(QGraphicsView::ScrollHandDrag);
        isDragEnabled = true;
    }
}

void PCB2Gcode::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Escape && isDragEnabled){
        ui->graphicsViewPreview->setDragMode(QGraphicsView::NoDrag);
        isDragEnabled = false;
    }
    else{
        return;
    }
    QWidget::keyPressEvent(event);
}

void PCB2Gcode::onSaveImage(){

}


void PCB2Gcode::initUART()
{
    connect(uart, &UART::connectionStatusChanged, this, [=](bool connected) {
        if (connected)
            QMessageBox::information(this, "Connection", "Connected successfully.");
        else
            QMessageBox::critical(this, "Connection", "Failed to connect or disconnected.");
    });

    connect(ui->refreshButton, &QPushButton::clicked, this, [=]() {
        ui->comPortcomboBox->clear();
        ui->comPortcomboBox->addItems(uart->availablePorts());
    });

    connect(ui->connectionButton, &QPushButton::clicked, this, [=]() {
        if (uart->isConnected()) {
            uart->disconnectPort();
            ui->connectionButton->setText("Connect");
        } else {
            QString portName = ui->comPortcomboBox->currentText();
            if (uart->connectPort(portName))
                ui->connectionButton->setText("Disconnect");
        }
    });

}



