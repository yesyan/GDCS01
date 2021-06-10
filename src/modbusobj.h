#ifndef MODBUSOBJ_H
#define MODBUSOBJ_H

#include <QObject>
#include "modbus.h"
#include <QThread>
#include "global.h"

class ModBusObj : public QObject
{
    Q_OBJECT

public:

    ModBusObj(QObject *parent = nullptr);
    ~ModBusObj();

signals:
    void signalPollParam(quint8 paramType, const QVariant &);
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

    //操作主机参数
    void pollParam(quint8 opType, quint8 paramType,const QVariant &param);

private:
    int stopConnect();
    void readPollSysParam();
    void readPollNetParam();
    void writePollNetParam(const QVariant &);

    void readPollSerialPortParam();
    void writePollSerialPortParam(const QVariant &);
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

    //操作方式，读取或者写入
    enum OperationType{
        READ  = 0,
        WRITE = 1
    };

    enum ParamType{
        PollSysParam = 0, //主机系统参数
        PollNetParam,     //主机网络参数
        PollSpParam       //主机串口参数
    };

public:
    void pollParam(OperationType opType,ParamType paramType,const QVariant &param);

signals:
    //主机数据
    void signalPollParam(quint8 type,const QVariant &);

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
