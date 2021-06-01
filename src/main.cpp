#include "mainwindow.h"
#include "connectdialog.h"
#include "modbusobj.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
        return a.exec();
    }
    return 0;
}
