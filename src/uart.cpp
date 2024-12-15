#include "include/uart.h"
#include "qdebug.h"


UART::UART(QObject *parent)
    : QObject(parent), serialPort(new QSerialPort(this))
{
    connect(serialPort, &QSerialPort::readyRead, this, [=]() {
        emit dataReceived(serialPort->readAll());
    });

    connect(serialPort, &QSerialPort::errorOccurred, this, [](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError)
            qWarning() << "Serial port error:" << error;
    });
}

UART::~UART()
{
    if (serialPort->isOpen())
        serialPort->close();
    delete serialPort;
}

bool UART::connectPort(const QString &portName)
{
    if (serialPort->isOpen())
        serialPort->close();

    serialPort->setPortName(portName);
    if (serialPort->open(QIODevice::ReadWrite)) {
        emit connectionStatusChanged(true);
        return true;
    }
    emit connectionStatusChanged(false);
    return false;
}

void UART::disconnectPort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        emit connectionStatusChanged(false);
    }
}

QStringList UART::availablePorts() const
{
    QStringList ports;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        ports.append(info.portName());
    }
    return ports;
}

bool UART::isConnected() const
{
    return serialPort->isOpen();
}

void UART::sendData(const QByteArray &data)
{
    if (serialPort->isOpen())
        serialPort->write(data);
}
