#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    m_translator = new QTranslator(this);
    outputSaveFile = nullptr;
    inputFile = nullptr;
    serialPort = new QSerialPort;
    QTimer *timer = new QTimer(this);
    QTimer *portTimer = new QTimer(this);
    isCircleSend = false;
    hideExtreItem();

    socket = new QUdpSocket(this);
    tcp_socket = new QTcpSocket(this);
    server = new QTcpServer(this);
    connect(server,&QTcpServer::newConnection,this,[=](){
        if(server->hasPendingConnections())
        {
            QTcpSocket *newSocket = server->nextPendingConnection();
            m_list.append(newSocket);
            qDebug()<<newSocket->peerAddress().toString()<<newSocket->peerPort();
            connect(newSocket,&QTcpSocket::readyRead,this,&MainWindow::readData);
            connect(newSocket,&QTcpSocket::disconnected,[=](){
                m_list.removeOne(newSocket);
                QMessageBox::information(this,"提示",QString("IP:%1 PORT:%2已退出连接")
                                         .arg((newSocket->peerAddress().toString()))
                                         .arg(newSocket->peerPort()));
            });
        }
    });//server模式添加和删除连接的逻辑
    //server模式的显示业务逻辑

    connect(tcp_socket,&QTcpSocket::connected,this,[=](){
        QMessageBox::information(this,"提示","服务器连接成功");
        m_netConnected = true;
        ui->netSettingButton->setText(tr("关闭监听"));
    });
    connect(tcp_socket,&QTcpSocket::disconnected,this,[=](){
        QMessageBox::warning(this,"提示","服务器断开连接");
        m_netConnected = false;
        ui->netSettingButton->setText(tr("开始监听"));
        ui->ipLineEdit->setEnabled(true);
        ui->portLineEdit->setEnabled(true);
        ui->comboBox->setEnabled(true);

    });
    connect(tcp_socket,&QTcpSocket::readyRead,this,[=](){
        QByteArray arr =tcp_socket->readAll();
        QString str = QString::fromLocal8Bit(arr);

        if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Unchecked
                && ui->recevierSettingWidgetRight->getIsShow() == Qt::Unchecked)//文件存储未勾选并且显示接收
        {
            if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Checked)
            {
                if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)
                {
                    ui->netTextBrowser->append(arr.toHex(' ').toUpper());
                }else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked){
                    ui->netTextBrowser->append(str);
                }
            }else if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Unchecked)
            {
                if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)
                {
                    ui->netTextBrowser->insertPlainText(arr.toHex(' ').toUpper());
                }else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked){
                    ui->netTextBrowser->insertPlainText(str);
                }

            }

        }else if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Checked)
        {
            if(outputSaveFile != nullptr)
            {
                outputSaveFile->write(arr);
                outputSaveFile->flush();
            }else{
                QMessageBox::critical(this,"警告","文件未指定");
            }
        }
        nReciveCount += arr.size();//接收计数
        ui->statusWidget->setNrecive(nReciveCount);

    });//tcp client接收事务逻辑

    connect(tcp_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, [=](){
        qDebug() << "连接失败原因：" << tcp_socket->errorString();
        QMessageBox::critical(this, "连接失败", tcp_socket->errorString());
        ui->netSettingButton->setText(tr("开始监听")); // 连接失败，按钮文字退回去
    });
    connect(ui->comboBox,&QComboBox::currentTextChanged,this,[=](){
        if(ui->comboBox->currentText() == "TCP client")
        {
            ui->label_2->setText(tr("服务器IP地址"));
            ui->label_3->setText(tr("服务器端口号"));
        }else if(ui->comboBox->currentText() == "TCP server")
        {
            ui->label_2->setText(tr("本地IP地址"));
            ui->label_3->setText(tr("本地端口号"));
        }else if(ui->comboBox->currentText() == "UDP")
        {
            ui->label_2->setText(tr("本地IP地址"));
            ui->label_3->setText(tr("本地端口号"));
        }

    });
    connect(ui->netSettingButton,&QPushButton::clicked,this,[=](){
        if(m_netConnected == false)//ui->netSettingButton->text() == "开始监听"
        {
            ui->netMesButton->disconnect();
            if(ui->comboBox->currentText() == "UDP")//选择UDP时的业务逻辑
            {
                QHostAddress ip_addr = QHostAddress(ui->ipLineEdit->text());
                quint16 port = ui->portLineEdit->text().toInt();
                if(socket->bind(ip_addr,port))
                {
                    m_netConnected = true;
                    qDebug()<<"ok";
                    showExtreItem();
                    connect(socket,&QUdpSocket::readyRead,this,[=](){
                        while (socket->hasPendingDatagrams()) {
                            QByteArray arr;                            
                            arr.resize(socket->pendingDatagramSize());
                            nReciveCount += socket->pendingDatagramSize();//数据接收
                            ui->statusWidget->setNrecive(nReciveCount);
                            socket->readDatagram(arr.data(),socket->pendingDatagramSize());
                            if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Unchecked
                                    && ui->recevierSettingWidgetRight->getIsShow() == Qt::Unchecked)//是否以文件存储
                            {
                                if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Checked)//是否自动换行
                                {
                                    if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)//是否为16进制显示
                                    {
                                        ui->netTextBrowser->append(arr.toHex(' ').toUpper());
                                    }
                                    else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked)
                                    {
                                        ui->netTextBrowser->append(QString::fromLocal8Bit(arr));
                                    }
                                }else if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Unchecked)
                                {
                                    if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)
                                    {
                                        ui->netTextBrowser->insertPlainText(arr.toHex(' ').toUpper());
                                    }
                                    else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked)
                                    {
                                        ui->netTextBrowser->insertPlainText(QString::fromLocal8Bit(arr));
                                    }
                                }
                            }else if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Checked)
                            {
                                if(outputSaveFile != nullptr)
                                {
                                    outputSaveFile->write(arr);
                                    outputSaveFile->flush();
                                }else{
                                    QMessageBox::critical(this,"警告","文件未指定");
                                }
                            }
                        }
                    });
                    connect(ui->netMesButton,&QPushButton::clicked,this,[=](){
                        if(ui->ipLineEdit->text() != "" && ui->portLineEdit->text() != "")
                        {
                            if(m_netSend == false)//ui->netMesButton->text() == "发送"
                            {
                                if(ui->netTextEdit->toPlainText() != "" && ui->hostIpLineEdit->text() != "" && ui->hostPortlineEdit->text() != "")
                                {

                                        QByteArray mes;
                                        if(ui->senderSettingWidgetRight->isUseFileToSend() == Qt::Unchecked)
                                        {
                                            mes = ui->netTextEdit->toPlainText().toLocal8Bit();
                                        }else if(ui->senderSettingWidgetRight->isUseFileToSend() == Qt::Checked
                                                 && inputFile != nullptr)
                                        {
                                            mes = inputFile->readAll();
                                        }
                                        if(ui->senderSettingWidgetRight->isUseLastBite() == Qt::Checked)
                                        {
                                            if(lastBiteItem == 2)//正校验
                                            {
                                                lastBite = 0;
                                                for(int i = 0;i < mes.size();i++)
                                                {
                                                    lastBite += (mes.at(i));
                                                }
                                            }else if(lastBiteItem == 1)//负校验
                                            {
                                                lastBite = 0;
                                                for(int i = 0;i < mes.size();i++)
                                                {
                                                    lastBite += (mes.at(i));
                                                }
                                                lastBite = (~lastBite)+1;
                                            }
                                            //固定位不需要操作
                                            mes.append(lastBite);
                                        }
                                        QHostAddress arr = QHostAddress(ui->hostIpLineEdit->text());
                                        quint16 port = ui->hostPortlineEdit->text().toUInt();
                                        if(isCircleSend)
                                        {
                                            ui->netTextEdit->blockSignals(false);
                                            timer->disconnect();
                                            connect(timer,&QTimer::timeout,this,[=](){
                                                sendMesUDP(arr,port,mes);
                                            });
                                            timer->start(splitTime);
                                            ui->netTextEdit->blockSignals(true);
                                            ui->netMesButton->setText(tr("停止发送"));
                                            m_netSend = true;
                                            ui->netSettingButton->setEnabled(false);
                                        }else{
                                            sendMesUDP(arr,port,mes);
                                        }


                                        if(ui->senderSettingWidgetRight->isSentToClearAuto() == Qt::Checked)
                                        {
                                            ui->netTextEdit->clear();
                                        }


                                }
                            }
                            else if(isCircleSend == true && getMnetSend() == true)
                            {
                                timer->stop();
                                timer->disconnect();
                                ui->netTextEdit->setEnabled(true);
                                ui->netMesButton->setText(tr("发送"));
                                m_netSend = false;
                                ui->netSettingButton->setEnabled(true);
                            }

                        }
                    });
                }else {
                    qDebug()<<socket->error();
                }

            }else if(ui->comboBox->currentText() == "TCP server")//TCP server业务逻辑
            {
                QHostAddress addr = QHostAddress(ui->ipLineEdit->text());
                quint16 port = ui->portLineEdit->text().toUInt();
                bool ok =  server->listen(addr,port);
                m_netConnected = true;
                qDebug()<<ok;
                if(ok)
                {
                    showExtreItem();
                }
                connect(ui->netMesButton,&QPushButton::clicked,this,[=](){
                    QByteArray arr = ui->netTextEdit->toPlainText().toLatin1();
                    if(ui->senderSettingWidgetRight->isUseLastBite() == Qt::Checked)
                    {
                        switch (lastBiteItem) {
                            case 1:
                                //负校验
                                lastBite = 0;
                                for(int i = 0;i < arr.size();i++)
                                {
                                    lastBite += (arr.at(i));
                                }
                                lastBite = (~lastBite) + 1;
                                break;
                            case 2:
                                //正校验
                                lastBite = 0;
                                for(int i = 0;i < arr.size();i++)
                                {
                                    lastBite += (arr.at(i));
                                }
                                break;
                            default:
                                break;

                        }
                        arr.append(lastBite);
                    }
                    if(m_netSend == false)//ui->netMesButton->text() == "发送"
                    {
                        if(isCircleSend)
                        {
                            timer->disconnect();
                            connect(timer,&QTimer::timeout,this,[=](){
                                for(auto it:m_list)
                                {
                                    it->write(arr);
                                    nSendCount += arr.size();
                                    ui->statusWidget->setNsend(nSendCount);
                                }
                            });
                            timer->start(splitTime);
                            ui->netMesButton->setText(tr("停止发送"));
                            m_netSend = true;
                            ui->netSettingButton->setEnabled(false);
                        }else{
                            for(auto it:m_list)
                            {
                                it->write(arr);
                                nSendCount += arr.size();//tcp server发送计数
                                ui->statusWidget->setNsend(nSendCount);
                            }
                        }
                    }else if(m_netSend == true && isCircleSend == true)//ui->netMesButton->text() == "停止发送"
                    {
                        timer->disconnect();
                        timer->stop();
                        ui->netMesButton->setText(tr("发送"));
                        m_netSend = false;
                        ui->netSettingButton->setEnabled(true);
                    }


                });

            }else if(ui->comboBox->currentText() == "TCP client")//选择TCP client的业务逻辑
            {
                //创建
                QHostAddress addr = QHostAddress(ui->ipLineEdit->text());
                quint16 port = ui->portLineEdit->text().toUInt();
                qDebug()<<addr<<port;

                tcp_socket->connectToHost(addr,port);

                connect(ui->netMesButton,&QPushButton::clicked,this,[=](){
                    QByteArray arr = ui->netTextEdit->toPlainText().toLocal8Bit();
                    if(ui->senderSettingWidgetRight->isUseLastBite() == Qt::Checked)
                    {
                        switch (lastBiteItem) {
                            case 1:
                                //负校验
                                lastBite = 0;
                                for(int i = 0;i < arr.size();i++)
                                {
                                    lastBite += (arr.at(i));
                                }
                                lastBite = (~lastBite) + 1;
                                break;
                            case 2:
                                //正校验
                                lastBite = 0;
                                for(int i = 0;i < arr.size();i++)
                                {
                                    lastBite += (arr.at(i));
                                }
                                break;
                            default:
                                break;

                        }
                        arr.append(lastBite);
                    }
                    if(m_netSend == false)//ui->netMesButton->text() == "发送"
                    {
                        if(isCircleSend)
                        {
                            timer->disconnect();
                            connect(timer,&QTimer::timeout,this,[=](){
                                tcp_socket->write(arr);
                                nSendCount += arr.size();//tcp client循环发送
                                ui->statusWidget->setNsend(nSendCount);
                            });
                            timer->start(splitTime);
                            ui->netTextEdit->setEnabled(false);
                            ui->netMesButton->setText(tr("停止发送"));
                            m_netSend = true;
                            ui->netSettingButton->setEnabled(false);
                        }else
                        {
                            tcp_socket->write(arr);
                            nSendCount += arr.size();//tcp client发送计数
                            ui->statusWidget->setNsend(nSendCount);
                        }
                    }else if(m_netSend == true && isCircleSend == true)//ui->netMesButton->text() == "停止发送"
                    {
                        timer->stop();
                        timer->disconnect();
                        ui->netTextEdit->setEnabled(true);
                        ui->netMesButton->setText(tr("发送"));
                        m_netSend = false;
                        ui->netSettingButton->setEnabled(true);
                    }


                });
                //断开
            }
            else if(ui->comboBox->currentText() == "modbus master")
            {
                modbusDialog = new modbusMasterDialog;
                modbusDialog->show();
                connect(modbusDialog,&modbusMasterDialog::rejected,this,[=](){
                    m_netConnected = false;
                    ui->comboBox->setEnabled(true);
                    ui->netSettingButton->setText(tr("开始"));
                });
            }
            ui->netSettingButton->setText(tr("关闭监听"));
            ui->comboBox->setEnabled(false);
            ui->ipLineEdit->setEnabled(false);
            ui->portLineEdit->setEnabled(false);
        }else if(m_netConnected == true)//ui->netSettingButton->text() == "关闭监听"
        {
            if(ui->comboBox->currentText() == "UDP")
            {
                socket->abort();
                ui->netSettingButton->setText(tr("开始监听"));
                hideExtreItem();
            }else if(ui->comboBox->currentText() == "TCP server")
            {
                server->close();
                ui->netSettingButton->setText(tr("开始监听"));
                while(!m_list.empty())
                {
                    QTcpSocket *newSocket = m_list.takeFirst();
                    newSocket->disconnectFromHost();
                    newSocket->deleteLater();
                }
                hideExtreItem();
            }else if(ui->comboBox->currentText() == "TCP client")
            {
                tcp_socket->disconnectFromHost();
                ui->netSettingButton->setText(tr("开始监听"));
            }
            m_netConnected = false;
            ui->comboBox->setEnabled(true);
            ui->ipLineEdit->setEnabled(true);
            ui->portLineEdit->setEnabled(true);
        }

    });//网络设置的点击按钮事件逻辑

    connect(ui->recevierSettingWidgetRight,&recevierSettingForm::clearBroswer,this,[=](){
        ui->netTextBrowser->clear();
    });//清除net接受区
    connect(ui->recevierSettingWidgetRight,&recevierSettingForm::saveText,this,[=](QUrl url){
        QUrl localUrl = url.toLocalFile();

        QFile file(localUrl.toString());
        if(file.open(QIODevice::ReadWrite))
        {
            QByteArray arr = ui->netTextBrowser->toPlainText().toLatin1();
            if(file.write(arr) != -1)
            {
                QMessageBox::information(this,"提示","内容保存成功");
            }else{
                QMessageBox::critical(this,"错误",QString("内容保存失败%1").arg(file.errorString()));
            }
        }else{
            qDebug()<<file.errorString();
        }
        file.close();
    });//保存net接受区的文本
    connect(ui->recevierSettingWidgetRight,&recevierSettingForm::outputIntoFile,this,[=](QUrl url){
        if(url.isEmpty()) return;
        QUrl localUrl = url.toLocalFile();
        outputSaveFile = new QFile(localUrl.toString(),this);

        if(outputSaveFile->open(QIODevice::ReadWrite | QIODevice::Append))
        {
            ui->netTextBrowser->clear();
            ui->netTextBrowser->setText(QString("接收转到文件%1").arg(localUrl.toString()));
            ui->netTextBrowser->setEnabled(false);

        }else{
            qDebug()<<outputSaveFile->errorString();
        }
    });//转存到文件业务逻辑
    connect(ui->recevierSettingWidgetRight,&recevierSettingForm::releaseBroswer,this,[=](){
        ui->netTextBrowser->clear();
        ui->netTextBrowser->setEnabled(true);
        outputSaveFile->close();
        delete outputSaveFile;
        outputSaveFile = nullptr;
    });//取消转存到文件逻辑
    connect(ui->recevierSettingWidgetRight,&recevierSettingForm::setBroswerDisable,this,[=](Qt::CheckState state){
        if(state == Qt::Checked)
        {
            ui->netTextBrowser->setEnabled(false);
        }else if(state == Qt::Unchecked)
        {
            ui->netTextBrowser->setEnabled(true);
        }
    });//设置net接收区不可用
    connect(ui->senderSettingWidgetRight,&senderSettingForm::clearInput,this,[=](){
        if(isCircleSend)
        {
            return;
        }else{
            ui->netTextEdit->clear();
        }

    });//清空net发送框
    connect(ui->senderSettingWidgetRight,&senderSettingForm::fileIntoInput,this,[=](){
        inputFile = new QFile(QFileDialog::getSaveFileUrl().toLocalFile(),this);
        if(inputFile->open(QIODevice::ReadWrite))
        {
            QByteArray arr = inputFile->readAll();
            ui->netTextEdit->setText(QString::fromLocal8Bit(arr));
        }else{
            QMessageBox::critical(this,"错误",QString("%1").arg(inputFile->errorString()));
        }
    });//将文件内容加载到net发送框
    connect(ui->senderSettingWidgetRight,&senderSettingForm::checkToUseFile,this,[=](QUrl url){
        if(!url.isEmpty())
        {
            inputFile = new QFile(url.toLocalFile());
            if(inputFile->open(QIODevice::ReadOnly))
            {
                ui->netTextEdit->clear();
                ui->netTextEdit->setText(QString("正在使用文件%1进行发送").arg(url.toLocalFile()));
                ui->netTextEdit->setEnabled(false);
                qDebug()<<url;
            }else{
                QMessageBox::critical(this,"错误",QString("%1").arg(inputFile->errorString()));
            }
        }

    });//如果点击使用文件进行发送，将inputFile以只读方式打开选择的文件
    connect(ui->senderSettingWidgetRight,&senderSettingForm::unCheckToUseFile,this,[=](){
        if(inputFile != nullptr)
        {
            delete inputFile;
            inputFile = nullptr;
            ui->netTextEdit->clear();
            ui->netTextEdit->setEnabled(true);
        }
    });//取消勾选使用文件发送，设置inputfile为nullptr
    connect(ui->senderSettingWidgetRight,&senderSettingForm::checkToSendLastBite,this,[=](int item,uint16_t bite){
        if(item > 0 && item < 3)
        {
            lastBiteItem = item;
            lastBite = 0;
        }else if(item == 3)
        {
            lastBiteItem = item;
            lastBite = bite;
        }
    });//接收回传的信号，并设置全局变量
    connect(ui->senderSettingWidgetRight,&senderSettingForm::checkHexToSend,this,[=](){
        QString str = ui->netTextEdit->toPlainText();
        if(ui->senderSettingWidgetRight->isUseHexToSend() == Qt::Checked)
        {
             QByteArray arr = str.toLocal8Bit();
             ui->netTextEdit->clear();
             arr = arr.toHex(' ').toUpper();
             ui->netTextEdit->setText(arr);
        }else if(ui->senderSettingWidgetRight->isUseHexToSend() == Qt::Unchecked){
            QByteArray arr = QByteArray::fromHex(str.toLatin1());
            QString newStr = QString::fromLocal8Bit(arr);
            ui->netTextEdit->clear();
            ui->netTextEdit->setText(newStr);
        }
    });//net输入框转16进制
    connect(ui->portSettingWidget,&portSettingForm::openPort,this,[=](QString pName){

        if(!pName.isEmpty())
        {
            serialPort->setPortName(pName) ;
            serialPort->setBaudRate(QSerialPort::Baud115200);
            serialPort->setDataBits(QSerialPort::Data8);
            serialPort->setStopBits(QSerialPort::OneStop);
            serialPort->setParity(QSerialPort::NoParity);
            if(serialPort->open(QIODevice::ReadWrite))
            {
                    QMessageBox::information(this,"提示","串口打开成功");
            }
        }else{
            QMessageBox::critical(this,"警告","未获取到串口");
        }
    });//配置串口信息
    connect(ui->portSettingWidget,&portSettingForm::closePort,[=](){
        if(serialPort->isOpen())
        {
            serialPort->close();
        }
    });//关闭串口
    connect(ui->reciverSettingWidgetLeft,&recevierSettingForm::clearBroswer,this,[=](){
        ui->portTextBrowser->clear();
    });//port接收区清空
    connect(ui->reciverSettingWidgetLeft,&recevierSettingForm::saveText,this,[=](QUrl url){
        if(url.isEmpty())
        {
            return;
        }else{
            QUrl localUrl = url.toLocalFile();

            QFile file(localUrl.toString());
            if(file.open(QIODevice::ReadWrite))
            {
                QByteArray arr = ui->portTextBrowser->toPlainText().toLatin1();
                if(file.write(arr) != -1)
                {
                    QMessageBox::information(this,"提示","内容保存成功");
                }else{
                    QMessageBox::critical(this,"错误",QString("内容保存失败%1").arg(file.errorString()));
                }
            }else{
                qDebug()<<file.errorString();
            }
            file.close();
        }
    });//port接收区保存到文件
    connect(ui->reciverSettingWidgetLeft,&recevierSettingForm::outputIntoFile,this,[=](QUrl url){
        if(url.isEmpty())
        {
            return;
        }else{
                outputSaveFile = new QFile(url.toLocalFile(),this);
                if(outputSaveFile->open(QIODevice::Append | QIODevice::ReadWrite))
                {
                    ui->portTextBrowser->clear();
                    ui->portTextBrowser->setText(QString("接收转到文件%1").arg(url.toLocalFile()));
                    ui->portTextBrowser->setEnabled(false);
                    qDebug()<<"ok";
                }
        }
    });//port接收转到文件
    connect(ui->reciverSettingWidgetLeft,&recevierSettingForm::releaseBroswer,this,[=](){
        ui->portTextBrowser->clear();
        ui->portTextBrowser->setEnabled(true);
        delete outputSaveFile;
        outputSaveFile = nullptr;
    });//port接受区恢复
    connect(ui->reciverSettingWidgetLeft,&recevierSettingForm::setBroswerDisable,this,[=](Qt::CheckState state){
        if(state == Qt::Checked)
        {
            ui->portTextBrowser->setEnabled(false);
        }else if(state == Qt::Unchecked)
        {
            ui->portTextBrowser->setEnabled(true);
        }
    });//设置port接收区不可用
    connect(ui->portMesButton,&QPushButton::clicked,this,[=](){
        if(m_portSend == false)//ui->portMesButton->text() == "发送"
        {
            if(serialPort->isOpen() && !ui->portTextEdit->toPlainText().isEmpty())
            {
                m_portConnected = true;
                QByteArray arr = ui->portTextEdit->toPlainText().toLatin1();
                if(ui->senderSettingWidgetLeft->isUseLastBite() == Qt::Checked)
                {
                    if(lastBiteItem == 2)//正校验
                    {
                        lastBite = 0;
                        for(int i = 0;i < arr.size();i++)
                        {
                            lastBite += (arr.at(i));
                        }

                    }else if(lastBiteItem == 1)//负校验
                    {
                        lastBite = 0;
                        for(int i = 0;i < arr.size();i++)
                        {
                            lastBite += (arr.at(i));
                        }
                        lastBite = (~lastBite)+1;
                    }
                    //固定位不需要操作
                    arr.append(lastBite);
                }
                if(isPortCircleSend)
                {
                    connect(portTimer,&QTimer::timeout,this,[=](){
                        serialPort->write(arr);
                        pSendCount += arr.size();//port发送
                        ui->statusWidget->setPsend(pSendCount);
                    });
                    portTimer->start(portSplitTime);
                    ui->portMesButton->setText(tr("停止发送"));
                    m_portSend = true;

                }else{
                    serialPort->write(arr);
                    pSendCount += arr.size();
                    ui->statusWidget->setPsend(pSendCount);
                }

                if(ui->senderSettingWidgetLeft->isSentToClearAuto() == Qt::Checked)
                {
                    ui->portTextEdit->clear();
                }
            }
        }else if(m_portSend == true && isPortCircleSend == true)//ui->portMesButton->text() == "停止发送"
        {
            portTimer->stop();
            portTimer->disconnect();
            isCircleSend = false;
            ui->portMesButton->setText("发送");
            m_portSend = false;
        }

    });//串口发送/循环 数据
    connect(serialPort,&QSerialPort::readyRead,this,[=](){
        QByteArray arr = serialPort->readLine();
//        QString str = QString::fromLocal8Bit(arr);
        if(ui->reciverSettingWidgetLeft->getIsShow() == Qt::Unchecked)
        {
            if(ui->reciverSettingWidgetLeft->getIsChangeLine() == Qt::Checked)
            {
                if(ui->reciverSettingWidgetLeft->getIsChangeHex() == Qt::Checked)
                {
                    ui->portTextBrowser->append(QString::fromLocal8Bit(arr.toHex(' ').toUpper()));
                }else{
                    ui->portTextBrowser->append(QString::fromLocal8Bit(arr));
                }

            }else if(ui->reciverSettingWidgetLeft->getIsChangeLine() == Qt::Unchecked)
            {
                if(ui->reciverSettingWidgetLeft->getIsChangeHex() == Qt::Checked)
                {
                    ui->portTextBrowser->insertPlainText(QString::fromLocal8Bit(arr.toHex(' ').toUpper()));
                }else{
                    ui->portTextBrowser->insertPlainText(QString::fromLocal8Bit(arr));
                }
            }
        }

        pReciveCount += arr.size();
        ui->statusWidget->setPrecive(pReciveCount);

    });//串口接收数据逻辑
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::checkHexToSend,this,[=](){
        QString str = ui->portTextEdit->toPlainText();
        if(ui->senderSettingWidgetLeft->isUseHexToSend() == Qt::Checked)
        {
             QByteArray arr = str.toLocal8Bit();
             ui->portTextEdit->clear();
             arr = arr.toHex(' ').toUpper();
             ui->portTextEdit->setText(arr);
        }else if(ui->senderSettingWidgetLeft->isUseHexToSend() == Qt::Unchecked){
            QByteArray arr = QByteArray::fromHex(str.toLatin1());
            QString newStr = QString::fromLocal8Bit(arr);
            ui->portTextEdit->clear();
            ui->portTextEdit->setText(newStr);
        }
    });//port输入框转16进制
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::fileIntoInput,this,[=](){
        inputFile = new QFile(QFileDialog::getSaveFileUrl().toLocalFile(),this);
        if(inputFile->open(QIODevice::ReadWrite))
        {
            QByteArray arr = inputFile->readAll();
            ui->portTextEdit->setText(QString::fromLocal8Bit(arr));
        }else{
            QMessageBox::critical(this,"错误",QString("%1").arg(inputFile->errorString()));
        }
    });//将文件内容加载到port发送框
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::checkToUseFile,this,[=](QUrl url){
        if(!url.isEmpty())
        {
            inputFile = new QFile(url.toLocalFile());
            if(inputFile->open(QIODevice::ReadOnly))
            {
                ui->portTextEdit->clear();
                ui->portTextEdit->setText(QString("正在使用文件%1进行发送").arg(url.toLocalFile()));
                ui->portTextEdit->setEnabled(false);
                qDebug()<<url;
            }else{
                QMessageBox::critical(this,"错误",QString("%1").arg(inputFile->errorString()));
            }
        }

    });//如果点击使用文件进行发送，将inputFile以只读方式打开选择的文件
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::unCheckToUseFile,this,[=](){
        if(inputFile != nullptr)
        {
            delete inputFile;
            inputFile = nullptr;
            ui->portTextEdit->clear();
            ui->portTextEdit->setEnabled(true);
        }
    });//取消勾选使用文件发送，设置inputfile为nullptr
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::checkToSendLastBite,this,[=](int item,uint16_t bite){
        if(item > 0 && item < 3)
        {
            lastBiteItem = item;
            lastBite = 0;
        }else if(item == 3)
        {
            lastBiteItem = item;
            lastBite = bite;
        }
    });//接收回传的信号，并设置全局变量
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::clearInput,this,[=](){
        ui->portTextEdit->clear();
    });//port清空按钮
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::checkToCircleSend,this,[=](int sTime){
        isPortCircleSend = true;
        portSplitTime = sTime;
    });//设置自动发送的参数
    connect(ui->senderSettingWidgetLeft,&senderSettingForm::unCircleToSend,this,[=](){
        isPortCircleSend = false;
        portTimer->stop();
    });//停止自动发送
    connect(ui->senderSettingWidgetRight,&senderSettingForm::checkToCircleSend,this,[=](int sTime){
        isCircleSend = true;
        splitTime = sTime;
    });//设置net自动发送的参数
    connect(ui->senderSettingWidgetRight,&senderSettingForm::unCircleToSend,this,[=](){
        isCircleSend = false;
        timer->stop();
    });//停止net自动发送
    connect(ui->statusWidget,&statusBarForm::portCountReset,this,[=](){
        pSendCount = 0;
        pReciveCount = 0;
        ui->statusWidget->setPrecive(pReciveCount);
        ui->statusWidget->setPsend(pSendCount);
    });//清空port接发数据
    connect(ui->statusWidget,&statusBarForm::netCountReset,this,[=](){
        nSendCount = 0;
        nReciveCount = 0;
        ui->statusWidget->setNsend(nSendCount);
        ui->statusWidget->setNrecive(nReciveCount);
    });//清空net接发数据
    connect(ui->comboBox,&QComboBox::currentTextChanged,this,[=](){
        if(ui->comboBox->currentText() == "modbus master")
        {
            ui->netSettingButton->setText("开始");
            ui->ipLineEdit->setEnabled(false);
            ui->portLineEdit->setEnabled(false);
        }
        else{
            ui->ipLineEdit->setEnabled(true);
            ui->portLineEdit->setEnabled(true);
        }

    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::getMportSend()
{
    return m_portSend;
}

void MainWindow::setMportSend(bool status)
{
    if(status == false || status == true)
    {
        m_portSend = status;
    }
}

bool MainWindow::getMnetSend()
{
    return m_netSend;
}

void MainWindow::setMnetSend(bool status)
{
    if(status == false || status == true)
    {
        m_netSend = status;
    }
}

bool MainWindow::getMportConnected()
{
    return m_portConnected;
}

void MainWindow::setMportConnected(bool status)
{
    if(status == false || status == true)
    {
        m_portConnected = status;
    }
}

bool MainWindow::getMnetConnected()
{
    return m_netConnected;
}

void MainWindow::setMnetConnected(bool status)
{
    if(status == false || status == true)
    {
        m_netConnected = status;
    }
}

void MainWindow::sendMesUDP(const QHostAddress hostAddr, const quint16 hostPort, QByteArray mes)
{
    if(socket->isValid())
    {
        socket->writeDatagram(mes,hostAddr,hostPort);
        nSendCount += mes.size();//udp发送计数
        ui->statusWidget->setNsend(nSendCount);
    }

}//udp发送数据

void MainWindow::readData()
{
    QTcpSocket*sendSocket = dynamic_cast<QTcpSocket*>(sender());
    QByteArray mainArr = sendSocket->readAll();
    QString str = QString::fromLocal8Bit(mainArr);
    if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Unchecked
            && ui->recevierSettingWidgetRight->getIsShow() == Qt::Unchecked)
    {
        if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Checked)
        {
            if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)
            {
                ui->netTextBrowser->append(mainArr.toHex(' ').toUpper());
            }else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked){
                ui->netTextBrowser->append(str);
            }

        }else if(ui->recevierSettingWidgetRight->getIsChangeLine() == Qt::Unchecked)
        {
            if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Checked)
            {
                ui->netTextBrowser->insertPlainText(mainArr.toHex(' ').toUpper());
            }else if(ui->recevierSettingWidgetRight->getIsChangeHex() == Qt::Unchecked){
                ui->netTextBrowser->insertPlainText(str);
            }

        }

    }else if(ui->recevierSettingWidgetRight->getStatusOfOutput() == Qt::Checked)
    {
        if(outputSaveFile != nullptr)
        {
            outputSaveFile->write(mainArr);

            outputSaveFile->flush();
        }else{
            QMessageBox::critical(this,"警告","文件未指定");
        }
    }
    nReciveCount += mainArr.size();//tcp发送计数
    ui->statusWidget->setNrecive(nReciveCount);
}//tcp读取信息

