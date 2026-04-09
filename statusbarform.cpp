#include "statusbarform.h"
#include "ui_statusbarform.h"

statusBarForm::statusBarForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::statusBarForm)
{
    ui->setupUi(this);
    connect(ui->portCountResetButton,&QPushButton::clicked,this,[=](){
        emit portCountReset();
    });
}

statusBarForm::~statusBarForm()
{
    delete ui;

}

void statusBarForm::setPsend(int num)
{
    ui->portSendLabel->setText(QString(tr("已发送:%1")).arg(num));
}

void statusBarForm::setPrecive(int num)
{
    ui->portReciveLabel->setText(QString(tr("已收到:%1")).arg(num));
}

void statusBarForm::setNsend(int num)
{
    ui->netSendLabel->setText(QString(tr("已发送:%1")).arg(num));
}

void statusBarForm::setNrecive(int num)
{
    ui->netReciveLabel->setText(QString(tr("已收到:%1")).arg(num));
}

void statusBarForm::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);

}

void statusBarForm::setInfo(QString str)
{
    ui->netStatusLabel->setText(str);
    ui->portStatusLabel->setText(str);
}

void statusBarForm::on_netCountResetButton_clicked()
{
    emit netCountReset();
}

