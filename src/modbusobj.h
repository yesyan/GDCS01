#ifndef MODBUSOBJ_H
#define MODBUSOBJ_H

#include <QObject>
#include <QThread>

#include "modbus.h"
#include "global.h"

class ModBusObj : public QObject
{
    Q_OBJECT

public:

    ModBusObj(QObject *parent = nullptr);
    ~ModBusObj();

signals:
    void signalPollParam(quint8 paramType, const QVariant &);

    void signalReadValue(int slave,int addr,const QByteArray &);
    void signalContinuData(int type,const QByteArray &value);
    //报警信息
    void signalAlarmInfo(const QVariant &);

public slots:
    //网络连接
    int netWorkConnect(const QString &ip,int port);
    //串口连接
    int serialPortConnect(const QString &dev,int baud,char parity,int dataBit,int stopBit);

    //当前连接分机地址
    int getConnectSlaveAddr();

    //操作主机参数
    void pollParam(quint8 opType, quint8 paramType,const QVariant &param);

    //根据地址读取数据
    void readModBusRegister(int slave,int addr,int readCount,bool readOnly = false);
    //根据地址写入数据
    void writeModBusRegister(int slave,int addr,const QVector<quint16> &value);

    //读取连续数据
    void readContinuData(int slave ,int dataType,int timeOut);
    //启动报警信息轮询定时器
    void startAlarmTimer();
    //请求分机只读数据
    void readSlaveReadOnlyData(int slave, QVariantHash &value);

protected:
     void timerEvent(QTimerEvent *event) override;

private:
    int stopConnect();
    void readPollSysParam();
    void readPollNetParam();
    void writePollNetParam(const QVariant &);

    //获取分机系统参数
    bool getSlaveSysParam(int slave,QVariantHash &value);

    void readPollSerialPortParam();
    void writePollSerialPortParam(const QVariant &);
private:
    modbus_t *m_modBusCtx = nullptr;
    QString m_connectType;
    int m_timerId = 0;
    int m_alarmTimerId = 0;

    int m_timeOut = 0;
    int m_slave = 0;
    int m_readCount = 0;
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
        WRITE = 1,
        ONLYREAD
    };

    enum ParamType{
        PollSysParam = 0,   //主机系统参数
        PollNetParam = 1,   //主机网络参数
        PollSpParam =  2    //主机串口参数
    };

public:
    //主机参数
    void pollParam(OperationType opType,ParamType paramType,const QVariant &param);

    //根据地址读取数据
    void readModBusRegister(int slave,int addr,int readCount,bool readOnly = false);
    //根据地址写入数据
    void writeModBusRegister(int slave,int addr,const QVector<quint16> &value);

    //读取连续数据
    void readContinuData(int slave ,int dataType,int timeOut = 60000);
    //启动报警信息监测
    void startAlarmCycle();
    //请求分机的只读参数
    void readSlaveReadOnlyData(int slave, QVariantHash &value);

signals:
    //读取到主机数据
    void signalPollParam(quint8 type,const QVariant &);

    //读取的数据
    void signalReadValue(int slave,int addr,const QByteArray &);
    //读取到的连续数据
    void signalReadContinuData(int type,const QByteArray &value);
    //报警信息
    void signalAlarmInfo(const QVariant &);

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
