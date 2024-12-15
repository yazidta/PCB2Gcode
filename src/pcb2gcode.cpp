#include "include/pcb2gcode.h"
#include "ui_PCB2Gcode.h"
#include <QMessageBox>
#include <QApplication>

PCB2Gcode::PCB2Gcode(QWidget *parent) : QMainWindow(parent), ui(new Ui::PCB2Gcode), uart(new UART(this))
{
    ui->setupUi(this);

    initUART();
    connectSignals();


}

void PCB2Gcode::setupFirstTab()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(ui->testPointsPath);
    mainLayout->addWidget(ui->testPointsButton);

    mainLayout->addWidget(ui->firstLayer);
    mainLayout->addWidget(ui->browseButtonLayer);

    mainLayout->addWidget(ui->drillPath);
    mainLayout->addWidget(ui->browseButtonDrill);

    mainLayout->addWidget(ui->probeDiameter);
    mainLayout->addWidget(ui->boardOffset);
    mainLayout->addWidget(ui->showEdgesCheck);


    mainLayout->addWidget(ui->generateButton);
    mainLayout->addWidget(ui->previewButton);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void PCB2Gcode::connectSignals()
{

    connect(ui->generateButton, &QPushButton::clicked, this, &PCB2Gcode::onGenerate);

    connect(ui->previewButton, &QPushButton::clicked, this, &PCB2Gcode::onPreview);

    connect(ui->browseButtonLayer, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonLayerClicked);

    connect(ui->browseButtonDrill, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonDrillClicked);

    connect(ui->testPointsButton, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonTestPointsClicked);
}

PCB2Gcode::~PCB2Gcode()
{
    delete ui;
}

void PCB2Gcode::onBrowseButtonTestPointsClicked()
{
    QString TestPointsFile = QFileDialog::getOpenFileName(this, tr("Select Test Points File"));
    if (!TestPointsFile.isEmpty()) {
        ui->testPointsPath->setText(TestPointsFile);
    }
    else{
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invaild file Path, seems to be empty.");
    }
}

void PCB2Gcode::onBrowseButtonLayerClicked()
{
    QString layerFile = QFileDialog::getOpenFileName(this, tr("Select Gerber Layer File"));
    if (!layerFile.isEmpty()) {
        ui->firstLayer->setText(layerFile);
    }
}


void PCB2Gcode::onBrowseButtonDrillClicked()
{
    QString drillFile = QFileDialog::getOpenFileName(this, tr("Select Drill File"));
    if (!drillFile.isEmpty()) {
        ui->drillPath->setText(drillFile);
    }
}

void PCB2Gcode::onGenerate()
{
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

void PCB2Gcode::onPreview()
{
    QMessageBox::information(this, tr("Preview"), tr("Preview functionality not yet implemented."));
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



