#include "modbusobj.h"
#include "modbus-tcp.h"
#include "global.h"

#include <QDebug>
#include <QTimerEvent>
#include <QtMath>

static QString checkInfo(uint16_t value){

    return (value == 1 ? QStringLiteral("告警") : QStringLiteral("正常"));
}


ModBusObj::ModBusObj()
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

void ModBusObj::readPollParam(quint8 type)
{
    if(0 == type){

        //读取主机系统参数
        readPollSysParam();

    }else if(1 == type){

        //读取主机网路参数
        readPollNetParam();

    }else if(2 == type){

        //读取主机串口参数
        readPollSerialPortParam();
    }
}

void ModBusObj::writePollParam(quint8 type, const QVariant &value)
{
    if(0 == type){

        //写入系统时间
        auto sysParam = value.value<PollSysParam>();
        QVector<uint16_t> tmpData;
        tmpData.append(sysParam.sysDataTime1);
        tmpData.append(sysParam.sysDataTime2);
        tmpData.append(sysParam.sysDataTime3);

        auto ret = modbus_write_registers(m_modBusCtx,20,3,tmpData.data());
        if(ret == -1){
            qInfo() << QStringLiteral("主机写入系统时间失败.");
            return;
        }
        //配置生效
        uint16_t tValue = 1;
        modbus_write_registers(m_modBusCtx,19,1,&tValue);

        qInfo() << QStringLiteral("主机写入系统时间成功.");

    }else if(1 == type){

        //写入主机网络参数
        writePollNetParam(value);

    }else if(2 == type){

        //写入主机串口数据
        writePollSerialPortParam(value);
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
        }else{
             qInfo() << QStringLiteral("读取分机(%1)参数成功.").arg(slave);
        }
    }else{
        auto ret = modbus_read_registers(m_modBusCtx, slave*100+addr, readCount, tmpData.data());
        if(ret == -1){
            qInfo() << QStringLiteral("读取分机(%1)参数失败.").arg(slave);
            return;
        }else{
            qInfo() << QStringLiteral("读取分机(%1)参数成功.").arg(slave);
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
    }else{

        qInfo() << QStringLiteral("写入分机(%1)参数成功.").arg(slave);

    }
    //配置生效
    uint16_t tValue = 1;
    modbus_write_registers(m_modBusCtx,slave*100+0,1,&tValue);

}

void ModBusObj::readContinuData(int slave,int dataType,int timeOut)
{
    int rv = -1;
    if(nullptr == m_modBusCtx)
        return;

    //选择数据
    rv = modbus_write_register(m_modBusCtx, slave*100+1, dataType);
    if(rv == -1){
        qInfo() << QStringLiteral("配置从机(%1)连续数据类型失败.").arg(slave);
        return;
    }
    //请求连续数据
    uint16_t tValue = 1;
    rv = modbus_write_register(m_modBusCtx, slave*100+30, tValue);
    if(rv == -1){
        qInfo() << QStringLiteral("配置从机(%1)连续数据请求失败.").arg(slave);
        return;
    }

    m_readCount = 0;
    m_slave = slave;
    m_timeOut = timeOut;

    m_timerId = startTimer(1000);
}

void ModBusObj::startAlarmTimer()
{
    m_alarmTimerId = startTimer(1000);
}

void ModBusObj::readSlaveReadOnlyData(int slave)
{
    QVariantHash retValue;
    if(nullptr == m_modBusCtx)
        return;
    QVector<uint16_t> tValue;
    tValue.fill(0,12);
    auto ret = modbus_read_input_registers(m_modBusCtx, slave*100+4, 12, tValue.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取分机(%1)特征值失败").arg(slave);
        return;
    }

    for(auto index = 0 ; index < tValue.count(); ++index){
        retValue.insert(QString::number(index+4),tValue[index]);
    }
    tValue.fill(0,1);
    ret = modbus_read_input_registers(m_modBusCtx, slave*100+40, 1, tValue.data());
    retValue.insert(QString::number(40),tValue[0]);
    emit  signalSlaveFV(slave , retValue);
}

