/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

// 订阅窗口
#include "subscriptionwindow.h"
#include "ui_subscriptionwindow.h"
#include <QtMath>
#include <QDateTime>


SubscriptionWindow::SubscriptionWindow(QMqttSubscription *sub, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubscriptionWindow),
    m_sub(sub)
{
    ui->setupUi(this);

    DrawBackground(ui->widget_V, "电压");
    DrawBackground(ui->widget_I, "电流");
    DrawBackground(ui->widget_P, "功率");
    DrawBackground(ui->widget_Freq, "频率");
    DrawBackground(ui->widget_Ep, "电量");

    // 主题
    ui->labelSub->setText(m_sub->topic().filter());
    // QOS
    ui->labelQoS->setText(QString::number(m_sub->qos()));
    updateStatus(m_sub->state());

    // 连接信号：消息读取
    connect(m_sub, &QMqttSubscription::messageReceived, this, &SubscriptionWindow::updateMessage);
    // 连接信号：状态更新
    connect(m_sub, &QMqttSubscription::stateChanged, this, &SubscriptionWindow::updateStatus);
    // 连接信号：QOS值
    connect(m_sub, &QMqttSubscription::qosChanged, [this](quint8 qos) {
        ui->labelQoS->setText(QString::number(qos));
    });
    // 连接信号：按键--退订
    connect(ui->pushButton, &QAbstractButton::clicked, m_sub, &QMqttSubscription::unsubscribe);
}

// 析构
SubscriptionWindow::~SubscriptionWindow()
{
    m_sub->unsubscribe();
    delete ui;
}

/*
 * @breif 将16进制字节序列 转换为 对应的字符串
 */
QString SubscriptionWindow::ByteArrayToHexString(QByteArray data)
{
    QString ret(data.toHex().toUpper());    // QByteArray转十六进制CString

    // 在数字间插入空格
    int len = ret.length()/2;
    for(int i = 1;i < len; i++) {
        ret.insert( 2*i + i - 1," ");
    }

    return ret;
}


float DataAnalysis(QByteArray list, int staetId)
{
    int i = (list[staetId + 0] << 24) + (list[staetId + 1] << 16) + (list[staetId + 2] << 8) + (list[staetId + 3]);
    //unsigned char buf[4];
    //buf[0] = list[staetId + 0];
    //buf[1] = list[staetId + 1];
    //buf[2] = list[staetId + 2];
    //buf[3] = list[staetId + 3];
    float *p = (float *)&i;
    float float_number = *p;

    return float_number;
}

// 数据解析
void SubscriptionWindow::AnalyticalData(QByteArray data)
{
    if(!((data.at(0) == 0x5A) && (data.at(1) == 0x5A) && (data.at(2) == 0x5A) && (data.at(3) == 0x5A))) {
        return;
    }

    QDateTime current_date_time =QDateTime::currentDateTime();
    //QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
    double data_time = current_date_time.toSecsSinceEpoch();

    if(data.at(4) == 0x02) {
        double V = (double)DataAnalysis(data,  8) * 100 / 100;      // 电压
        double I = (double)DataAnalysis(data, 12) * 1000 / 1000;    // 电流
        double P = (double)DataAnalysis(data, 16) * 1000 / 1000;    // 瞬时总有功功率
        double Q = (double)DataAnalysis(data, 20) * 1000 / 1000;    // 瞬时总无功功率
        double S = (double)DataAnalysis(data, 24) * 1000 / 1000;    // 瞬时总视在功率
        double PF = (double)DataAnalysis(data, 28) * 1000 / 1000;   // 总功功率因数
        double Freq = (double)DataAnalysis(data, 36) * 1000 / 1000; // 电网频率

        port1_x.append(data_time);
        port1_y_V.append(V);
        port1_y_I.append(I);
        port1_y_P.append(P);
        port1_y_Freq.append(Freq);

        Graph_Show(ui->widget_V, port1_x, port1_y_V);
        Graph_Show(ui->widget_I, port1_x, port1_y_I);
        Graph_Show(ui->widget_P, port1_x, port1_y_P);
        Graph_Show(ui->widget_Freq, port1_x, port1_y_Freq);

        QString str = QString("%1V %2A %3kW %4kW %5kW %6 %7Hz").arg(V).arg(I).arg(P).arg(Q).arg(S).arg(PF).arg(Freq);
        ui->listWidget->addItem(str);
    }
    else if(data.at(4) == 0x03) {
        double ImpEp = (double)DataAnalysis(data, 8) * 1000 / 1000;

        port2_x.append(data_time);
        port2_y_Eq.append(ImpEp);

        Graph_Show(ui->widget_Ep, port2_x, port2_y_Eq);

        QString str = QString("%1kWh").arg(ImpEp);
        ui->listWidget->addItem(str);
    }
}

// 读取消息，更新消息
void SubscriptionWindow::updateMessage(const QMqttMessage &msg)
{
    const QByteArray data = msg.payload();
    ui->listWidget->addItem(ByteArrayToHexString(data));

    AnalyticalData(data);
}

