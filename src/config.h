#pragma once

#include <QVariantHash>

QT_BEGIN_HEADER
class QString;
class QStringList;
class QJsonValue;
QT_END_HEADER

class Json;
/**
 * 用于读写配置文件:
 * 1. 配置文件位于: config/config.json，存储配置的信息，例如数据库信息，QSS 文件的路径
 * 2. 读取配置，如 Config::getInstance().getDatabaseName();
 */
class  Config {

public:
    static Config* getInstance()
    {
        static Config instance;
        return &instance;
    }

    //!
    //! \brief 销毁 Config 的资源，如有必要，在 main 函数结束前调用，例如保存配置文件
    //!
    void destroy();

public:

    //存储连接参数
    void saveConnectType(bool isNet);
    void saveSpConnectParam(const QVariantHash &value);
    void saveNtConnectParam(const QVariantHash &value);

    //获取连接参数
    bool getConnectType();
    QVariantHash getSpConnectParam();
    QVariantHash getNtConnectParam();

private:
    Config();
    ~Config();
    Config(const Config& rParam) = delete;
    const Config& operator = (const Config& rParam) = delete;
    Json *json;
};
