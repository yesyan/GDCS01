#include "polldevicewidget.h"
#include "ui_polldevicewidget.h"

PollDeviceWidget::PollDeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PollDeviceWidget)
{
    ui->setupUi(this);
}

PollDeviceWidget::~PollDeviceWidget()
{
    delete ui;
}