// 更新状态
void SubscriptionWindow::updateStatus(QMqttSubscription::SubscriptionState state)
{
    switch (state) {
    case QMqttSubscription::Unsubscribed:   //取消订阅
        ui->labelStatus->setText(QLatin1String("Unsubscribed"));
        break;
    case QMqttSubscription::SubscriptionPending:    //订阅待定
        ui->labelStatus->setText(QLatin1String("Pending"));
        break;
    case QMqttSubscription::Subscribed:     // 订阅
        ui->labelStatus->setText(QLatin1String("Subscribed"));
        break;
    case QMqttSubscription::Error:          // 错误
        ui->labelStatus->setText(QLatin1String("Error"));
        break;
    default:
        ui->labelStatus->setText(QLatin1String("--Unknown--"));
        break;
    }
}

// 绘制背景
void SubscriptionWindow::DrawBackground(QCustomPlot *CustomPlot, QString str)
{
    //设置画布颜色
    QLinearGradient plotGradient;
    //lotGradient.setStart(0, 0);
    //plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    //plotGradient.setColorAt(1, QColor(50, 50, 50));
    //ui.widget->setBackground(plotGradient);
    CustomPlot->setBackground(plotGradient);

    CustomPlot->legend->setVisible(true);  // 显示名字

    //设置坐标颜色/坐标名称颜色
    CustomPlot->xAxis->setLabelColor(Qt::white);//文字颜色
    CustomPlot->yAxis->setLabelColor(Qt::white);
    CustomPlot->xAxis->setTickLabelColor(Qt::white);//坐标轴数字颜色
    CustomPlot->yAxis->setTickLabelColor(Qt::white);
    CustomPlot->xAxis->setBasePen(QPen(Qt::white, 1));//坐标轴颜色及宽度
    CustomPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    CustomPlot->xAxis->setTickPen(QPen(Qt::white, 1));//主刻度
    CustomPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    CustomPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));//副刻度
    CustomPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));


    //设置属性可缩放，移动等
    //ui.widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
    //	QCP::iSelectLegend | QCP::iSelectPlottables);
    CustomPlot->setInteractions(
                QCP::iRangeDrag             // 鼠标拖动
                | QCP::iRangeZoom           // 滚轮缩放
                | QCP::iSelectAxes          // 轴可选择
                | QCP::iSelectLegend        // 图例可选
                | QCP::iSelectPlottables);  // 线可选

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);//日期做X轴
    //dateTicker->setDateTimeFormat("yy-MM-dd hh:mm:ss");//日期格式(可参考QDateTime::fromString()函数)
    dateTicker->setDateTimeFormat("MM-dd hh:mm:ss");//日期格式(可参考QDateTime::fromString()函数)
    //dateTicker->setDateTimeFormat("hh:mm:ss");//日期格式(可参考QDateTime::fromString()函数)
    CustomPlot->xAxis->setTicker(dateTicker);//设置X轴为时间轴

    //CustomPlot->xAxis->setLabel("时间");//x轴的文字
    //CustomPlot->yAxis->setLabel("电流");//y轴的文字
    CustomPlot->yAxis->setLabel(str);//y轴的文字

    //设置伸缩比例 setRangeZoomFactor( double horizontalFactor, double verticalFactor );可以分别设定X,Y方向
    CustomPlot->axisRect()->setRangeZoomFactor(0.5, 1); //x方向为2,y方向为1不缩放

    // setup look of bottom tick labels:
    CustomPlot->xAxis->setTickLabelRotation(15);
    CustomPlot->xAxis->ticker()->setTickCount(10);
    //CustomPlot->xAxis->setNumberFormat("ebc");
    //CustomPlot->xAxis->setNumberPrecision(1);
    //CustomPlot->xAxis->moveRange(-10);

    CustomPlot->replot();//重绘
}

// 绘制曲线
void SubscriptionWindow::Graph_Show(QCustomPlot *CustomPlot, QVector<double> port_x, QVector<double> port_y)
{
    if (nullptr == CustomPlot->graph(0))
    {
        CustomPlot->addGraph();//添加一条曲线
    }
    CustomPlot->graph(0)->setName("");//曲线名称
    CustomPlot->graph(0)->setPen(QPen(Qt::green)); //0是曲线序号，添加的第一条是0，设置曲线颜色
    CustomPlot->graph(0)->setData(port_x, port_y); //输出各点的图像，x和y都是QVector类
    //CustomPlot->graph(0)->addData()
    CustomPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5)); //3：点的尺寸

    //x轴范围
    CustomPlot->xAxis->setRange(port_x[(0>port_x.size()-100)?0:port_x.size()-100], port_x[port_x.size()-1]);
    //Y轴 自动调整范围
    CustomPlot->graph(0)->rescaleValueAxis(true);

    CustomPlot->replot();//重绘
}
