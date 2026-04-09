#include "receviersettingform.h"
#include "ui_receviersettingform.h"

recevierSettingForm::recevierSettingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::recevierSettingForm)
{
    ui->setupUi(this);
    connect(ui->clearButton,&QPushButton::clicked,this,[=](){
        emit clearBroswer();
    });
    connect(ui->saveDataButton,&QPushButton::clicked,this,[=](){
        QFileDialog *fd = new QFileDialog(this);

        emit saveText(fd->getSaveFileUrl());
        delete fd;
    });
    connect(ui->checkBox,&QCheckBox::clicked,this,[=](bool isChecked){
        ui->checkBox->blockSignals(true);
        ui->checkBox->setChecked(!isChecked);
        ui->checkBox->blockSignals(false);
        if(ui->checkBox->checkState() == Qt::Unchecked)
        {
            QUrl choosedUrl = QFileDialog::getSaveFileUrl(this,"保存文件路径");
            qDebug()<<choosedUrl;
            if(!choosedUrl.isEmpty())
            {
                emit outputIntoFile(choosedUrl);
                ui->checkBox->setChecked(true);
                ui->clearButton->setEnabled(false);
            }

        }else if(ui->checkBox->checkState() == Qt::Checked){
            emit releaseBroswer();
            ui->checkBox->setChecked(false);
            ui->clearButton->setEnabled(true);
        }

    });
    connect(ui->checkBox_2,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_2->checkState() == Qt::Checked)
        {
            emit changeLineShow();
        }
    });
    connect(ui->checkBox_4,&QCheckBox::clicked,this,[=](){

        emit setBroswerDisable(ui->checkBox_4->checkState());

    });
}

recevierSettingForm::~recevierSettingForm()
{
    delete ui;
}

int recevierSettingForm::getStatusOfOutput()
{
    return ui->checkBox->checkState();
}

int recevierSettingForm::getIsChangeLine()
{
    return ui->checkBox_2->checkState();
}

int recevierSettingForm::getIsChangeHex()
{
    return ui->checkBox_3->checkState();
}

int recevierSettingForm::getIsShow()
{
    return ui->checkBox_4->checkState();
}

void recevierSettingForm::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}


