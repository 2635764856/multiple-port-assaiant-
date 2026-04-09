#include "modbusmasterdialog.h"
#include "ui_modbusmasterdialog.h"

modbusMasterDialog::modbusMasterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::modbusMasterDialog)
{
    ui->setupUi(this);
    smodel = new QStandardItemModel(this);
    tmodel = new QStandardItemModel(this);

    ui->tableView->verticalHeader()->hide();
    ui->tableView->setShowGrid(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->connectionComboBox,&QComboBox::currentTextChanged,this,[=](){
        if(ui->connectionComboBox->currentText() == "Serial")
        {
            connectionChoice = 0;
            ui->lineEdit->setText("COM1");
        }else if(ui->connectionComboBox->currentText() == "TCP")
        {
            connectionChoice = 1;
            ui->lineEdit->setText("127.0.0.1:502");
        }
    });//连接方式选择
    ui->memSelectComboBox->addItem("Coils",QModbusDataUnit::Coils);
    ui->memSelectComboBox->addItem("Discrete Input",QModbusDataUnit::DiscreteInputs);
    ui->memSelectComboBox->addItem("Input Registers",QModbusDataUnit::InputRegisters);
    ui->memSelectComboBox->addItem("Holding Registers",QModbusDataUnit::HoldingRegisters);
    connect(ui->connectButton,&QPushButton::clicked,this,[=](){
        if(!isConnected)
        {
            switch (connectionChoice) {
                case 0:
                    modbusDevice = new QModbusRtuSerialMaster(this);
                    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter
                                                         ,ui->lineEdit->text());
                    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter
                                                         ,QSerialPort::Baud115200);
                    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter
                                                         ,QSerialPort::Data8);
                    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter
                                                         ,QSerialPort::OneStop);
                    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter
                                                         ,QSerialPort::NoParity);
                    break;
                case 1:
                    modbusDevice = new QModbusTcpClient(this);
                    QUrl url = QUrl::fromUserInput(ui->lineEdit->text());
                    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter,url.host());
                    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter,url.port());
                    break;
            }
            connect(modbusDevice,&QModbusDevice::stateChanged,this,[=](QModbusDevice::State state){
                if(state == QModbusDevice::ConnectedState)
                {
                    qDebug()<<"连接成功";
                    ui->connectButton->setText(tr("断开连接"));
                    isConnected = true;
                }else if(state == QModbusDevice::UnconnectedState){
                    qDebug()<<"连接断开";
                    isConnected = false;
                    ui->connectButton->setText(tr("连接"));
                    modbusDevice->deleteLater();
                }

            });
            if(modbusDevice->connectDevice())
            {
                qDebug()<<"正在连接。。。";
            }else{
                QMessageBox::critical(this,tr("错误"),modbusDevice->errorString());
                modbusDevice->deleteLater();
            }
        }else{
            modbusDevice->disconnectDevice();
        }
    });//连接按钮逻辑
    connect(ui->memSelectComboBox,&QComboBox::currentTextChanged,this,[=](){
        if(ui->memSelectComboBox->currentData() == QModbusDataUnit::Coils
                || ui->memSelectComboBox->currentData() == QModbusDataUnit::DiscreteInputs)
        {
            tmodel->clear();
            tmodel->setHorizontalHeaderLabels(QStringList()<<"#"<<"Value");//设置写入区的title
            for(int i = 0;i < 10; i++)
            {
                QStandardItem *checkItem = new QStandardItem;
                checkItem->setCheckable(true);
                checkItem->setCheckState(Qt::Unchecked);
                QStandardItem *index = new QStandardItem(QString::number(i));
                QList<QStandardItem*> list;
                list<<index<<checkItem;
                tmodel->appendRow(list);
            }
            if(ui->memSelectComboBox->currentData() == QModbusDataUnit::DiscreteInputs)
            {
                ui->treeView->setEnabled(false);
            }else{
                ui->treeView->setEnabled(true);
            }
            ui->treeView->setModel(tmodel);
            ui->treeView->show();
        }else if(ui->memSelectComboBox->currentData() == QModbusDataUnit::InputRegisters
                 || ui->memSelectComboBox->currentData() == QModbusDataUnit::HoldingRegisters)
        {
            tmodel->clear();
            tmodel->setHorizontalHeaderLabels(QStringList()<<"#"<<"Value");//设置写入区的title
            for(int i = 0;i < 10; i++)
            {
                QStandardItem *checkItem = new QStandardItem;
                checkItem->setEditable(true);
                checkItem->setText(QString("0x00"));
                QStandardItem *index = new QStandardItem(QString::number(i));
                QList<QStandardItem*> list;
                list<<index<<checkItem;
                tmodel->appendRow(list);
            }
            if(ui->memSelectComboBox->currentData() == QModbusDataUnit::InputRegisters)
            {
                ui->treeView->setEnabled(false);
            }else{
                ui->treeView->setEnabled(true);
            }
            setDisplay();
            ui->treeView->setModel(tmodel);
            ui->treeView->show();
        }


    });
    connect(ui->readButton,&QPushButton::clicked,this,[=](){
        if(!isConnected)
        {
            QMessageBox::critical(this,tr("错误"),tr("连接错误"));
            return;
        }
        readData();
    });
    connect(ui->writeButton,&QPushButton::clicked,this,[=](){
        if(!isConnected)
        {
            QMessageBox::critical(this,tr("错误"),tr("连接错误"));
            return;
        }
        setData();
    });
    connect(ui->writeCountComboBox,&QComboBox::currentTextChanged,this,&modbusMasterDialog::setDisplay);
    connect(ui->writeStartSpinBox,QOverload<int>::of(&QSpinBox::valueChanged),this,&modbusMasterDialog::setDisplay);

}

