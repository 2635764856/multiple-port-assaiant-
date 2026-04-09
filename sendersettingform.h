#ifndef SENDERSETTINGFORM_H
#define SENDERSETTINGFORM_H

#include <QWidget>
#include<QUrl>
#include<QFileDialog>
#include<QDebug>
#include<QMessageBox>
#include<QEvent>
#include"setlastbitedialog.h"
namespace Ui {
class senderSettingForm;
}

class senderSettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit senderSettingForm(QWidget *parent = nullptr);
    ~senderSettingForm();
    int isUseFileToSend();//是否使用文件原进行发送
    int isUseLastBite();//是否使用校验位
    int isSentToClearAuto();//发送完是否自动清空
    int isUseHexToSend();//是否使用16进制进行发送
    int isCircleToSend();//是否循环发送
    int getSpiltTime();//获取循环发送的时间间隔
    void changeEvent(QEvent *event) override;

signals:
    void clearInput();
    void fileIntoInput();
    void checkToUseFile(QUrl url);
    void unCheckToUseFile();
    void checkToSendLastBite(int i,uint16_t bite);
    void checkHexToSend();
    void checkToCircleSend(int splitTime);
    void unCircleToSend();

private:
    Ui::senderSettingForm *ui;
    setLastBiteDialog *dialog;
};

#endif // SENDERSETTINGFORM_H
