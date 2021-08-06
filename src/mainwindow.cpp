#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicewidget.h"
#include "connectdialog.h"
#include "modbusobj.h"
#include "polldevicewidget.h"
#include "loghandler.h"
#include "global.h"
#include "modbusobj.h"

#include <QStackedLayout>
#include <QTableWidget>
#include <QMenu>
#include <QTextEdit>


static const int SlaveCount = 30;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUi();
    setWindowTitle(QStringLiteral("振动软件"));

    connect(ModBusObjInstance::getInstance()->getModBusObj(),&ModBusObj::signalAlarmInfo,this,&MainWindow::onRecvCycleInfo);

    //开启定时轮询
    ModBusObjInstance::getInstance()->startAlarmCycle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRecvCycleInfo(const QVariant &value)
{
    auto listValue = value.toList();
    for(auto index = 0; index < listValue.count(); ++index){
        for(auto cIndex = 0; cIndex < ui->tableWidget->columnCount(); ++cIndex){

            if(!listValue[index].isValid()){
                auto item = ui->tableWidget->item(index,cIndex);
                item->setText("none");
            }
            auto columnText = ui->tableWidget->horizontalHeaderItem(cIndex)->text();
            if(listValue[index].toHash().contains(columnText)){
                auto tValue = listValue[index].toHash()[columnText].toString();
                auto item = ui->tableWidget->item(index,cIndex);
                if(tValue == QStringLiteral("告警")){
                    item->setText(tValue);
                    item->setTextColor(Qt::red);
                }else{
                    item->setText(tValue);
                    item->setTextColor(QColor(2,65,132));
                }
            }
        }
    }
}

void MainWindow::initUi()
{
    ui->tableWidget->setRowCount(SlaveCount);
    ui->tableWidget->setColumnCount(14);
    ui->tableWidget->setHorizontalHeaderLabels({DeviceID, HardwareVersion, SoftwareVersion,
                                                   X_AccSpeed,X_Speed,X_Displace,Y_AccSpeed,
                                                   Y_Speed,Y_Displace,Z_AccSpeed,Z_Speed,
                                                   Z_Displace,Slave_Temp,Slave_Temp_Alarm});

    for(auto row = 0;row < SlaveCount; ++row){
        for(auto column = 0 ; column < ui->tableWidget->columnCount(); ++column){
            auto item = new QTableWidgetItem("none");
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row,column,item);
        }
    }

    //双击跳转到设备详细参数
    connect(ui->tableWidget,&QTableWidget::doubleClicked,this,[=](const QModelIndex &index){
        auto row = index.row();
        m_devWid = new DeviceWidget;

        connect(m_devWid,&DeviceWidget::signalBack,this,[=](){
            ui->stackedWidget->setCurrentIndex(0);
            ui->stackedWidget->removeWidget(m_devWid);
            m_devWid->deleteLater();
            m_devWid = nullptr;
        });
        ui->stackedWidget->addWidget(m_devWid);
        m_devWid->loadData(row+1);
        ui->stackedWidget->setCurrentIndex(1);
    });

    //选中区域变化，读取分机参数
    connect(ui->tableWidget,&QTableWidget::currentItemChanged,this,[=](QTableWidgetItem *current, QTableWidgetItem *previous){

        if(previous){
            if(current->row() == previous->row())
                return;
        }

        auto slave = current->row()+1;
        ModBusObjInstance::getInstance()->readSlaveReadOnlyData(slave);

    });

    //刷新分机的只读参数
    connect(ModBusObjInstance::getInstance()->getModBusObj(),&ModBusObj::signalSlaveFV,this,[=](int slave,const QVariant &value){

        ui->groupBox->setTitle(QStringLiteral("分机%1只读参数").arg(slave));
        //刷新界面
        QHash<QString, QVariant>::const_iterator iter = value.toHash().constBegin();
        while (iter != value.toHash().constEnd()) {
            QString className = "spinBox_" + iter.key();
            auto spinBox = this->findChild<QSpinBox*>(className);
            if(spinBox){
                spinBox->setValue(iter.value().toInt());
            }
            ++iter;
        }

    });

    auto tlayout = new QHBoxLayout;
    auto textEdit = LogHandler::getInstance()->getTextView();
    tlayout->addWidget(textEdit);
    ui->widget_log->setLayout(tlayout);
}