void MainWindow::showExtreItem()
{
//    ui->label_4->show();
//    ui->label_5->show();
//    ui->hostIpLineEdit->show();
//    ui->hostPortlineEdit->show();
    ui->targetInfoWidget->show();
    ui->hostIpLineEdit->setEnabled(true);
    ui->hostPortlineEdit->setEnabled(true);
}
//显示net接受区的ip和端口输入框
void MainWindow::hideExtreItem()
{
    ui->hostIpLineEdit->setEnabled(false);
    ui->hostPortlineEdit->setEnabled(false);
//    ui->label_4->hide();
//    ui->label_5->hide();
//    ui->hostIpLineEdit->hide();
//    ui->hostPortlineEdit->hide();
    ui->targetInfoWidget->hide();
}//隐藏net接受区的ip和端口输入框

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);

}//语言变更事件

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Enter)
    {
        QWidget* widget = qobject_cast<QWidget*>(watched);
        if(widget)
        {
            QString str = widget->toolTip();
            if(!str.isEmpty())
            {
                ui->statusWidget->setInfo(str);
            }else{
                ui->statusWidget->setInfo(tr("状态：就绪"));
            }
        }else if(event->type() == QEvent::Leave)
        {
            ui->statusWidget->setInfo(tr("状态：就绪"));
        }
    }
    QObject::eventFilter(watched,event);
}//鼠标过滤事件修改状态栏


