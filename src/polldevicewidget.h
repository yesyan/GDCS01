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

private:
    Ui::PollDeviceWidget *ui;
};

#endif // POLLDEVICEWIDGET_H
