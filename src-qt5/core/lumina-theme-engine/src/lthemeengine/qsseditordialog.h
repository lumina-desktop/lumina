#ifndef QSSEDITORDIALOG_H
#define QSSEDITORDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class QSSEditorDialog;
}

class QAbstractButton;

class QSSEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QSSEditorDialog(const QString &filePath, QWidget *parent = 0);
    ~QSSEditorDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    void save();
    void hideEvent(QHideEvent *);
    Ui::QSSEditorDialog *m_ui;
    QString m_filePath;
};

#endif // QSSEDITORDIALOG_H
