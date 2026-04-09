#ifndef PORTSETTINGFORM_H
#define PORTSETTINGFORM_H

#include <QWidget>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QDebug>
#include<QEvent>
namespace Ui {
class portSettingForm;
}

class portSettingForm : public QWidget
{
    Q_OBJECT
public:
    explicit portSettingForm(QWidget *parent = nullptr);
    ~portSettingForm();
    QString getPortName();
    void setPortName(QString pname);
    void changeEvent(QEvent *event) override;
signals:
    void openPort(QString portName);
    void closePort();
private:
    Ui::portSettingForm *ui;
    bool m_openPort = false;

};

#endif // PORTSETTINGFORM_H
