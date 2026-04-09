#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QUdpSocket>
#include<QTcpServer>
#include<QTcpSocket>
#include<QDebug>
#include<QTextEdit>
#include<QMessageBox>
#include<QFile>
#include"setlastbitedialog.h"
#include<string.h>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QTimer>
#include<QSettings>
#include<QTranslator>
#include<QEvent>
#include"modbusmasterdialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool getMportSend();
    void setMportSend(bool status);
    bool getMnetSend();
    void setMnetSend(bool status);
    bool getMportConnected();
    void setMportConnected(bool status);
    bool getMnetConnected();
    void setMnetConnected(bool status);
signals:
    void statusInfo(QString info);
protected:
    void sendMesUDP(const QHostAddress addr,const quint16 port,QByteArray mes);
    void readData();
    void showExtreItem();
    void hideExtreItem();
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_action_3_triggered();

    void on_action_triggered();

    void on_action_2_triggered();

    void on_actionEnglish_triggered();

    void on_action_4_triggered();

    void on_action_5_triggered();

private:
    Ui::MainWindow *ui;
    QUdpSocket *socket;
    QTcpSocket *tcp_socket;
    QTcpServer *server;
    QList<QTcpSocket*> m_list;
    QFile *outputSaveFile;
    QFile *inputFile;
    uint16_t lastBite;
    int lastBiteItem;
    QSerialPort *serialPort;
    bool isCircleSend = false;
    bool isPortCircleSend = false;
    int splitTime = 0;
    int portSplitTime = 0;
    bool m_portSend = false;//port发送按钮
    bool m_netSend = false;//net发送按钮
    bool m_portConnected = false;//port连接按钮
    bool m_netConnected = false;//net发送按钮
    QTranslator *m_translator;
    int pSendCount = 0;
    int pReciveCount = 0;
    int nSendCount = 0;
    int nReciveCount = 0;
    modbusMasterDialog *modbusDialog;
};
#endif // MAINWINDOW_H
