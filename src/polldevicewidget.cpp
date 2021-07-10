#include "polldevicewidget.h"
#include "ui_polldevicewidget.h"

#include "modbusobj.h"

PollDeviceWidget::PollDeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PollDeviceWidget)
{
    ui->setupUi(this);
    connect(ModBusObjInstance::getInstance()->getModBusObj(),&ModBusObj::signalPollParam,this,&PollDeviceWidget::onRecvData);

    QRegExp rx ("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)") ;
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_localIp->setValidator(validator);
    ui->lineEdit_dns->setValidator(validator);
    ui->lineEdit_getaway->setValidator(validator);
    ui->lineEdit_remoteIp->setValidator(validator);

    //加载主机系统参数
    ModBusObjInstance::getInstance()->readPollParam(0);
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

        ui->spinBox_baud->setValue(pollSerialPortParam.baud);
        auto stopbit = int((pollSerialPortParam.param >> 8) & 0xff)/10;
        //停止位
        ui->comboBox_stopbit->setCurrentText(QString::number(stopbit));
        //校验位
        auto parity = (pollSerialPortParam.param & 0xf);
        ui->comboBox_pairty->setCurrentIndex(parity);
        //数据位
        auto databit = (pollSerialPortParam.param & 0xff) >> 4;
        ui->comboBox_databit->setCurrentText(QString::number(databit));

       //轮询设备长度
       ui->spinBox_cycleSize->setValue(pollSerialPortParam.cycleSize);
    }
}

void PollDeviceWidget::on_pushButton_sysTime_clicked()
{
    //写入系统时间
    PollSysParam sysParam;
    sysParam.sysDataTime1 = ui->spinBox_systime1->value();
    sysParam.sysDataTime2 = ui->spinBox_systime2->value();
    sysParam.sysDataTime3 = ui->spinBox_systime3->value();

    ModBusObjInstance::getInstance()->writePollParam(0,QVariant::fromValue(sysParam));
}

void PollDeviceWidget::on_pushButton_net_clicked()
{
    //写入网络参数
    PollNetParam netParam;
    netParam.localIP = ui->lineEdit_localIp->text();
    netParam.gateWay = ui->lineEdit_getaway->text();
    netParam.dns = ui->lineEdit_dns->text();
    netParam.localPort = ui->spinBox_localPort->value();
    netParam.remoteIP = ui->lineEdit_remoteIp->text();
    netParam.remotePort = ui->spinBox_remotePort->value();

    ModBusObjInstance::getInstance()->writePollParam(1, QVariant::fromValue(netParam));

}

void PollDeviceWidget::on_pushButton_serialport_clicked()
{
    //写入串口参数
    PollSerialPortParam sparam;
    sparam.baud = ui->spinBox_baud->value();
    sparam.cycleSize = ui->spinBox_cycleSize->value();
    quint16 tParam = 0;
    auto stopBit = ui->comboBox_stopbit->currentText().toInt();
    tParam  |= ((stopBit*10) & 0xff) << 8;

    auto databit = ui->comboBox_databit->currentText().toInt();
    tParam |= (databit & 0xf) << 4;

    auto parity = ui->comboBox_pairty->currentIndex();
    tParam |= (parity & 0xf);

    sparam.param = tParam;

    ModBusObjInstance::getInstance()->writePollParam(2, QVariant::fromValue(sparam));
}

void PollDeviceWidget::on_pushButton_sysparam_clicked()
{
    ModBusObjInstance::getInstance()->readPollParam(0);
}

void PollDeviceWidget::on_pushButton_netparam_clicked()
{
    ModBusObjInstance::getInstance()->readPollParam(1);
}

void PollDeviceWidget::on_pushButton_sparam_clicked()
{
    ModBusObjInstance::getInstance()->readPollParam(2);
}

