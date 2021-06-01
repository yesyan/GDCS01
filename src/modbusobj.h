#ifndef MODBUSOBJ_H
#define MODBUSOBJ_H

#include <QObject>
#include "modbus.h"
#include <QThread>

class ModBusObj : public QObject
{
    Q_OBJECT

public:

    ModBusObj(QObject *parent = nullptr);
    ~ModBusObj();

public slots:
    //网络连接
    int netWorkConnect(const QString &ip,int port);
    //串口连接
    int serialPortConnect(const QString &dev,int baud,char parity,int dataBit,int stopBit);

    //当前连接分机地址
    int getConnectSlaveAddr();
    //获取分机简要信息
    int getSlaveSimpleInfo(int slave,QVariantHash &value);
    //获取分机设备参数
    int getSlaveDevParam(int slave,QVariantHash &value);


    //设置系统时间
    int setSysTime(const QDateTime &dateTime);


private:
    //停止连接
    int stopConnect();

private:
    modbus_t *m_modBusCtx = nullptr;
    QString m_connectType;
};


class ModBusObjInstance : public QObject{

    Q_OBJECT
public:
    static ModBusObjInstance *getInstance()
    {
        static ModBusObjInstance instance;
        return &instance;
    }

    int connectToNet(const QString &ip,int port);
    int connectToSerialPort(const QString &dev,int baud,char parity,int dataBit,int stopBit);


signals:
    void getSlaveSimpleInfo(int slave);
    void getSlaveDevParam(int slave);

    void slaveSimpleInfo(const QVariantHash &);
    void slaveDevParam(const QVariantHash &);

private:
    ModBusObjInstance();
    ~ModBusObjInstance();
    ModBusObjInstance(const ModBusObjInstance& rParam) = delete;
    const ModBusObjInstance& operator = (const ModBusObjInstance& rParam) = delete;

private:
    QThread m_thread;
    ModBusObj *m_modBusObj = nullptr;
};

#endif // MODBUSOBJ_H
