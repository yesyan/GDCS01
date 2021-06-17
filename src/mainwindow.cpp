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


static const int SlaveCount = 32;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUi();
    setWindowTitle(QStringLiteral("振动软件"));

    connect(ModBusObjInstance::getInstance(),&ModBusObjInstance::signalSlaveParam,this,&MainWindow::onRecvSlaveParam);
    //检测分机状态
    checkSlaveStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onRecvSlaveParam(int slave, int type, const QVariant &value)
{
    if(type == ModBusObjInstance::SlaveSysParam){
        auto devId = value.toHash()[DeviceID].toString();
        auto hardVersion = value.toHash()[HardwareVersion].toString();
        auto softVersion = value.toHash()[SoftwareVersion].toString();
        m_slaveTableWidget->item(slave,0)->setText(devId);
        m_slaveTableWidget->item(slave,1)->setText(hardVersion);
        m_slaveTableWidget->item(slave,2)->setText(softVersion);
    }
}

void MainWindow::initUi()
{
    auto layout = new QStackedLayout;
    m_slaveTableWidget = new QTableWidget;
    m_slaveTableWidget->setRowCount(SlaveCount);
    m_slaveTableWidget->setColumnCount(3);
    m_slaveTableWidget->setHorizontalHeaderLabels({DeviceID, HardwareVersion, SoftwareVersion});
    //m_slaveTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_slaveTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    for(auto row = 0;row < SlaveCount; ++row){
        m_slaveTableWidget->setItem(row,0,new QTableWidgetItem("none"));
        m_slaveTableWidget->setItem(row,1,new QTableWidgetItem("none"));
        m_slaveTableWidget->setItem(row,2,new QTableWidgetItem("none"));
    }

    layout->addWidget(m_slaveTableWidget);

    m_devWid = new DeviceWidget;
    layout->addWidget(m_devWid);
    connect(m_devWid,&DeviceWidget::signalBack,this,[=](){
        layout->setCurrentIndex(0);
    });
    ui->tab_2->setLayout(layout);

    connect(m_slaveTableWidget,&QTableWidget::doubleClicked,this,[=](const QModelIndex &index){
        auto row = index.row();
        m_devWid->loadData(row);
        layout->setCurrentIndex(1);
    });

    auto tlayout = new QHBoxLayout;
    auto textEdit = LogHandler::getInstance()->getTextView();
    tlayout->addWidget(textEdit);
    ui->widget->setLayout(tlayout);
}

void MainWindow::checkSlaveStatus()
{
    for(auto slave = 0 ; slave < SlaveCount ; ++slave){
        ModBusObjInstance::getInstance()->readSlaveParam(slave,ModBusObjInstance::SlaveSysParam);
    }
}

