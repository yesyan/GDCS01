#include "devicechartview.h"
#include <QChart>
#include <QLineSeries>
#include <QTime>

#include "modbusobj.h"

DeviceChartView::DeviceChartView(QWidget *parent)
                :QChartView(parent),m_chart(new QChart)
{
    initUi();
    connect(ModBusObjInstance::getInstance(),&ModBusObjInstance::signalReadContinuData,this,&DeviceChartView::onRenderView);

}

void DeviceChartView::initUi()
{
    m_chart->setTitle(QStringLiteral("设备曲线图"));

    //创建曲线图
    //x轴
    auto series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("X加速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("X速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);


    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("X位移"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    //y轴
    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Y加速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Y速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Y位移"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    //z轴
    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Z加速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Z速度"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("Z位移"));
    m_chart->addSeries(series);
    m_lineSeries.append(series);

    m_chart->createDefaultAxes();
    this->setChart(m_chart);
}

void DeviceChartView::onRenderView(int index, const QByteArray &value)
{
    if(index < m_lineSeries.count()){
        auto lineSeries = m_lineSeries[index];
        lineSeries->clear();

        QVector<quint16> tValue;
        tValue.resize(value.size()/sizeof(quint16));
        memcpy(tValue.data(),value.data(),value.size());

        for(auto index = 0 ;index < tValue.size() ; ++index){
            lineSeries->append(index,tValue[index]);
        }
    }
}

void DeviceChartView::setLineSeriesVisible(int index, bool visible)
{
    if(index < m_lineSeries.count()){
        auto lineSeries = m_lineSeries[index];
        lineSeries->setVisible(visible);
    }
}
