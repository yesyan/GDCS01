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

void PollDeviceWidget::onRecvData(quint8 type,const QVariant &value)
{
    if(0 == type){
        //主机系统参数
        auto pollSysParam = value.value<PollSysParam>();
        ui->lineEdit_devId->setText(pollSysParam.devId);
        ui->lineEdit_hardversion->setText(pollSysParam.hardVersion);
        ui->lineEdit_softversion->setText(pollSysParam.softVersion);
        ui->spinBox_systime1->setValue(pollSysParam.sysDataTime1);
        ui->spinBox_systime2->setValue(pollSysParam.sysDataTime2);
        ui->spinBox_systime3->setValue(pollSysParam.sysDataTime3);
        //ui->lineEdit_sysdatetime->setText(pollSysParam.sysDataTime);

    }else if(1 == type){
        //主机网络参数
        auto pollNetParam = value.value<PollNetParam>();
        ui->lineEdit_localIp->setText(pollNetParam.localIP);
        ui->lineEdit_remoteIp->setText(pollNetParam.remoteIP);
        ui->lineEdit_getaway->setText(pollNetParam.gateWay);
        ui->lineEdit_dns->setText(pollNetParam.dns);
        ui->spinBox_localPort->setValue(pollNetParam.localPort);
        ui->spinBox_remotePort->setValue(pollNetParam.remotePort);

    }else if(2 == type){
        //主机串口参数
        auto pollSerialPortParam = value.value<PollSerialPortParam>();

        ui->comboBox_baud->setCurrentText(QString::number(pollSerialPortParam.baud));
        auto stopbit = double((pollSerialPortParam.param >> 8) & 0xff)/10;
        //停止位
        ui->comboBox_stopbit->setCurrentText(QString::number(stopbit));
        //校验位
        auto parity = (pollSerialPortParam.param & 0xf);
        ui->comboBox_pairty->setCurrentIndex(parity);
        //数据位
        auto databit = (pollSerialPortParam.param & 0xff) >> 4;
         ui->comboBox_databit->setCurrentText(QString::number(databit));
    }
}

void PollDeviceWidget::on_pushButton_sysTime_clicked()
{
    //写入系统时间
    PollSysParam sysParam;
    sysParam.sysDataTime1 = ui->spinBox_systime1->value();
    sysParam.sysDataTime2 = ui->spinBox_systime2->value();
    sysParam.sysDataTime3 = ui->spinBox_systime1->value();

    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::WRITE,ModBusObjInstance::PollSysParam,QVariant::fromValue(sysParam));
}

void PollDeviceWidget::on_pushButton_net_clicked()
{
    //TODO 写入网络参数
}

void PollDeviceWidget::on_pushButton_serialport_clicked()
{
    //TODO 写入串口参数
}

void PollDeviceWidget::loadDevData()
{
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollSysParam,QVariant());
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollNetParam,QVariant());
    ModBusObjInstance::getInstance()->pollParam(ModBusObjInstance::READ,ModBusObjInstance::PollSpParam,QVariant());
}

