#include "include/gerbermanager.h"
#include <QDebug>
#include <QFile>
#include <QPainter>
namespace py = pybind11;


GerberManager::GerberManager() {
    try {
        py::initialize_interpreter();
        gcodeConverter = new GCodeConverter(this);
        PyGILState_STATE gstate = PyGILState_Ensure();

        py::module_ sys = py::module_::import("sys");

        // Log the Python executable being used
        qDebug() << "Python executable:" << QString::fromStdString(sys.attr("executable").cast<std::string>());

        // Set the PYTHONPATH environment in Python
        py::list path = sys.attr("path");
        path.append("D:/github/flyingrobots/PCB2Gcode/python"); // 'gerber_wrapper.py' directory
        path.append("C:/Users/yazed/anaconda3/envs/pcb2gcode_env/Lib/site-packages"); // 'gerber_wrapper.py' directory

        // Log modified Python path
        qDebug() << "Python path after appending:" << QString::fromStdString(py::str(path).cast<std::string>());

        // Import the custom gerber_wrapper module
        py::module_ gerberWrapper = py::module_::import("gerber_wrapper");

        // Instantiate the GerberWrapper class
        gerberStack  = gerberWrapper.attr("GerberWrapper")();

        PyGILState_Release(gstate);
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python initialization error:" << QString::fromStdString(e.what());
    }
}

GerberManager::~GerberManager() {
    try {
        // Remove the temporary file if it exists
        QFile::remove(QString::fromStdString(tempImagePath));
        delete gcodeConverter;
        py::finalize_interpreter();
    }
    catch (...) {
        qDebug() << "Error finalizing Python interpreter.";
    }
}

bool GerberManager::loadGerberFiles(const QStringList& filePaths) {
    try {
        py::gil_scoped_acquire acquire;
        py::list py_file_paths;
        for (const QString& path : filePaths) {
            py_file_paths.append(path.toStdString());
        }
        gerberStack.attr("loadGerberFiles")(py_file_paths);
        qDebug() << "Gerber files loaded successfully.";
        return true;
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error while loading Gerber files:" << QString::fromStdString(e.what());
        return false;
    }
}

void GerberManager::clearGerberFiles() {
    try {
        gerberStack.attr("clearGerberFiles")();
        qDebug() << "Cleared all loaded Gerber files.";
    } catch (const py::error_already_set& e) {
        qDebug() << "Python error while clearing Gerber files:" << QString::fromStdString(e.what());
    }
}

QPixmap GerberManager::renderGerber(int dpmm) {
    try {
        py::gil_scoped_acquire acquire;

        py::object pyRenderToPng = gerberStack.attr("renderToPng");
        std::string tempFilePath = pyRenderToPng(dpmm).cast<std::string>();

        if (tempFilePath.empty()) {
            qDebug() << "Failed to render the image, temporary file path is empty.";
            return QPixmap();
        }

        QPixmap pixmap;

        if (pixmap.load(QString::fromStdString(tempFilePath))){
            qDebug() << "Successfully loaded rendered PNG into QPixmap.";
            QFile::remove(QString::fromStdString(tempFilePath));
            return pixmap;
        }
        else{
            qDebug() << "Failed to laod rendered PNG into QPixmap.";
            return QPixmap();
        }

    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error during rendering:" << QString::fromStdString(e.what());
        return QPixmap();
    }
}

QList<TestPoint> GerberManager::loadTestPoints(const QString& csvPath){
    if (gcodeConverter->loadCSVFile(csvPath)){
        return gcodeConverter->filterTopSidePoints();
    }
    else{
        qDebug() << "Failed to laod test points from .csv. ";
        return {};
    }
}
void GerberManager::getBoundingBox(){
    try{
        py::gil_scoped_acquire aquire;
        py::object pyGetBoundingBox = gerberStack.attr("getBoundingBox");
        py::object boundingBox = pyGetBoundingBox();

        if (boundingBox.is_none()) {
            qDebug() << "Bounding box is None. Cannot proceed.";
            return;
        }

        // Extract bounding box values
        minX = boundingBox["minX"].cast<double>();
        minY = boundingBox["minY"].cast<double>();
        maxX = boundingBox["maxX"].cast<double>();
        maxY = boundingBox["maxY"].cast<double>();

        qDebug() << "Bounding Box: min_x=" << minX << " min_y=" << minY << " max_x=" << maxX << " max_y=" << maxY;
    }
    catch (const py::error_already_set& e) {
        qDebug() << "Python error during fetching bounding box:" << QString::fromStdString(e.what());
    }
}


