#ifndef FONTCONFIGDIALOG_H
#define FONTCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class FontConfigDialog;
}

class QXmlStreamWriter;

class FontConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontConfigDialog(QWidget *parent = 0);
    ~FontConfigDialog();

public slots:
    void accept();
    void writeOption(QXmlStreamWriter *stream, const QString &name, const QString &type, const QString &value);

private:
    Ui::FontConfigDialog *m_ui;
};

#endif // FONTCONFIGDIALOG_H
