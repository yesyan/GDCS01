#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QObject>
#include <QDateTime>

static const QString HardwareVersion = QStringLiteral("硬件版本");

static const QString SoftwareVersion = QStringLiteral("软件版本");

static const QString DeviceID = QStringLiteral("设备ID");

static const QString DeviceType = QStringLiteral("设备型号");

static const QString SysDateTime = QStringLiteral("系统时间");

static const QString DevParam  = QStringLiteral("设备参数");


//主机系统参数
struct PollSysParam{

public:
    QString devId;
    QString softVersion;
    QString hardVersion;
    QString sysDataTime;
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
    quint16 baud;
    quint16 cycleSize;//轮询设备长度
    quint16  param;
};
Q_DECLARE_METATYPE(PollSerialPortParam);


#endif // GLOBAL_H
