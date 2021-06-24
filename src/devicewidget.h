#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QWidget>

namespace Ui {
class DeviceWidget;
}

class DeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceWidget(QWidget *parent = nullptr);
    ~DeviceWidget();

    void loadData(uint8_t slave);

public slots:
    void onRecvModBusValue(int slave, int addr, const QByteArray &value);

private slots:
    void onWriteModBusRegister();

signals:
    void signalBack();

private:
    void initUi();

private:
    Ui::DeviceWidget *ui;
    quint8 m_slave;
};

#endif // DEVICEWIDGET_H
