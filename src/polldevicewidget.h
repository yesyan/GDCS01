#ifndef POLLDEVICEWIDGET_H
#define POLLDEVICEWIDGET_H

#include <QWidget>

namespace Ui {
class PollDeviceWidget;
}

class PollDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PollDeviceWidget(QWidget *parent = nullptr);
    ~PollDeviceWidget();

public slots:
    void onRecvData(quint8 type,const QVariant &);
private:
    //加载设备数据
    void loadDevData();
private:
    Ui::PollDeviceWidget *ui;
};

#endif // POLLDEVICEWIDGET_H