modbusMasterDialog::~modbusMasterDialog()
{
    delete ui;
}

void modbusMasterDialog::readData()
{
    smodel->clear();//清除smodel中的数据，避免追加
    smodel->setHorizontalHeaderLabels(QStringList()<<"#"<<"Value");
    QModbusDataUnit::RegisterType type = static_cast<QModbusDataUnit::RegisterType>(ui->memSelectComboBox->currentData().toInt());
    quint16 readStart = ui->readStartSpinBox->value();
    quint16 readCount = ui->readCountComboBox->currentText().toUInt();
    const QModbusDataUnit unit(type
                               ,readStart
                               ,readCount);
    int serverAddress = ui->slaveSpinBox->value();
    if(auto *reply = modbusDevice->sendReadRequest(unit,serverAddress))
    {
        if(!reply->isFinished())
        {
            connect(reply,&QModbusReply::finished,this,[=](){
                if(reply->error() == QModbusDevice::NoError)
                {
                    const QModbusDataUnit dataUnit = reply->result();
                    for(int i = 0;i < readCount;i++)
                    {
//                        qDebug()<<dataUnit.startAddress()+i<<dataUnit.value(i);
                        QList<QStandardItem*> list;
                        QStandardItem *item = new QStandardItem(QString("%1").arg(dataUnit.startAddress()+i));
                        QStandardItem *item2 = new QStandardItem(QString("%1").arg(dataUnit.value(i),2,16).toUpper());
                        item->setTextAlignment(Qt::AlignHCenter);
                        item2->setTextAlignment(Qt::AlignHCenter);
                        list<<item<<item2;
                        smodel->appendRow(list);
                    }
                    ui->tableView->setModel(smodel);
                    ui->tableView->show();
                }
            });
        }
    }



}//读从机的数据

void modbusMasterDialog::setData()
{
    QModbusDataUnit::RegisterType type = static_cast<QModbusDataUnit::RegisterType>(ui->memSelectComboBox->currentData().toInt());
    quint16 writeStart = ui->writeStartSpinBox->value();
    quint16 writeCount = ui->writeCountComboBox->currentText().toUInt();
    QModbusDataUnit unit(type
                               ,writeStart
                               ,writeCount);
    int serverAddress = ui->slaveSpinBox->value();
    if(writeCount + writeStart > 10)
    {
        QMessageBox::critical(this,tr("错误"),tr("范围选择错误"));
        return;
    }
    for(int i = 0;i < writeCount;i++)
    {
        int row = i + writeStart;
        QStandardItem* item = tmodel->item(row,1);
        if(item)
        {
            quint16 valueToWrite = 0;
            if(type == QModbusDataUnit::Coils)
            {
                valueToWrite = item->checkState() == Qt::Checked ? 1 :0 ;
            }else if(type == QModbusDataUnit::HoldingRegisters)
            {
                bool ok;
                valueToWrite = item->text().toUShort(&ok,0);
                if(!ok)
                {
                    QMessageBox::critical(this,tr("错误"),tr("参数设置错误"));
                    return;
                }

            }
            unit.setValue(i,valueToWrite);
        }
    }

    if(auto *reply = modbusDevice->sendWriteRequest(unit,serverAddress))
    {
        if(!reply->isFinished())
        {
            connect(reply,&QModbusReply::finished,this,[=](){
                if(reply->error() == QModbusDevice::NoError)
                {
                    QMessageBox::information(this,tr("成功"),tr("数据修改成功"));
                }else {
                    QMessageBox::critical(this,tr("失败"),tr("数据修改失败"));
                }
                reply->deleteLater();
            });
        }else{
            reply->deleteLater();
        }

    }else{
        QMessageBox::critical(this,tr("发送失败"),modbusDevice->errorString());
    }

}

void modbusMasterDialog::setDisplay()
{
    if(!tmodel) return;
    int startRow = ui->writeStartSpinBox->value();
    int count = ui->writeCountComboBox->currentText().toUInt();
    for(int i = 0;i<tmodel->rowCount();i++)
    {
        bool isShow = i >= startRow && i < startRow+count;
        QStandardItem *item1 = tmodel->item(i,0);
        QStandardItem *item2 = tmodel->item(i,1);
        item1->setEnabled(isShow);
        item2->setEnabled(isShow);
    }


}

void modbusMasterDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
