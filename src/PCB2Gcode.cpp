#include "include/pcb2gcode.h"
#include "ui_pcb2gcode.h"
#include <QMessageBox>

PCB2Gcode::PCB2Gcode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PCB2Gcode)
{
    ui->setupUi(this);

    // Connect buttons to their respective slots
    connect(ui->browseButtonLayer, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonLayerClicked);
    connect(ui->browseButtonDrill, &QPushButton::clicked, this, &PCB2Gcode::onBrowseButtonDrillClicked);
    connect(ui->generateButton, &QPushButton::clicked, this, &PCB2Gcode::onGenerate);
    connect(ui->previewButton, &QPushButton::clicked, this, &PCB2Gcode::onPreview);
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
        appSettings.setGerberLayerPath(layerFile);  // Store the layer file path in settings
    }
}

void PCB2Gcode::onBrowseButtonDrillClicked()
{
    QString drillFile = QFileDialog::getOpenFileName(this, tr("Select Drill File"));
    if (!drillFile.isEmpty()) {
        ui->drillPath->setText(drillFile);
        appSettings.setDrillFilePath(drillFile);  // Store the drill file path in settings
    }
}

void PCB2Gcode::onGenerate()
{
    // Placeholder for generation logic (e.g., G-code generation)
    QString gCodeContent = "Generated G-code";  // Placeholder content
    if (gcodeManager.saveGCodeFile("output.gcode", gCodeContent)) {
        QMessageBox::information(this, tr("Generate"), tr("G-code generated successfully."));
    } else {
        QMessageBox::warning(this, tr("Generate"), tr("Failed to generate G-code."));
    }
}

void PCB2Gcode::onPreview()
{
    // Placeholder for preview logic
    QMessageBox::information(this, tr("Preview"), tr("Preview functionality not yet implemented."));
}
