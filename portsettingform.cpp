#include "portsettingform.h"
#include "ui_portsettingform.h"

portSettingForm::portSettingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::portSettingForm)
{
    ui->setupUi(this);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox->addItem(info.portName()); // COM加到UI下拉框里
    }
    connect(ui->openButton,&QPushButton::clicked,this,[=](){
        if(m_openPort == false)//ui->openButton->text() == "打开"
        {
            emit openPort(ui->comboBox->currentText());
            ui->openButton->setText(tr("取消监听"));
            m_openPort = true;
        }
        else if(m_openPort == true)//ui->openButton->text() == "取消监听"
        {
            emit closePort();
            ui->openButton->setText(tr("打开"));
            m_openPort = false;
        }


    });
}

portSettingForm::~portSettingForm()
{
    delete ui;
}

QString portSettingForm::getPortName()
{
    return ui->comboBox->currentText();
}

void portSettingForm::setPortName(QString pname)
{
    ui->comboBox->setCurrentText(pname);
}

void portSettingForm::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}


