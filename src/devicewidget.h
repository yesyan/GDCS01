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
    //请求从机
    void on_pushButton_reqData_clicked();
    //重新加载从机设备参数
    void on_pushButton_reload_clicked();

signals:
    void signalBack();

private:
    void initUi();

private:
    Ui::DeviceWidget *ui;
    quint8 m_slave;
};

#endif // DEVICEWIDGET_H