void ModBusObj::timerEvent(QTimerEvent *event)
{
    if(m_timerId == event->timerId()){

        if(m_readCount >= (m_timeOut/1000)){
            qInfo() << QStringLiteral("读取从机(%1)连续数据超时.").arg(m_slave);
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
            auto ret = modbus_read_registers(m_modBusCtx,m_slave*100+23,1,&tValueSize);
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续震动采集长度失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }
            QVector<uint16_t> tmpData;
            auto itemCount = tValueSize/sizeof (uint16_t);
            tmpData.resize(itemCount);
            ret = modbus_read_input_registers(m_modBusCtx, 4000, itemCount, tmpData.data());
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续数据失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }

            //连续数据选择（为何种数据）
            uint16_t tValueType = 0;
            ret = modbus_read_registers(m_modBusCtx,m_slave*100+1,1,&tValueType);
            if(ret == -1){
                qInfo() << QStringLiteral("读取分机(%1)连续数据选择失败.").arg(m_slave);
                killTimer(m_timerId);
                return;
            }

            QByteArray byteData;
            byteData.resize(tValueSize);
            memcpy(byteData.data(),tmpData.data(),tValueSize);

            auto index = qLn(tValueType)/qLn(2);
            emit signalContinuData(index,byteData);
            killTimer(m_timerId);
        }

    }else if(m_alarmTimerId == event->timerId()){

        QVariantList alarmInfo;
        for(auto slave = 1; slave < 31; ++slave){

            int readAddr = 100*slave + 0;
            QVector<uint16_t> tmpData;
            tmpData.fill(0,4);
            auto ret = modbus_read_input_registers(m_modBusCtx,readAddr,4,tmpData.data());
            if(ret == -1){
                alarmInfo.append(QVariant());
                continue;
            }

            QVariantHash singleValue;

            //if((tmpData[2]<<16 |tmpData[3])>0)
           // {
              //硬件版本
              QString hw_tmp,soft_tmp;
              hw_tmp.sprintf("v%d.%d.%d", tmpData[0] / 100, (tmpData[0] % 100) / 10, (tmpData[0] % 10));
              soft_tmp.sprintf("v%d.%d.%d", tmpData[1] / 100, (tmpData[1] % 100) / 10, (tmpData[1] % 10));
             singleValue.insert(HardwareVersion,QString::number(tmpData[0]));
              //软件版本
              singleValue.insert(SoftwareVersion,soft_tmp);
              //设备ID信息
              singleValue.insert(DeviceID,QString::number(tmpData[2]<<16 |tmpData[3]));
          //  }


            tmpData.fill(0,11);
            readAddr = 100*slave + 16;
            ret = modbus_read_input_registers(m_modBusCtx,readAddr,11,tmpData.data());
            if(ret == -1){
                alarmInfo.append(singleValue);
                continue;
            }

            singleValue.insert(X_AccSpeed,checkInfo(tmpData[0]));
            singleValue.insert(X_Speed,checkInfo(tmpData[1]));
            singleValue.insert(X_Displace,checkInfo(tmpData[2]));

            singleValue.insert(Y_AccSpeed,checkInfo(tmpData[3]));
            singleValue.insert(Y_Speed,checkInfo(tmpData[4]));
            singleValue.insert(Y_Displace,checkInfo(tmpData[5]));

            singleValue.insert(Z_AccSpeed,checkInfo(tmpData[6]));
            singleValue.insert(Z_Speed,checkInfo(tmpData[7]));
            singleValue.insert(Z_Displace,checkInfo(tmpData[8]));

            singleValue.insert(Slave_Temp,QString::number(tmpData[9]/10));
            singleValue.insert(Slave_Temp_Alarm,checkInfo(tmpData[10]));

            alarmInfo.append(singleValue);
        }
        emit signalAlarmInfo(alarmInfo);
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
    }
    auto devId = (tmpData[2] << 16) |(tmpData[3]);

    PollSysParam sysParam;
    QString hw_tmp,soft_tmp;
    hw_tmp.sprintf("v%d.%d.%d", tmpData[0] / 100, (tmpData[0] % 100) / 10, (tmpData[0] % 10));
    soft_tmp.sprintf("v%d.%d.%d", tmpData[1] / 100, (tmpData[1] % 100) / 10, (tmpData[1] % 10));
    sysParam.hardVersion = hw_tmp;//QString::number(tmpData[0]);
    sysParam.softVersion = soft_tmp;//QString::number(tmpData[1]);

    sysParam.devId = QString::number(devId);

    tmpData.fill(0);
    ret = modbus_read_registers(m_modBusCtx,20,3,tmpData.data());
    if(ret == -1){
        qInfo() << QStringLiteral("读取系统时间失败.");
    }
    sysParam.sysDataTime1 = tmpData[0];
    sysParam.sysDataTime2 = tmpData[1];
    sysParam.sysDataTime3 = tmpData[2];

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

    qInfo() << QStringLiteral("写入主机网络参数成功.");
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

    qInfo() << QStringLiteral("写入主机串口参数成功.");
}

int ModBusObjInstance::connectToNet(const QString &ip, int port)
{
    return m_modBusObj->netWorkConnect(ip,port);
}

int ModBusObjInstance::connectToSerialPort(const QString &dev, int baud, char parity, int dataBit, int stopBit)
{
    return m_modBusObj->serialPortConnect(dev,baud,parity,dataBit,stopBit);
}

void ModBusObjInstance::readPollParam(quint8 type)
{
    QMetaObject::invokeMethod(m_modBusObj,"readPollParam",Qt::AutoConnection,
                              Q_ARG(quint8, type));

}

void ModBusObjInstance::writePollParam(quint8 type, const QVariant &value)
{
    QMetaObject::invokeMethod(m_modBusObj,"writePollParam",Qt::AutoConnection,
                              Q_ARG(quint8, type),Q_ARG(const QVariant &,value));

}

void ModBusObjInstance::readModBusRegister(int slave, int addr, int readCount, bool readOnly)
{
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
    QMetaObject::invokeMethod(m_modBusObj,"readContinuData",Qt::AutoConnection,
                              Q_ARG(int, slave),Q_ARG(int,dataType),Q_ARG(int,timeOut));
}

void ModBusObjInstance::startAlarmCycle()
{
    QMetaObject::invokeMethod(m_modBusObj,"startAlarmTimer");
}

void ModBusObjInstance::readSlaveReadOnlyData(int slave)
{
    QMetaObject::invokeMethod(m_modBusObj,"readSlaveReadOnlyData",Q_ARG(int,slave));
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
