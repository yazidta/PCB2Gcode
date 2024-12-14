#include "include/pcb2gcode.h"
#include "ui_PCB2Gcode.h"
#include <QMessageBox>
#include <QApplication>

PCB2Gcode::PCB2Gcode(QWidget *parent) : QMainWindow(parent), ui(new Ui::PCB2Gcode)
{
    ui->setupUi(this);

    connectSignals();


}

void PCB2Gcode::setupFirstTab()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(ui->firstLayer);
    mainLayout->addWidget(ui->browseButtonLayer);
    mainLayout->addWidget(ui->drillPath);
    mainLayout->addWidget(ui->firstMirror);
    mainLayout->addWidget(ui->probeDiameter);
    mainLayout->addWidget(ui->boardOffset);
    mainLayout->addWidget(ui->showEdgesCheck);


    mainLayout->addWidget(ui->browseButtonDrill);
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
}

PCB2Gcode::~PCB2Gcode()
{
    delete ui;
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
    QStringList pairs = gerberManager.extractTestPoints();
    QString gCode = gerberManager.generateGCodeForTestPoints(pairs);

    QFile gcodeFile("test_program.gcode");
    if (gcodeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&gcodeFile);
        out << gCode;
        gcodeFile.close();
    }

    QMessageBox::information(this, tr("G-code Generation"), tr("G-code generated successfully."));
   // QMessageBox::information(this, tr("Generate"), tr("G-code generation not implemented yet."));
}

void PCB2Gcode::onPreview()
{
    QMessageBox::information(this, tr("Preview"), tr("Preview functionality not yet implemented."));
}

