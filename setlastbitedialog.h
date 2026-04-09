#ifndef SETLASTBITEDIALOG_H
#define SETLASTBITEDIALOG_H

#include <QDialog>
#include<QEvent>
namespace Ui {
class setLastBiteDialog;
}

class setLastBiteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setLastBiteDialog(QWidget *parent = nullptr);
    ~setLastBiteDialog();
    void changeEvent(QEvent *event) override;
signals:
    void CheckedItem(int i, uint16_t bite = 0);

private:
    Ui::setLastBiteDialog *ui;
};

#endif // SETLASTBITEDIALOG_H
