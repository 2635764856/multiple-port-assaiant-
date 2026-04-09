#ifndef STATUSBARFORM_H
#define STATUSBARFORM_H

#include <QWidget>
#include<QEvent>

namespace Ui {
class statusBarForm;
}

class statusBarForm : public QWidget
{
    Q_OBJECT

public:
    explicit statusBarForm(QWidget *parent = nullptr);
    ~statusBarForm();
    void setPsend(int num);//设置port发送数量
    void setPrecive(int num);//设置port接收数量
    void setNsend(int num);//设置net发送数量
    void setNrecive(int num);//设置net接收数量
    void changeEvent(QEvent *event) override;
    void setInfo(QString str);
signals:
    void portCountReset();
    void netCountReset();
private slots:
    void on_netCountResetButton_clicked();

private:
    Ui::statusBarForm *ui;
    int portSendCount = 0;
    int portReciveCount = 0;
    int netSendCount = 0;
    int netReciveCount = 0;
    QString portStatus;
    QString netStatus;
};

#endif // STATUSBARFORM_H
