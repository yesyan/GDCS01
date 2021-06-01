#include "modbusobj.h"
#include "modbus-tcp.h"
#include "global.h"

#include <QDebug>

ModBusObj::ModBusObj(QObject *parent)
          :QObject(parent)
{

}

ModBusObj::~ModBusObj()
{

}

int ModBusObj::netWorkConnect(const QString &ip, int port)
{
    //先断开连接
    stopConnect();

    m_modBusCtx = modbus_new_tcp(ip.toStdString().data(),port);
    if(nullptr == m_modBusCtx){
        qInfo() << QStringLiteral("创建网络连接失败.");
        return -1;
    }

    auto ret = modbus_connect(m_modBusCtx);
    if(ret == -1){
        qInfo() << QStringLiteral("连接分机失败.");
        return ret;
    }
    //响应超时时间为1000毫秒
    ret = modbus_set_response_timeout(m_modBusCtx,1,0);
    if(ret == -1){
        qInfo() << QStringLiteral("设置响应超时时间失败.");
        return ret;
    }
    m_connectType = "network";
    return ret;
}

int ModBusObj::serialPortConnect(const QString &dev, int baud, char parity, int dataBit, int stopBit)
{
    //先断开连接
    stopConnect();

    m_modBusCtx = modbus_new_rtu(dev.toStdString().data(),baud,parity,dataBit,stopBit);
    if(nullptr == m_modBusCtx){
        qInfo() << QStringLiteral("创建串口连接失败.");
        return -1;
    }

    auto ret = modbus_connect(m_modBusCtx);
    if(ret == -1){
        qInfo() << QStringLiteral("连接分机失败.");
        return ret;
    }

    //响应超时时间为1000毫秒
    ret = modbus_set_response_timeout(m_modBusCtx,1,0);
    if(ret == -1){
        qInfo() << QStringLiteral("设置响应超时时间失败.");
        return ret;
    }
    m_connectType = "serialPort";
    return ret;
}

int ModBusObj::getConnectSlaveAddr()
{
    if(m_modBusCtx)
        return modbus_get_slave(m_modBusCtx);
    return -1;
}

int ModBusObj::getSlaveSimpleInfo(int slave, QVariantHash &value)
{
    if(nullptr == m_modBusCtx)
        return -1;
    auto ret = modbus_set_slave(m_modBusCtx,slave);
    if(ret == -1){
        qInfo() << QStringLiteral("设置分机地址失败.");
        return ret;
    }

    QVector<uint16_t> tmpData;
    tmpData.resize(29);
    ret = modbus_read_input_registers(m_modBusCtx,300,29,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取分机(%1)系统参数失败.").arg(slave);
        return ret;
    }
    //硬件版本
    value.insert(HardwareVersion,tmpData[0]);
    //软件版本
    value.insert(SoftwareVersion,tmpData[1]);
    auto devId = (tmpData[2] << 16) & 0xff;
    devId |= (tmpData[3] & 0xff);
    //设备id
    value.insert(DeviceID,devId);
    //设备型号
    QByteArray devType;
    devType.resize(20);
    memcpy(devType.data(),&tmpData[5],20);
    value.insert(DeviceType,devType);

    //供电方式
    switch (tmpData[27])
    {
    case 1:
        value.insert(PowerMode,QStringLiteral("外部供电"));
        break;
    case 2:
        value.insert(PowerMode,QStringLiteral("电池供电"));
        break;
    case 3:
        value.insert(PowerMode,QStringLiteral("正在充电"));
        break;
    default:
        value.insert(PowerMode,QStringLiteral("未知"));
        break;
    }

    //电量
    value.insert(ElectricQuantity,tmpData[28]);
    return 0;
}

int ModBusObj::getSlaveDevParam(int slave, QVariantHash &value)
{
    if(nullptr == m_modBusCtx)
        return -1;

    auto ret = modbus_set_slave(m_modBusCtx,slave);
    if(ret == -1){
        qInfo() << QStringLiteral("设置分机地址失败.");
        return ret;
    }

    //系统时间
    QVector<uint16_t> tmpData;
    tmpData.resize(3);
    ret = modbus_read_registers(m_modBusCtx, 185, 3, tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("获取系统时间失败.");
        return ret;
    }
    value.insert(SysDateTime,QString("20%1-%2-%3 %4:%5:%6").arg(tmpData[0]>>8).arg(tmpData[0]&0x00ff).arg(tmpData[1]>>8)
                                      .arg(tmpData[1]&0x00ff).arg(tmpData[2]>>8).arg(tmpData[2]&0x00ff));

    //设备参数
    tmpData.fill(0,11);
    ret = modbus_read_registers(m_modBusCtx, 128, 11, tmpData.data());
    DeviceParam devParam;
    devParam.xAcc = tmpData[0];
    devParam.xSpe = tmpData[1];
    devParam.xAmp = tmpData[2];
    devParam.yAcc = tmpData[3];
    devParam.ySpe = tmpData[4];
    devParam.yAmp = tmpData[5];
    devParam.zAcc = tmpData[6];
    devParam.zSpe = tmpData[7];
    devParam.zAmp = tmpData[8];
    devParam.tmp  = tmpData[9];
    devParam.JGtime = tmpData[10];
    value.insert(DevParam,QVariant::fromValue(devParam));

    return 0;
}

int ModBusObj::setSysTime(const QDateTime &)
{
    return 0;
}

int ModBusObj::stopConnect()
{
    if(nullptr != m_modBusCtx){
        modbus_close(m_modBusCtx);
//        modbus_free(m_modBusCtx);
        m_modBusCtx = nullptr;
    }
    return 0;
}

int ModBusObjInstance::connectToNet(const QString &ip, int port)
{
    return m_modBusObj->netWorkConnect(ip,port);
}

int ModBusObjInstance::connectToSerialPort(const QString &dev, int baud, char parity, int dataBit, int stopBit)
{
    return m_modBusObj->serialPortConnect(dev,baud,parity,dataBit,stopBit);
}

ModBusObjInstance::ModBusObjInstance()
{
    m_modBusObj = new ModBusObj;
    m_modBusObj->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_modBusObj, &QObject::deleteLater);
    m_thread.start();
}

ModBusObjInstance::~ModBusObjInstance()
{
    m_thread.quit();
    m_thread.wait();
}
