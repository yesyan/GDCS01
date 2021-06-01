#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "devicewidget.h"
#include "connectdialog.h"
#include "modbusobj.h"
#include "polldevicewidget.h"

#include <QStackedLayout>
#include <QTableWidget>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUi();
    //showMaximized();
    //读取主机参数
    //检测分机状态
    checkSlaveStatus();

    connect(ModBusObjInstance::getInstance(),&ModBusObjInstance::slaveDevParam,this,&MainWindow::onSlaveSimpleInfo);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSlaveSimpleInfo(const QVariantHash &)
{
    //busId
}

void MainWindow::initUi()
{
    auto layout = new QStackedLayout;
    m_slaveTableWidget = new QTableWidget;
    m_slaveTableWidget->setRowCount(32);
    m_slaveTableWidget->setColumnCount(3);
    m_slaveTableWidget->setHorizontalHeaderLabels({QStringLiteral("设备ID") ,
                                                   QStringLiteral("硬件版本"),
                                                   QStringLiteral("软件版本")});
    m_slaveTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_slaveTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_slaveTableWidget->setItem(0,0,new QTableWidgetItem("ddd"));
    m_slaveTableWidget->setItem(0,1,new QTableWidgetItem("sss"));
    m_slaveTableWidget->setItem(0,2,new QTableWidgetItem("vvv"));

    layout->addWidget(m_slaveTableWidget);

    m_devWid = new DeviceWidget;
    layout->addWidget(m_devWid);

    ui->tab_2->setLayout(layout);

    connect(m_slaveTableWidget,&QTableWidget::doubleClicked,this,[=](const QModelIndex &index){

        //TODO 设置数据
        //auto row = index.row();
        //切换界面
        layout->setCurrentIndex(1);
    });
}

void MainWindow::checkSlaveStatus()
{

}

void MainWindow::loadPollParam()
{

}

