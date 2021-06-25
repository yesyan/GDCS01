#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "modbusobj.h"

#include <QMessageBox>

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("连接方式"));
    ui->groupBox->setDisabled(true);
    connect(ui->checkBox_net,&QCheckBox::stateChanged,this,[this](int state){
        if(state == Qt::Checked){
            ui->groupBox->setDisabled(true);
            ui->groupBox_2->setDisabled(false);
        }
    });
    connect(ui->checkBox_serialport,&QCheckBox::stateChanged,this,[this](int state){
        if(state == Qt::Checked){
            ui->groupBox->setDisabled(false);
            ui->groupBox_2->setDisabled(true);
        }
    });

    QRegExp rx ("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)") ;
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->lineEdit_ip->setValidator(validator);

    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,[=](){
        if(connectModBus()){
            this->accept();
        }else{
            QMessageBox::information(nullptr,QStringLiteral("提示"),QStringLiteral("连接ModBus失败,程序退出."));
            this->reject();
        }
    });

    connect(ui->buttonBox,&QDialogButtonBox::rejected,this,[=](){
        this->reject();
    });

    //测试连接
    connect(ui->pushButton,&QPushButton::clicked,this,[=](){
        if(connectModBus()){
            QMessageBox::information(nullptr,QStringLiteral("提示"),QStringLiteral("连接ModBus成功."));
        }else{
            QMessageBox::information(nullptr,QStringLiteral("提示"),QStringLiteral("连接ModBus失败."));
        }
    });
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

bool ConnectDialog::connectModBus()
{
    if(ui->checkBox_net->isChecked()){
        auto ip = ui->lineEdit_ip->text();
        auto localPort = ui->spinBox_port->value();

        return (-1 != ModBusObjInstance::getInstance()->connectToNet(ip,localPort));
    }else{

        auto comPort = ui->comboBox_port->currentText();
        auto baud    = ui->comboBox_baud->currentText().toInt();
        auto dataBit = ui->comboBox_dataBit->currentText().toInt();
        auto parity  = ui->comboBox_parity->currentText();
        char parityChar;
        if(parity == QStringLiteral("无校验")){
            parityChar = 'N';
        }else if(parity == QStringLiteral("奇校验")){
            parityChar = 'O';
        }else if(parity == QStringLiteral("偶校验")){
            parityChar = 'E';
        }
        auto stopBit = ui->comboBox_stopBit->currentText().toInt();
        return (-1 != ModBusObjInstance::getInstance()->connectToSerialPort(comPort,baud,parityChar,dataBit,stopBit));
    }
}
