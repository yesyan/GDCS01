#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QObject>

static const QString HardwareVersion = QStringLiteral("硬件版本");

static const QString SoftwareVersion = QStringLiteral("软件版本");

static const QString DeviceID = QStringLiteral("设备ID");

static const QString DeviceType = QStringLiteral("设备型号");

static const QString PowerMode  = QStringLiteral("供电方式");

static const QString ElectricQuantity = QStringLiteral("电量");

static const QString SysDateTime = QStringLiteral("系统时间");

static const QString DevParam  = QStringLiteral("设备参数");

//设备参数
class DeviceParam{

public:
    uint16_t xAcc = 0;
    uint16_t xSpe = 0;
    uint16_t xAmp = 0;
    uint16_t yAcc = 0;
    uint16_t ySpe = 0;
    uint16_t yAmp = 0;
    uint16_t zAcc = 0;
    uint16_t zSpe = 0;
    uint16_t zAmp = 0;
    uint16_t tmp = 0;
    uint16_t JGtime = 0;
};
Q_DECLARE_METATYPE(DeviceParam);

#endif // GLOBAL_H
