#include "polldevicewidget.h"
#include "ui_polldevicewidget.h"

#include "modbusobj.h"

PollDeviceWidget::PollDeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PollDeviceWidget)
{
    ui->setupUi(this);
    connect(ModBusObjInstance::getInstance(),&ModBusObjInstance::signalPollParam,this,&PollDeviceWidget::onRecvData);
    loadDevData();
}

PollDeviceWidget::~PollDeviceWidget()
{
    delete ui;
}

void PollDeviceWidget::onRecvData(quint8 type, const QVariant &value)
{
    if(0 == type){

    }else if(1 == type){

    }else if(2 == type){

    }
}

void PollDeviceWidget::loadDevData()
{
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollSysParam,QVariant());
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollNetParam,QVariant());
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollSpParam,QVariant());

}
