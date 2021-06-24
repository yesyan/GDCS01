#include "devicewidget.h"
#include "ui_devicewidget.h"
#include "modbusobj.h"

#include <QMessageBox>

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceWidget)
{
    ui->setupUi(this);
    initUi();

    connect(ModBusObjInstance::getInstance(),&ModBusObjInstance::signalReadValue,this,&DeviceWidget::onRecvModBusValue);
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::loadData(uint8_t slave)
{
    ui->label_slave->setText(QStringLiteral("从机%1参数信息").arg(slave));
    ModBusObjInstance::getInstance()->readModBusRegister(slave,1,35);
    m_slave = slave;
}

void DeviceWidget::onRecvModBusValue(int slave, int addr, const QByteArray &value)
{
    if(slave == m_slave && addr == 1 && value.size() == 35*sizeof (quint16)){


        QVector<quint16> tValue;
        tValue.resize(35);
        memcpy(tValue.data(),value.data(),value.size());

        //信道
        ui->spinBox_2->setValue(tValue[2]);
        //采样率
        ui->spinBox_3->setValue(tValue[3]);
        //空中速率
        ui->spinBox_5->setValue(tValue[5]);
        //远端通讯格式
        ui->comboBox_6->setCurrentIndex(tValue[6]);
        //RS485波特率
        int baud = (tValue[7] << 16) | tValue[8];
        ui->spinBox_7->setValue(baud);
        //校验位
        ui->comboBox_9_1->setCurrentIndex(tValue[9] & 0xf);
        //数据位
        ui->comboBox_9_3->setCurrentText(QString::number((tValue[9] >> 4) & 0xf));
        //停止位
        ui->comboBox_9_2->setCurrentText(QString::number(((tValue[9] >> 8) & 0xff)/10));

        //x方向
        ui->spinBox_11->setValue(tValue[11]);
        ui->spinBox_12->setValue(tValue[12]);
        ui->spinBox_13->setValue(tValue[13]);
        //y方向
        ui->spinBox_14->setValue(tValue[14]);
        ui->spinBox_15->setValue(tValue[15]);
        ui->spinBox_16->setValue(tValue[16]);
        //y方向
        ui->spinBox_17->setValue(tValue[17]);
        ui->spinBox_18->setValue(value[18]);
        ui->spinBox_19->setValue(value[19]);
        //温度报警预警值
        ui->spinBox_20->setValue(value[20]);
        //采样间隔
        ui->spinBox_21->setValue(value[21]);
        //连续时间采样间隔
        ui->spinBox_22->setValue(value[22]);
        //连续震动采集长度
        ui->spinBox_23->setValue(value[23]);
        //连续数据反馈
        ui->spinBox_31->setValue(value[31]);
    }
}

void DeviceWidget::onWriteModBusRegister()
{
    auto addr = this->sender()->objectName().split('_').last();
    QVector<quint16> tmpData;
    auto spinBoxName = "spinBox_" + addr;
    auto spinBox = this->findChild<QSpinBox*>(spinBoxName);
    if(spinBox){
        tmpData.append(quint16(spinBox->value()));
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,addr.toInt(),tmpData);
    }
}

void DeviceWidget::initUi()
{
    connect(ui->toolButton_back,&QToolButton::clicked,this,[=](){
        emit signalBack();
    });

    //恢复出厂设置
    connect(ui->pushButton,&QPushButton::clicked,this,[=](){

        if(QMessageBox::question(nullptr,QStringLiteral("提示"),QStringLiteral("确定恢复出厂设置?")) == QMessageBox::Ok){
            QVector<quint16> tmpData;
            tmpData.append(1);
            ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,10,tmpData);
        }
    });

    //网络参数
    connect(ui->pushButton_6_9,&QPushButton::clicked,this,[=](){
          QVector<quint16> tmpData;
          //远端数据通讯格式
          tmpData.append(quint16(ui->comboBox_6->currentIndex()));
          //RS485波特率
          tmpData.append((ui->spinBox_7->value() >> 16) & 0xffff);
          tmpData.append(ui->spinBox_7->value() & 0xffff);
          //奇偶、停止、数据位
          quint16 tValue = quint16(ui->comboBox_9_1->currentIndex());
          tValue |= ui->comboBox_9_3->currentText().toInt() << 4;
          tValue |= ui->comboBox_9_2->currentText().toInt()*10 << 8;
          tmpData.append(tValue);

          ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,6,tmpData);
    });

    //数据配置
    connect(ui->pushButton_11_20,&QPushButton::clicked,this,[=](){
        QVector<quint16> tmpData;
        //X方向
        tmpData.append(quint16(ui->spinBox_11->value()));
        tmpData.append(quint16(ui->spinBox_12->value()));
        tmpData.append(quint16(ui->spinBox_13->value()));

        //y方向
        tmpData.append(quint16(ui->spinBox_14->value()));
        tmpData.append(quint16(ui->spinBox_15->value()));
        tmpData.append(quint16(ui->spinBox_16->value()));

        //z方向
        tmpData.append(quint16(ui->spinBox_17->value()));
        tmpData.append(quint16(ui->spinBox_18->value()));
        tmpData.append(quint16(ui->spinBox_19->value()));

        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,11,tmpData);

        //采样率
        tmpData.clear();
        tmpData.append(quint16(ui->spinBox_3->value()));
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,3,tmpData);

        //温度预警
        tmpData.clear();
        tmpData.append(quint16(ui->spinBox_20->value()));

        //采样间隔
        tmpData.append(quint16(ui->spinBox_21->value()));
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,20,tmpData);

    });
    connect(ui->pushButton_22,&QPushButton::clicked,this,&DeviceWidget::onWriteModBusRegister);
    connect(ui->pushButton_23,&QPushButton::clicked,this,&DeviceWidget::onWriteModBusRegister);
    //信道
    connect(ui->pushButton_2,&QPushButton::clicked,this,&DeviceWidget::onWriteModBusRegister);
    connect(ui->pushButton_31,&QPushButton::clicked,this,&DeviceWidget::onWriteModBusRegister);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&DeviceWidget::onWriteModBusRegister);

}
