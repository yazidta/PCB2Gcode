#include "include/pcb2gcode.h"
#include "ui_PCB2Gcode.h"

#include <QProgressDialog>



PCB2Gcode::PCB2Gcode(QWidget *parent) : QMainWindow(parent), ui(new Ui::PCB2Gcode), uart(new UART(this)), gerberManager()
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

    mainLayout->addWidget(ui->silkPath);
    mainLayout->addWidget(ui->browseButtonSilk);

    mainLayout->addWidget(ui->boardPath);
    mainLayout->addWidget(ui->browseButtonBoard);

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

    connect(ui->browseButtonMask, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonMaskClicked);

    connect(ui->browseButtonSilk, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonSilkClicked);

    connect(ui->browseButtonBoard, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonBoardClicked);

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
    QString layerFile = QFileDialog::getOpenFileName(this, tr("Select Copper Layer Gerber File"));
    if (!layerFile.isEmpty()) {
        ui->firstLayer->setText(layerFile);
    }
    else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }

}
void PCB2Gcode::onBrowseButtonMaskClicked(){
    QString maskFile = QFileDialog::getOpenFileName(this, tr("Select Solder Mask Gerber File"));
    if (!maskFile.isEmpty()) {
        ui->maskPath->setText(maskFile);
    }
    else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }
}

void PCB2Gcode::onBrowseButtonBoardClicked()
{
    QString boardFile = QFileDialog::getOpenFileName(this, tr("Select Board Edge Cuts Gerber File"));
    if (!boardFile.isEmpty()) {
        ui->boardPath->setText(boardFile);
    } else {
        QMessageBox::critical(this, "ERROR: FILE PATH", "Invalid file path. It seems to be empty.");
    }
}

void PCB2Gcode::onBrowseButtonSilkClicked()
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

    QString layerFile = ui->firstLayer->text();
    QString maskFile = ui->maskPath->text();
    QString silkFile = ui->silkPath->text();
    QString boardFile = ui->boardPath->text();

    if (layerFile.isEmpty() || maskFile.isEmpty() || silkFile.isEmpty() || boardFile.isEmpty()) {
        QMessageBox::warning(this, tr("Incomplete Selection"),
                             tr("Please select all Gerber files: Copper Layer, Mask, Silkscreen, and Board Edge Cuts."));
        return;
    }

    QStringList filePaths = { layerFile, maskFile, silkFile, boardFile };

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



