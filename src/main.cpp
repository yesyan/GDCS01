#include "mainwindow.h"
#include "connectdialog.h"
#include "loghandler.h"
#include "modbusobj.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

static void initialize()
{

    // 安装日志处理工具
    LogHandler::getInstance()->installMessageHandler();

    // 设置界面样式
    QStringList qssFileNames;
    qssFileNames << ":/qss/WhiteStyle.qss";
    QString qss;

    for (const QString &name : qssFileNames) {
        qDebug().noquote() << QString("Loading QSS file: %1").arg(name);

        QFile file(name);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug().noquote() << QString("Error: Loading QSS file: %1 failed").arg(name);
            continue;
        }

        qss.append(file.readAll()).append("\n");
        file.close();
    }

    if (!qss.isEmpty()) {
        qApp->setStyleSheet(qss);
    }
}


static void finalize()
{
    LogHandler::getInstance()->uninstallMessageHandler();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ::initialize();

    ConnectDialog connectDlg;
    if(QDialog::Accepted == connectDlg.exec()){

        QVariantHash value;
        connectDlg.getData(value);
        auto type = value["type"].toString();
        if(type == "net"){
            auto ip = value["ip"].toString();
            auto port = value["port"].toInt();
            ModBusObjInstance::getInstance()->connectToNet(ip,port);
        }else{
            //TODO串口连接
        }

        auto w = new MainWindow;
        w->show();
        int code = a.exec();
        ::finalize();
        return code;
    }
    return 0;
}
