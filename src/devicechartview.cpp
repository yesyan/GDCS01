#include "devicechartview.h"
#include <QChart>
#include <QLineSeries>
#include <QTime>
#include <QtEndian>
#include <QValueAxis>
#include <QLogValueAxis>

#include "modbusobj.h"

DeviceChartView::DeviceChartView(QWidget *parent)
                :QChartView(parent),m_chart(new QChart)
{
    initUi();
    connect(ModBusObjInstance::getInstance()->getModBusObj(),&ModBusObj::signalContinuData,this,&DeviceChartView::onRenderView);

    setRubberBand(QChartView::RectangleRubberBand);
    //设置反走样
    setRenderHint(QPainter::Antialiasing);
}

void DeviceChartView::initUi()
{
    m_chart->setTitle(QStringLiteral("设备曲线图"));
    m_chart->setAnimationOptions(QChart::AllAnimations);

    //X坐标轴
    m_axisX = new QValueAxis();
    m_axisX->setTitleText(QStringLiteral("个数"));
    m_axisX->setLabelFormat("%d");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    //Y坐标轴
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Values");
    m_axisY->setLabelFormat("%d");
    m_axisY->setRange(-10,10);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);


    //创建曲线图
    //x轴
    auto series = new QLineSeries(m_chart);
    series->setName(QStringLiteral("X加速度"));
    m_chart->addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);
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

    this->setChart(m_chart);
}

void DeviceChartView::onRenderView(int index, const QByteArray &value)
{
    if(index < m_lineSeries.count()){
        auto lineSeries = m_lineSeries[index];
        lineSeries->clear();

        QVector<qint16> tValue;
        tValue.resize(value.size()/sizeof(qint16));
        memcpy(tValue.data(),value.data(),value.size());

        for(auto index = 0 ;index < tValue.size() ; ++index){
            //lineSeries->append(index,qFromBigEndian<quint16>(tValue[index]));
            lineSeries->append(index,tValue[index]);
        }
        m_axisX->setMax(tValue.count());
    }
}

void DeviceChartView::setLineSeriesVisible(int index, bool visible)
{
    if(index < m_lineSeries.count()){
        auto lineSeries = m_lineSeries[index];
        lineSeries->setVisible(visible);
    }
}

void DeviceChartView::mouseMoveEvent(QMouseEvent *event)
{
    int x, y;

    if (m_isClicking) {
        if (m_xOld == 0 && m_yOld == 0) {

        } else {
            x = event->x() - m_xOld;
            y = event->y() - m_yOld;
            chart()->scroll(-x, y);
        }

        m_xOld = event->x();
        m_yOld = event->y();

        return;
    }

    QChartView::mouseMoveEvent(event);
}

void DeviceChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isClicking) {
        m_xOld = m_yOld = 0;
        m_isClicking = false;
    }

    /* Disable original right click event */
    if (!(event->button() & Qt::RightButton)) {
        QChartView::mouseReleaseEvent(event);
    }
}

void DeviceChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        m_isClicking = true;
    } else if (event->button() & Qt::RightButton) {
        chart()->zoomReset();
    }

    QChartView::mousePressEvent(event);

}

void DeviceChartView::wheelEvent(QWheelEvent *event)
{
    for(QAbstractSeries *series : this->chart()->series())
    {
        bool minXok= false,minYok= false,maxXok= false,maxYok = false;
        qreal minX = this->chart()->axisX(series)->property("min").toReal(&minXok);
        qreal maxX = this->chart()->axisX(series)->property("max").toReal(&maxXok);
        qreal minY = this->chart()->axisY(series)->property("min").toReal(&minYok);
        qreal maxY = this->chart()->axisY(series)->property("max").toReal(&maxYok);
        if(minXok && minYok && maxXok && maxYok)
        {
            if(event->angleDelta().y() > 0 )
            {
                QPointF val= this->chart()->mapToValue(event->posF());
                qreal valX = val.x();
                qreal valY = val.y();

                qreal zoomFactor = 0.5;
                QPointF topLeftVal;
                QPointF buttomRightVal;
                if(event->modifiers() ==  Qt::ControlModifier)
                {
                    topLeftVal      = QPointF(zoomFactor*(valX + minX), maxY);
                    buttomRightVal  = QPointF(zoomFactor*(valX + maxX), minY);
                }
                else if(event->modifiers() ==  Qt::ShiftModifier)
                {
                    topLeftVal      = QPointF(minX,zoomFactor *(valY + maxY));
                    buttomRightVal  = QPointF(maxX,zoomFactor *(valY + minY));
                }
                else
                {
                    topLeftVal      = (val + QPointF(minX, maxY))*zoomFactor;
                    buttomRightVal  = (val + QPointF(maxX, minY))*zoomFactor;
                }
                QPointF topLeft     = this->chart()->mapToPosition(topLeftVal);
                QPointF buttomRight = this->chart()->mapToPosition(buttomRightVal);
                this->chart()->zoomIn(QRectF(topLeft,buttomRight));
            }
            else
            {
                this->chart()->zoomOut();
            }
            break;
        }
    }
    event->accept();
}
