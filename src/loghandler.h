#ifndef LOGHANDLER_H
#define LOGHANDLER_H

class LogHandlerPrivate;
class QTextEdit;

class  LogHandler
{
public:
    //!
    //! \brief 获取此单例的实例
    //! \return
    //!
    static LogHandler *getInstance()
    {
        static LogHandler instance;
        return &instance;
    }

   /**
     * @brief   释放资源
     */
    void uninstallMessageHandler();

    /**
     * @brief   给 Qt 安装消息处理函数
     */
    void installMessageHandler();

    /**
     * @brief   获取日志显示视图
     */
    QTextEdit* getTextView();
private:
    LogHandler();
    ~LogHandler();
    LogHandler(const LogHandler& rParam) = delete;
    const LogHandler& operator = (const LogHandler& rParam) = delete;

private:
    LogHandlerPrivate *d;
};

#endif // LOGHANDLER_H
