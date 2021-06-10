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
signals:
    void signalBack();

private:
    void initUi();
private:
    Ui::DeviceWidget *ui;
};

#endif // DEVICEWIDGET_H