void MainWindow::on_action_3_triggered()
{
    if(QMessageBox::question(this,"提示","是否退出") == QMessageBox::Yes)
    {
        this->close();
    }
}//退出


void MainWindow::on_action_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存配置文件", "./config.ini", "配置文件 (*.ini)");
    if(filePath.isEmpty()) return;
    QSettings setting(filePath,QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("network");
    setting.setValue("NET",ui->comboBox->currentText());
    setting.setValue("IP",ui->ipLineEdit->text());
    setting.setValue("PORT",ui->portLineEdit->text());
    setting.endGroup();

    setting.beginGroup("Port");
    setting.setValue("COM",ui->portSettingWidget->getPortName());
    setting.endGroup();

    setting.beginGroup("netInfo");
    setting.setValue("sender",ui->netTextEdit->toPlainText());
    setting.setValue("recive",ui->netTextBrowser->toPlainText());
    setting.endGroup();

    setting.beginGroup("portInfo");
    setting.setValue("sender",ui->portTextEdit->toPlainText());
    setting.setValue("recive",ui->portTextBrowser->toPlainText());
    setting.endGroup();

    QMessageBox::information(this,"提示","配置文件保存成功");

}//保存配置文件

void MainWindow::on_action_2_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择配置文件", "./config.ini", "配置文件 (*.ini)");
    if(filePath.isEmpty()) return;
    QSettings setting(filePath,QSettings::IniFormat);
    setting.setIniCodec("UTF-8");

    setting.beginGroup("network");
    ui->comboBox->setCurrentText(setting.value("NET","UDP").toString());
    ui->ipLineEdit->setText(setting.value("IP","127.0.0.1").toString());
    ui->portLineEdit->setText(setting.value("PORT","8007").toString());
    setting.endGroup();

    setting.beginGroup("Port");
    ui->portSettingWidget->setPortName((setting.value("COM","COM1")).toString());
    setting.endGroup();

    setting.beginGroup("netInfo");
    ui->netTextEdit->setText(setting.value("sender"," ").toString());
    ui->netTextBrowser->setText(setting.value("recive"," ").toString());
    setting.endGroup();

    setting.beginGroup("portInfo");
    ui->portTextEdit->setText(setting.value("sender"," ").toString());
    ui->portTextBrowser->setText(setting.value("recive"," ").toString());
    setting.endGroup();

    QMessageBox::information(this,"提示","配置文件加载成功");
}//加载配置文件


void MainWindow::on_actionEnglish_triggered()
{
    if(m_translator->load(":/Language/linguist_en.qm"))
    {
        qApp->installTranslator(m_translator);
    }else{
        qDebug()<<"路径有误";
    }

}//英文


void MainWindow::on_action_4_triggered()
{
    qApp->removeTranslator(m_translator);
}//中文


void MainWindow::on_action_5_triggered()
{
    pSendCount = 0;
    pReciveCount = 0;
    nSendCount = 0;
    nReciveCount = 0;
    ui->statusWidget->setNsend(nSendCount);
    ui->statusWidget->setNrecive(nReciveCount);
    ui->statusWidget->setPrecive(pReciveCount);
    ui->statusWidget->setPsend(pSendCount);
    QMessageBox::information(this,"提示","计数已重置");
}//重置全部计数

