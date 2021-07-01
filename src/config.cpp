#include "config.h"
#include "json.h"

#include <QString>
#include <QStringList>
#include <QCoreApplication>


Config::Config() {
    json = new Json(qApp->applicationDirPath() + "/config/config.json", true); // 配置文件路径
}

Config::~Config() {
    destroy();
}

void Config::destroy() {
    delete json;
    json = Q_NULLPTR;
}

void Config::saveConnectType(bool isNet)
{
    json->set("NetConnect",QJsonValue(isNet));
    json->save(qApp->applicationDirPath() + "/config/config.json");

}

void Config::saveSpConnectParam(const QVariantHash &value)
{
    auto jsonObject = QJsonObject::fromVariantHash(value);
    json->set("SpParam",jsonObject);
    json->save(qApp->applicationDirPath() + "/config/config.json");
}

void Config::saveNtConnectParam(const QVariantHash &value)
{
    auto jsonObject = QJsonObject::fromVariantHash(value);
    json->set("NtParam",jsonObject);
    json->save(qApp->applicationDirPath() + "/config/config.json");

}

bool Config::getConnectType()
{
    return json->getBool("NetConnect");
}

QVariantHash Config::getSpConnectParam()
{
    return json->getJsonObject("SpParam").toVariantHash();
}

QVariantHash Config::getNtConnectParam()
{
    return json->getJsonObject("NtParam").toVariantHash();
}
