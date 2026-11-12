#ifndef QSSEDITORDIALOG_H
#define QSSEDITORDIALOG_H

#include <QDialog>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QTimer>

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
    void colorPicked(QAction*);
    bool isStyleSheetValid(const QString&);
    void validateStyleSheet();

private:
    void save();
    void hideEvent(QHideEvent *);
    Ui::QSSEditorDialog *m_ui;
    QString m_filePath;
    QMenu *colorMenu;
    QTimer *validateTimer;

};

#endif // QSSEDITORDIALOG_H