QPixmap GerberManager::overlayTestPoints(const QPixmap& baseImage, const QList<TestPoint>& points){
    if (baseImage.isNull()){
        qDebug() << "Base image is null.";
        return baseImage;
    }
    QSize imageSize = baseImage.size();
    qDebug() << "Base image size: width = " << imageSize.width() << ", height =" << imageSize.height();
    qDebug() << "Bounding box: min_x = " << minX << ", min_y = " << minY << ", max_x = " << maxX << ", max_y = " << maxY;

    double imageWidthMM = maxX - minX;
    double imageHeightMM = maxY - minY;


    qDebug() << "PCB width in mm: " << imageWidthMM << ", height in mm: " << imageHeightMM;

    if (imageWidthMM <= 0 || imageHeightMM <= 0) {
        qDebug() << "Invalid bounding box dimensions.";
        return baseImage;
    }

    double scaleX = imageSize.width() / imageWidthMM;
    double scaleY = imageSize.height() / imageHeightMM;

    qDebug() << "ScaleX: " << scaleX << ", ScaleY: " << scaleY;


    QPixmap imageTestPoints = baseImage;
    QPainter painter(&imageTestPoints);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(Qt::yellow, 2);   // Brighter color with thickness
    painter.setPen(pen);
    QBrush brush(Qt::yellow);
    painter.setBrush(brush);
    for (const auto& tp : points) {
        double scaledX = (tp.x - minX) * scaleX;
        double scaledY = imageSize.height() - ((tp.y - minY) * scaleY);
        qDebug() << "Original coordinates (" << tp.x << "," << tp.y << ") scaled to: (" << scaledX << "," << scaledY << ")";

        if (scaledX < 0 || scaledX >= imageSize.width() || scaledY < 0 || scaledY >= imageSize.height()) {
            qDebug() << "Point out of bounds: (" << scaledX << "," << scaledY << ")";
        }
        else {
            qDebug() << "Drawing point at scaled coordinates (" << scaledX << "," << scaledY << ")";
            painter.drawEllipse(QPointF(scaledX, scaledY), 20, 20);

            painter.drawLine(QPointF(scaledX - 10, scaledY), QPointF(scaledX + 10, scaledY)); // Horizontal line
            painter.drawLine(QPointF(scaledX, scaledY - 10), QPointF(scaledX, scaledY + 10)); // Vertical line
        }
    }
    painter.end();
    return imageTestPoints;
}

std::pair<std::vector<PadInfo>, std::vector<TraceInfo>> GerberManager::getPadAndTraceCoordinates() {
    try {
        py::gil_scoped_acquire acquire;
        py::object extractPadAndTraceCoords = gerberStack.attr("extractPadAndTraceCoords");
        py::object extractedData = extractPadAndTraceCoords();

        std::vector<PadInfo> padCoords;
        std::vector<TraceInfo> traceCoords;
        //double minX = 0;
        //double minY = 0;
        getBoundingBox();
        //qDebug() << "minX = " << minX << ", minY = " << minY;
        if (py::isinstance<py::dict>(extractedData)) { // Ensure the return type is a dictionary
            py::dict dataDict = extractedData.cast<py::dict>();

            // Extract pads
            if (dataDict.contains("pads")) {
                py::object padsList = dataDict["pads"];
                if (py::isinstance<py::sequence>(padsList)) {
                    for (auto item : padsList) {
                        if (py::isinstance<py::dict>(item)) {
                            py::dict padDict = item.cast<py::dict>();
                            PadInfo pad;
                            pad.x = padDict["x"].cast<double>()- minX;
                            pad.y = padDict["y"].cast<double>()- minY;
                            pad.aperture = padDict["aperture"].cast<std::string>();
                            padCoords.emplace_back(pad);
                        }
                    }
                }
            }

            // Extract traces
            if (dataDict.contains("traces")) {
                py::object tracesList = dataDict["traces"];
                if (py::isinstance<py::sequence>(tracesList)) {
                    for (auto item : tracesList) {
                        if (py::isinstance<py::dict>(item)) {
                            py::dict traceDict = item.cast<py::dict>();
                            TraceInfo trace;
                            trace.start_x = traceDict["start_x"].cast<double>() - minX  ;
                            trace.start_y =traceDict["start_y"].cast<double>() -minY;
                            trace.end_x = traceDict["end_x"].cast<double>() - minX;
                            trace.end_y =  traceDict["end_y"].cast<double>()-minY;
                           // trace.width = traceDict["width"].cast<std::string>();
                            traceCoords.emplace_back(trace);
                        }
                    }
                }
            }
        }
        return {padCoords, traceCoords};
    } catch (const py::error_already_set& e) {
        qDebug() << "Python error while extracting pad and trace coordinates: " << QString::fromStdString(e.what());
        return {};
    }
}

