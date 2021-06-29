#include "modbusobj.h"
#include "modbus-tcp.h"
#include "global.h"

#include <QDebug>
#include <QTimerEvent>

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
        qInfo() << QStringLiteral("网络连接失败.");
        return ret;
    }

    ret = modbus_set_slave(m_modBusCtx,1);
    if(ret == -1){
        qInfo() << QStringLiteral("设置分机ID失败.");
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

    ret = modbus_set_slave(m_modBusCtx,1);
    if(ret == -1){
        qInfo() << QStringLiteral("设置分机ID失败.");
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

    int readAddr = 100*slave + 0;
    QVector<uint16_t> tmpData;
    tmpData.resize(4);
    auto ret = modbus_read_input_registers(m_modBusCtx,readAddr,4,tmpData.data());
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

void ModBusObj::pollParam(quint8 opType, quint8 paramType, const QVariant &param)
{
    if(0 == opType && 0 == paramType){
        //读取主机系统参数
        readPollSysParam();

    }else if(1 == opType && 0 == paramType){
        //写入系统时间
        auto sysParam = param.value<PollSysParam>();
        QVector<uint16_t> tmpData;
        tmpData.append(sysParam.sysDataTime1);
        tmpData.append(sysParam.sysDataTime2);
        tmpData.append(sysParam.sysDataTime3);

        auto ret = modbus_write_registers(m_modBusCtx,20,3,tmpData.data());
        if(ret == -1){
            qInfo() << QStringLiteral("写入系统时间失败.");
        }
        //配置生效
        uint16_t tValue = 1;
        modbus_write_registers(m_modBusCtx,19,1,&tValue);

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

void ModBusObj::readModBusRegister(int slave, int addr, int readCount, bool readOnly)
{
    if(nullptr == m_modBusCtx)
        return;

    QVector<quint16> tmpData;
    tmpData.resize(readCount);

    if(readOnly){
        auto ret = modbus_read_input_registers(m_modBusCtx, slave*100+addr, readCount, tmpData.data());
        if(ret == -1){
            qInfo() << QStringLiteral("读取分机(%1)参数失败.").arg(slave);
            return;
        }
    }else{
        auto ret = modbus_read_registers(m_modBusCtx, slave*100+addr, readCount, tmpData.data());
        if(ret == -1){
            qInfo() << QStringLiteral("读取分机(%1)参数失败.").arg(slave);
            return;
        }
    }
    QByteArray byteData;
    byteData.resize(readCount * sizeof (quint16));
    memcpy(byteData.data(),tmpData.data(),readCount * sizeof (quint16));

    emit signalReadValue(slave,addr, byteData);
}

void ModBusObj::writeModBusRegister(int slave, int addr, const QVector<quint16> &value)
{
    if(nullptr == m_modBusCtx)
        return;

    auto ret = modbus_write_registers(m_modBusCtx,slave*100+addr,value.size(),value.data());
    if(ret == -1){
        qInfo() << QStringLiteral("写入分机(%1)参数失败.").arg(slave);
        return;
    }
    //配置生效
    uint16_t tValue = 1;
    modbus_write_registers(m_modBusCtx,slave*100+0,1,&tValue);

}

void ModBusObj::readContinuData(int slave,int dataType,int timeOut)
{
    if(nullptr == m_modBusCtx)
        return;

    //选择数据
    modbus_write_register(m_modBusCtx, slave*100+1, dataType);
    //请求连续数据
    uint16_t tValue = 1;
    modbus_write_register(m_modBusCtx, slave*100+30, tValue);

    m_readCount = 0;
    m_slave = slave;
    m_timeOut = timeOut;

    m_timerId = startTimer(1000);
}

void ModBusObj::timerEvent(QTimerEvent *event)
{
    if(m_timerId == event->timerId()){

        if(m_readCount >= (m_timeOut/1000)){
            qInfo() << QStringLiteral("读取分机(%1)连续数据超时.").arg(m_slave);
            killTimer(m_timerId);
            return;
        }

        QVector<uint16_t> statusValue;
        statusValue.resize(1);
        modbus_read_registers(m_modBusCtx, m_slave*100+30, 1, statusValue.data());

        //轮询连续数据状态(1:表示采集连续数据 0:连续数据采集完成 2:连续数据采集失败)
        if(statusValue[0] == 1){
            ++m_readCount;

        }else if(statusValue[0] == 2){
            qInfo() << QStringLiteral("读取分机(%1)连续数据采集失败.").arg(m_slave);
            killTimer(m_timerId);
            return;

        }else if(statusValue[0] == 0){

            //读取连续震动采集长度
            uint16_t tValueSize;
            auto ret = modbus_read_input_registers(m_modBusCtx,m_slave*100+23,1,&tValueSize);
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续震动采集长度失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }
            QVector<uint16_t> tmpData;
            tmpData.resize(tValueSize/2);
            ret = modbus_read_input_registers(m_modBusCtx, 4000, tValueSize, tmpData.data());
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续数据失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }

            //连续数据选择（为何种数据）
            uint16_t tValueType = 0;
            ret = modbus_read_input_registers(m_modBusCtx,m_slave*100+1,1,&tValueType);
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续数据选择失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }

            QByteArray byteData;
            byteData.resize(tValueSize);
            memcpy(byteData.data(),tmpData.data(),tValueSize);

            emit signalContinuData(tValueType,byteData);
            killTimer(m_timerId);
        }

    }
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
    sysParam.hardVersion = QString::number(tmpData[0]);
    sysParam.softVersion = QString::number(tmpData[1]);
    sysParam.devId = QString::number(devId);

    tmpData.fill(0);
    ret = modbus_read_input_registers(m_modBusCtx,20,3,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取系统时间失败.");
    }
    sysParam.sysDataTime1 = tmpData[0];
    sysParam.sysDataTime2 = tmpData[1];
    sysParam.sysDataTime2 = tmpData[2];

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

void ModBusObj::writePollNetParam(const QVariant &param)
{
    auto netParam = param.value<PollNetParam>();
    QVector<uint16_t> tmpData;
    //本地ip
    auto localIp = netParam.localIP.split('.');
    for(auto item : localIp){
        tmpData.append(item.toUShort());
    }
    //网关
    auto gateWay = netParam.gateWay.split('.');
    for(auto item : gateWay){
        tmpData.append(item.toUShort());
    }
    //DNS
    auto dns = netParam.dns.split('.');
    for(auto item : dns){
        tmpData.append(item.toUShort());
    }
    //本地端口
    tmpData.append(netParam.localPort);
    auto ret = modbus_write_registers(m_modBusCtx,0,13,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("写入主机网络参数失败.");
    }

    QVector<uint16_t> tmpDataNext;
    //远端ip
    auto remoteIp= netParam.remoteIP.split('.');
    for(auto item : remoteIp){
        tmpDataNext.append(item.toUShort());
    }
    //远端端口
    tmpDataNext.append(netParam.remotePort);
    ret = modbus_write_registers(m_modBusCtx,26,5,tmpDataNext.data());
    if(ret == -1){
        qInfo() << QStringLiteral("写入主机网络参数失败.");
    }

    //配置生效
    uint16_t tValue = 1;
    modbus_write_registers(m_modBusCtx,19,1,&tValue);

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
    auto baud = (tmpData[0] << 16);
    baud |= (tmpData[1] & 0xffff);

    pollSpParam.baud = quint32(baud);
    pollSpParam.param = tmpData[2];
    pollSpParam.cycleSize = tmpData[3];

    emit signalPollParam(2,QVariant::fromValue(pollSpParam));

}

void ModBusObj::writePollSerialPortParam(const QVariant &param)
{

    auto spParam = param.value<PollSerialPortParam>();
    QVector<uint16_t> tmpData;
    tmpData.append((spParam.baud >> 16) & 0xffff);
    tmpData.append(spParam.baud & 0xffff);
    tmpData.append(spParam.param);
    tmpData.append(spParam.cycleSize);

    auto ret = modbus_write_registers(m_modBusCtx,13,4,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("写入主机串口参数失败.");
    }
    //配置生效
    uint16_t tValue = 1;
    modbus_write_registers(m_modBusCtx,19,1,&tValue);

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

void ModBusObjInstance::readModBusRegister(int slave, int addr, int readCount, bool readOnly)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"readModBusRegister",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(int, addr),Q_ARG(int, readCount),
                              Q_ARG(bool,readOnly));
}

void ModBusObjInstance::writeModBusRegister(int slave, int addr, const QVector<quint16> &value)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"writeModBusRegister",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(int, addr),
                              Q_ARG(const QVector<quint16> &,value));

}

void ModBusObjInstance::readContinuData(int slave, int dataType, int timeOut)
{
    if(nullptr == m_modBusObj)
        return;
    QMetaObject::invokeMethod(m_modBusObj,"readContinuData",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(int,dataType),Q_ARG(int,timeOut));
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
    //读取的值
    connect(m_modBusObj,&ModBusObj::signalReadValue,this,&ModBusObjInstance::signalReadValue);

    //读取的连续数据
    connect(m_modBusObj,&ModBusObj::signalContinuData,this,&ModBusObjInstance::signalReadContinuData);

    m_thread.start();
}

ModBusObjInstance::~ModBusObjInstance()
{
    m_thread.quit();
    m_thread.wait();
}
