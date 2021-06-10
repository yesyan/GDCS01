#include "devicewidget.h"
#include "ui_devicewidget.h"

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceWidget)
{
    ui->setupUi(this);
    initUi();
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::loadData(uint8_t slave)
{
    ui->label_slave->setText(QStringLiteral("从机%1参数信息").arg(slave));
}

void DeviceWidget::initUi()
{
    connect(ui->toolButton_back,&QToolButton::clicked,this,[=](){
        emit signalBack();
    });
}
