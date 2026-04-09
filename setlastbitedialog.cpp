#include "setlastbitedialog.h"
#include "ui_setlastbitedialog.h"

setLastBiteDialog::setLastBiteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setLastBiteDialog)
{
    ui->setupUi(this);
    connect(ui->yesButton,&QPushButton::clicked,this,[=](){
        if(ui->radioButton->isChecked() == true)
        {
            emit CheckedItem(1);
        }else if(ui->radioButton_2->isChecked() == true)
        {
            emit CheckedItem(2);
        }else if(ui->radioButton_3->isChecked() == true)
        {
            if(ui->lineEdit->text().isEmpty())
            {
                emit CheckedItem(3,0);
            }else{
                emit CheckedItem(3,ui->lineEdit->text().toUInt());
            }
        }
        this->close();
    });

    connect(ui->cancelButton,&QPushButton::clicked,this,[=](){
        this->close();
    });
}

setLastBiteDialog::~setLastBiteDialog()
{
    delete ui;
}

void setLastBiteDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
