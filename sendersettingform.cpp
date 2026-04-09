#include "sendersettingform.h"
#include "ui_sendersettingform.h"

senderSettingForm::senderSettingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::senderSettingForm)
{
    ui->setupUi(this);
    connect(ui->clearButton,&QPushButton::clicked,this,[=](){
        emit clearInput();
    });
    connect(ui->loadFileButton,&QPushButton::clicked,this,[=](){
        emit fileIntoInput();
    });
    connect(ui->checkBox,&QCheckBox::clicked,this,[=](bool isChanged){
        ui->checkBox->blockSignals(true);
        ui->checkBox->setChecked(!isChanged);
        ui->checkBox->blockSignals(false);
        if(ui->checkBox->checkState() == Qt::Unchecked)
        {
            ui->checkBox->setChecked(true);
            QUrl url = QFileDialog::getSaveFileUrl();
            if(!url.isEmpty())
            {
                emit checkToUseFile(url);
            }
        }else if(ui->checkBox->checkState() == Qt::Checked)
        {
            ui->checkBox->setChecked(false);
            emit unCheckToUseFile();
        }

    });
    connect(ui->checkBox_2,&QCheckBox::clicked,this,[=](){
//        ui->checkBox_2->blockSignals(true);
//        ui->checkBox_2->setChecked(!isChanged);
//        ui->checkBox_2->blockSignals(false);
        if(ui->checkBox_2->checkState() == Qt::Checked)
        {
            dialog = new setLastBiteDialog;
            dialog->show();
            connect(dialog,&setLastBiteDialog::CheckedItem,this,[=](int item,uint16_t bite){
                qDebug()<<item<<bite;
                if(item == 1 || item == 2)
                {
                    emit checkToSendLastBite(item,0);
                }else if(item == 3)
                {
                    emit checkToSendLastBite(item,bite);
                }
            });//回传mainwindow获取到的item和bite
        }

    });//展示dialog并尝试获取item和bite

    connect(ui->checkBox_4,&QCheckBox::clicked,this,[=](){
        emit checkHexToSend();
    });
    connect(ui->checkBox_5,&QCheckBox::clicked,this,[=](bool isChanged){
        ui->checkBox_5->blockSignals(true);
        ui->checkBox_5->setChecked(!isChanged);
        ui->checkBox_5->blockSignals(false);
        if(ui->splitTimeEdit->text().contains(QRegExp("^[0-9]+$")) && ui->checkBox_5->checkState() == Qt::Unchecked)
        {
            int splitTime = ui->splitTimeEdit->text().toInt();
            emit checkToCircleSend(splitTime);
            ui->checkBox_5->setChecked(true);
        }else if(!ui->splitTimeEdit->text().contains(QRegExp("^[0-9]+$"))){
            QMessageBox::critical(this,"错误","时间设置错误");
        }else if(ui->checkBox_5->checkState() == Qt::Checked)
        {
            ui->checkBox_5->setChecked(false);
            emit unCircleToSend();
        }
    });//循环发送事件
}

senderSettingForm::~senderSettingForm()
{
    delete ui;
}

int senderSettingForm::isUseFileToSend()
{
    return ui->checkBox->checkState();
}

int senderSettingForm::isUseLastBite()
{
    return ui->checkBox_2->checkState();
}

int senderSettingForm::isSentToClearAuto()
{
    return ui->checkBox_3->checkState();
}

int senderSettingForm::isUseHexToSend()
{
    return ui->checkBox_4->checkState();
}

int senderSettingForm::isCircleToSend()
{
    return ui->checkBox_5->checkState();
}

int senderSettingForm::getSpiltTime()
{
    return ui->splitTimeEdit->text().toInt();
}

void senderSettingForm::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
