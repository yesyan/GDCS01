#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);

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
}

void ConnectDialog::getData(QVariantHash &value)
{
    if(ui->checkBox_net->isChecked()){
        value["type"] = "net";
        value["ip"] = ui->lineEdit_ip->text();
        value["port"] = ui->spinBox_port->value();
    }else{
        value["type"] = "serialPort";
        value["port"] = ui->comboBox_port->currentText();
        value["baud"] = ui->comboBox_baud->currentText();
        value["dataBit"] = ui->comboBox_dataBit->currentText();
        value["parity"] = ui->comboBox_parity->currentText();
        value["stopBit"] = ui->comboBox_stopBit->currentText();
    }
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}
