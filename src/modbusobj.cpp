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

bool ModBusObj::getSlaveSysParam(int slave, QVariantHash &value)
{
    if(nullptr == m_modBusCtx)
        return false;
    auto ret = modbus_set_slave(m_modBusCtx,slave);
    if(ret == -1){
        qInfo() << QStringLiteral("设置分机地址失败.");
        return false;
    }
    int readAddr = 100*slave + 0;
    QVector<uint16_t> tmpData;
    tmpData.resize(4);
    ret = modbus_read_input_registers(m_modBusCtx,readAddr,4,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取分机(%1)系统参数失败.").arg(slave);
        return false;
    }

    //硬件版本
    value.insert(HardwareVersion,QString::number(tmpData[0]));
    //软件版本
    value.insert(SoftwareVersion,QString::number(tmpData[1]));
    //设备ID信息
    value.insert(DeviceID,QString::number(tmpData[2]) + QString::number(tmpData[3]));

    return true;
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

    return 0;
}

void ModBusObj::pollParam(quint8 opType, quint8 paramType, const QVariant &param)
{
    if(0 == opType && 0 == paramType){
        readPollSysParam();

    }else if(0 == opType && 1 == paramType){
        //读取主机网路参数
        readPollNetParam();

    }else if(1 == opType && 1 == paramType){
        //写入主机网络参数
        writePollNetParam(param);

    }else if(0 == opType && 2 == paramType){
        //读取主机串口参数
        readPollSerialPortParam();

    }else if(1 == opType && 2 == paramType){
        //写入主机串口数据
        writePollSerialPortParam(param);

    }
}

void ModBusObj::readSlaveParam(int slave, quint8 paramType)
{
    if(paramType == 3){
        //从机系统参数
        QVariantHash tValue;
        if(getSlaveSysParam(slave,tValue))
            emit signalSlaveParam(slave,3,tValue);
    }
}

void ModBusObj::writeSlaveParam(int slave, quint8 paramType, const QVariant &param)
{

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

void ModBusObj::readPollSysParam()
{
    QVector<uint16_t> tmpData;
    tmpData.resize(4);
    auto ret = modbus_read_input_registers(m_modBusCtx,0,4,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取主机系统参数失败.");
        return;
    }
    auto devId = (tmpData[2] << 16) & 0xff;
    devId |= (tmpData[3] & 0xff);

    PollSysParam sysParam;
    sysParam.hardVersion = tmpData[0];
    sysParam.softVersion = tmpData[1];
    sysParam.devId = devId;

    tmpData.fill(0);
    ret = modbus_read_input_registers(m_modBusCtx,20,3,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取系统时间失败.");
        return;
    }
    sysParam.sysDataTime = QStringLiteral("%1年%2月%3日%4时%5分%6秒").arg(tmpData[0]>>8 & 0xff).arg(tmpData[0] & 0xff)\
                                                                   .arg(tmpData[1]>>8 & 0xff).arg(tmpData[1] & 0xff)\
                                                                   .arg(tmpData[2]>>8 & 0xff).arg(tmpData[2] & 0xff);

    emit signalPollParam(0,QVariant::fromValue(sysParam));
}

void ModBusObj::readPollNetParam()
{
    QVector<uint16_t> tmpData;
    tmpData.resize(13);

    //本地ip、网关、DNS、本地端口
    auto ret = modbus_read_registers(m_modBusCtx,0,13,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取主机网络参数失败.");
    }
    PollNetParam pollNetParam;
    pollNetParam.localIP = QString("%1.%2.%3.%4").arg(tmpData[0]).arg(tmpData[1]).arg(tmpData[2]).arg(tmpData[3]);
    pollNetParam.gateWay = QString("%1.%2.%3.%4").arg(tmpData[4]).arg(tmpData[5]).arg(tmpData[6]).arg(tmpData[7]);
    pollNetParam.dns     = QString("%1.%2.%3.%4").arg(tmpData[8]).arg(tmpData[9]).arg(tmpData[10]).arg(tmpData[11]);
    pollNetParam.localPort = tmpData[12];
    tmpData.fill(0);

    ret = modbus_read_registers(m_modBusCtx,26,5,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取主机网络参数失败.");
    }
    pollNetParam.remoteIP   = QString("%1.%2.%3.%4").arg(tmpData[0]).arg(tmpData[1]).arg(tmpData[2]).arg(tmpData[3]);
    pollNetParam.remotePort = tmpData[4];

    emit signalPollParam(1,QVariant::fromValue(pollNetParam));
}

void ModBusObj::writePollNetParam(const QVariant &value)
{
}

void ModBusObj::readPollSerialPortParam()
{
    QVector<uint16_t> tmpData;
    tmpData.resize(4);

    auto ret = modbus_read_registers(m_modBusCtx,13,4,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取主机网络参数失败.");
    }
    PollSerialPortParam pollSpParam;
    auto baud = (tmpData[0] << 16) & 0xff;
    baud |= (tmpData[1] & 0xff);

    pollSpParam.baud = baud;
    pollSpParam.param = tmpData[2];
    pollSpParam.cycleSize = tmpData[3];

    emit signalPollParam(2,QVariant::fromValue(pollSpParam));

}

void ModBusObj::writePollSerialPortParam(const QVariant &value)
{

}

int ModBusObjInstance::connectToNet(const QString &ip, int port)
{
    return m_modBusObj->netWorkConnect(ip,port);
}

int ModBusObjInstance::connectToSerialPort(const QString &dev, int baud, char parity, int dataBit, int stopBit)
{
    return m_modBusObj->serialPortConnect(dev,baud,parity,dataBit,stopBit);
}

void ModBusObjInstance::pollParam(ModBusObjInstance::OperationType opType, ModBusObjInstance::ParamType paramType, const QVariant &param)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"pollParam",Qt::AutoConnection,
                              Q_ARG(quint8, opType),Q_ARG(quint8, paramType),
                              Q_ARG(const QVariant &,param));
}

void ModBusObjInstance::readSlaveParam(int slave, ModBusObjInstance::ParamType paramType)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"readSlaveParam",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(quint8, paramType));

}

void ModBusObjInstance::writeSlaveParam(int slave, ModBusObjInstance::ParamType paramType, const QVariant &value)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"writeSlaveParam",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(quint8, paramType),
                              Q_ARG(const QVariant &,value));

}

ModBusObjInstance::ModBusObjInstance()
{
    m_modBusObj = new ModBusObj;
    m_modBusObj->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_modBusObj, &QObject::deleteLater);

    //主机回传参数
    connect(m_modBusObj,&ModBusObj::signalPollParam,this,&ModBusObjInstance::signalPollParam);
    //分机回传参数
    connect(m_modBusObj,&ModBusObj::signalSlaveParam,this,&ModBusObjInstance::signalSlaveParam);

    m_thread.start();
}

ModBusObjInstance::~ModBusObjInstance()
{
    m_thread.quit();
    m_thread.wait();
}
