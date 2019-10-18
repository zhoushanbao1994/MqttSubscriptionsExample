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


SubscriptionWindow::SubscriptionWindow(QMqttSubscription *sub, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubscriptionWindow),
    m_sub(sub)
{
    ui->setupUi(this);

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

// 读取消息，更新消息
void SubscriptionWindow::updateMessage(const QMqttMessage &msg)
{
    ui->listWidget->addItem(ByteArrayToHexString(msg.payload()));
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
