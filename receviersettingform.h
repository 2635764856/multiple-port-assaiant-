#ifndef RECEVIERSETTINGFORM_H
#define RECEVIERSETTINGFORM_H

#include <QWidget>
#include<QFileDialog>
#include<QUrl>
#include<QDebug>
#include<QEvent>
namespace Ui {
class recevierSettingForm;
}

class recevierSettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit recevierSettingForm(QWidget *parent = nullptr);
    ~recevierSettingForm();
    int getStatusOfOutput();
    int getIsChangeLine();
    int getIsChangeHex();
    int getIsShow();
    void changeEvent(QEvent *event)override;

signals:
    void clearBroswer();//清空输入框
    void saveText(QUrl path);//保存接收区已经收到的文本到path路径
    void outputIntoFile(QUrl path);//将后面需要显示到broswer的信息保存到path路径
    void releaseBroswer();//释放broswer，设置为可显示
    void changeLineShow();
    void setBroswerDisable(Qt::CheckState);

private:
    Ui::recevierSettingForm *ui;
};

#endif // RECEVIERSETTINGFORM_H
