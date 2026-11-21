#ifndef QSSEDITORDIALOG_H
#define QSSEDITORDIALOG_H

#include <QDialog>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QProcess>
#include <QTemporaryFile>

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
    void validateStyleSheet(); //starts an asynchronous check (lthemeengine-sstest)
    void validationFinished(); //updates the status icon from the check's output

private:
    void save();
    void hideEvent(QHideEvent *);
    Ui::QSSEditorDialog *m_ui;
    QString m_filePath;
    QMenu *colorMenu;
    QTimer *validateTimer;
    QProcess *validateProc; //runs lthemeengine-sstest without blocking the editor
    QTemporaryFile *validateFile; //kept alive until the check has read it

};

#endif // QSSEDITORDIALOG_H
