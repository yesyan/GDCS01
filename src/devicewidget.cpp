#include "devicewidget.h"
#include "ui_devicewidget.h"
#include "modbusobj.h"
#include "devicechartview.h"

#include <QMessageBox>

static const int SlaveReadSize = 36;

DeviceWidget::DeviceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceWidget)
{
    ui->setupUi(this);
    initUi();

    connect(ModBusObjInstance::getInstance()->getModBusObj(),&ModBusObj::signalReadValue,this,&DeviceWidget::onRecvModBusValue);
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::loadData(uint8_t slave)
{
    ui->label_slave->setText(QStringLiteral("从机%1参数信息").arg(slave));
    ModBusObjInstance::getInstance()->readModBusRegister(slave,0,SlaveReadSize);
    m_slave = slave;
}

void DeviceWidget::onRecvModBusValue(int slave, int addr, const QByteArray &value)
{
    if(slave == m_slave && addr == 0 && value.size() == SlaveReadSize*sizeof (quint16)){


        QVector<quint16> tValue;
        tValue.resize(35);
        memcpy(tValue.data(),value.data(),value.size());

        //信道
        ui->spinBox_2->setValue(tValue[2]);
        //采样率
        ui->spinBox_3->setValue(tValue[3]);
        //是否开启报警检测
        ui->checkBox_4->setChecked(tValue[4] == 1);
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
        ui->spinBox_11->setValue((float)(1.0)*tValue[11]/10); //特征是数据解析时缩小10倍
        ui->spinBox_12->setValue((float)(1.0)*tValue[12]/10);
        ui->spinBox_13->setValue((float)(1.0)*tValue[13]/10);
        //y方向
        ui->spinBox_14->setValue((float)(1.0)*tValue[14]/10);
        ui->spinBox_15->setValue((float)(1.0)*tValue[15]/10);
        ui->spinBox_16->setValue((float)(1.0)*tValue[16]/10);
        //y方向
        ui->spinBox_17->setValue((float)(1.0)*tValue[17]/10);
        ui->spinBox_18->setValue((float)(1.0)*tValue[18]/10);
        ui->spinBox_19->setValue((float)(1.0)*tValue[19]/10);
        //温度报警预警值
        ui->spinBox_20->setValue((float)(1.0)*tValue[20]/10);
        //采样间隔
        ui->spinBox_21->setValue(tValue[21]);
        //连续时间采样间隔
        ui->spinBox_22->setValue(tValue[22]);
        //连续震动采集长度
        ui->spinBox_23->setValue(tValue[23]);
        //设置报警是否需要发送连续波形
        ui->checkBox_24->setChecked(tValue[24] == 1);
        //连续数据反馈
        ui->spinBox_31->setValue(tValue[31]);
    }
}

void DeviceWidget::on_pushButton_reqData_clicked()
{
    auto dataType = 1 << ui->comboBox->currentIndex();
    ModBusObjInstance::getInstance()->readContinuData(m_slave,dataType);
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
        tmpData.append(quint16(ui->spinBox_11->value()*10));  //特征是数据发送时放大10倍
        tmpData.append(quint16(ui->spinBox_12->value()*10));
        tmpData.append(quint16(ui->spinBox_13->value()*10));

        //y方向
        tmpData.append(quint16(ui->spinBox_14->value()*10));
        tmpData.append(quint16(ui->spinBox_15->value()*10));
        tmpData.append(quint16(ui->spinBox_16->value()*10));

        //z方向
        tmpData.append(quint16(ui->spinBox_17->value()*10));
        tmpData.append(quint16(ui->spinBox_18->value()*10));
        tmpData.append(quint16(ui->spinBox_19->value()*10));

        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,11,tmpData);

        //采样率
        tmpData.clear();
        tmpData.append(quint16(ui->spinBox_3->value()));
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,3,tmpData);

        //温度预警
        tmpData.clear();
        tmpData.append(quint16(ui->spinBox_20->value()*10));

        //采样间隔
        tmpData.append(quint16(ui->spinBox_21->value()));
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,20,tmpData);

    });

    //设置参数
    connect(ui->buttonGroup_2,QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this,[=](QAbstractButton *button){
        auto addr = button->objectName().split('_').last();
        QVector<quint16> tmpData;
        auto spinBox = this->findChild<QSpinBox*>("spinBox_" + addr);
        if(spinBox){
            tmpData.append(quint16(spinBox->value()));
            ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,addr.toInt(),tmpData);
        }
    });

    //控制曲线图显示与隐藏
    connect(ui->buttonGroup,QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this,[=](QAbstractButton *button){
        auto index = button->objectName().split('_').last().toInt();
        ui->widget->setLineSeriesVisible(index,button->isChecked());
    });

    //是否开启报警检测
    connect(ui->checkBox_4,&QCheckBox::clicked,this,[=](bool checked){
        QVector<quint16> tmpData;
        quint16 tValue = checked ? 1 : 0;
        tmpData.append(tValue);
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,4,tmpData);
    });

    //设置报警是否需要发送连续波形
    connect(ui->checkBox_24,&QCheckBox::clicked,this,[=](bool checked){
        QVector<quint16> tmpData;
        quint16 tValue = checked ? 1 : 0;
        tmpData.append(tValue);
        ModBusObjInstance::getInstance()->writeModBusRegister(m_slave,24,tmpData);
    });
}
