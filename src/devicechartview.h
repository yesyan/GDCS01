#pragma once

#include <QChartView>


QT_CHARTS_BEGIN_NAMESPACE
class QChart;
class QLineSeries;
class QValueAxis;
class QLogValueAxis;
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

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    void initUi();

private:
    QChart *m_chart;
    QVector<QLineSeries*> m_lineSeries;

    bool m_isClicking = false;
    int m_xOld = 0;
    int m_yOld = 0;

    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;
};

