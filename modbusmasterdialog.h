#ifndef MODBUSMASTERDIALOG_H
#define MODBUSMASTERDIALOG_H

#include <QDialog>
#include<QModbusTcpClient>
#include<QModbusDataUnit>
#include<QModbusRtuSerialMaster>
#include<QModbusReply>
#include<QSerialPort>
#include<QUrl>
#include<QDebug>
#include<QMessageBox>
#include<QStringListModel>
#include<QListView>
#include<QStandardItem>
#include<QStandardItemModel>
#include<QList>
#include<QCheckBox>
#include<QSpinBox>
#include<QEvent>
namespace Ui {
class modbusMasterDialog;
}

class modbusMasterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit modbusMasterDialog(QWidget *parent = nullptr);
    ~modbusMasterDialog();
    void readData();
    void setData();
    void setDisplay();
    void changeEvent(QEvent *event) override;

private:
    Ui::modbusMasterDialog *ui;
    QModbusClient *modbusDevice;
    int connectionChoice = 0;
    bool isConnected = false;
    QStandardItemModel *smodel;//tableview的model
    QStandardItemModel *tmodel;//treeview的model
};

#endif // MODBUSMASTERDIALOG_H
