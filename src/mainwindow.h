#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DeviceWidget;
class QTableWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onSlaveSimpleInfo(const QVariantHash &value);
private:
    void initUi();
    //检测分机状态
    void checkSlaveStatus();
    //读取主机参数
    void loadPollParam();

private:
    Ui::MainWindow *ui;

    DeviceWidget *m_devWid = nullptr;
    QTableWidget *m_slaveTableWidget = nullptr;
};
#endif // MAINWINDOW_H
