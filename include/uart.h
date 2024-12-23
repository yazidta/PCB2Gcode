#ifndef UART_H
#define UART_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>


class UART : public QObject
{
    Q_OBJECT

public:
    explicit UART(QObject *parent = nullptr);
    ~UART();

    bool connectPort(const QString &portName);
    void disconnectPort();
    QStringList availablePorts() const;
    bool isConnected() const;
    void sendData(const QByteArray &data);

Q_SIGNALS:
    void connectionStatusChanged(bool Connected);
    void dataReceived(const QByteArray &data);

private:
    QSerialPort *serialPort;
};

#endif // UART_H
