#pragma once

#include <QChartView>


QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QLineSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE


class DeviceChartView : public QChartView
{

public:
    DeviceChartView(QWidget *parent = nullptr);



public slots:
    void onRenderView(int index, const QByteArray &value);
    //控制曲线图显示
    void setLineSeriesVisible(int index ,bool visible);

private:
    void initUi();

private:
    QChart *m_chart;
    QVector<QLineSeries*> m_lineSeries;
};

