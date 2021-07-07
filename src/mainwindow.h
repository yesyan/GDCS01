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
    void onRecvCycleInfo(const QVariant &value);

private:
    void initUi();

private:
    Ui::MainWindow *ui;

    DeviceWidget *m_devWid = nullptr;
};
#endif // MAINWINDOW_H
