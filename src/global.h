#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QObject>
#include <QDateTime>

static const QString HardwareVersion = QStringLiteral("硬件版本");

static const QString SoftwareVersion = QStringLiteral("软件版本");

static const QString DeviceID = QStringLiteral("设备ID");

static const QString X_AccSpeed = QStringLiteral("X轴加速度");

static const QString X_Speed = QStringLiteral("X轴速度");

static const QString X_Displace  = QStringLiteral("X轴位移");

static const QString Y_AccSpeed = QStringLiteral("Y轴加速度");

static const QString Y_Speed = QStringLiteral("Y轴速度");

static const QString Y_Displace  = QStringLiteral("Y轴位移");

static const QString Z_AccSpeed = QStringLiteral("Z轴加速度");

static const QString Z_Speed = QStringLiteral("Z轴速度");

static const QString Z_Displace  = QStringLiteral("Z轴位移");

static const QString Slave_Temp = QStringLiteral("温度");

static const QString Slave_Temp_Alarm  = QStringLiteral("温度报警");


//主机系统参数
struct PollSysParam{

public:
    QString devId;
    QString softVersion;
    QString hardVersion;
    quint16 sysDataTime1;
    quint16 sysDataTime2;
    quint16 sysDataTime3;
};
Q_DECLARE_METATYPE(PollSysParam);


//主机网络参数
struct PollNetParam{

public:
    QString  localIP;
    quint16  localPort = 0;
    QString  remoteIP;
    quint16  remotePort = 0;
    QString  dns;
    QString  gateWay;
};
Q_DECLARE_METATYPE(PollNetParam);


//主机串口参数
struct PollSerialPortParam{

public:
    quint32 baud;
    quint16 cycleSize;//轮询设备长度
    quint16  param;
};
Q_DECLARE_METATYPE(PollSerialPortParam);


#endif // GLOBAL_H
