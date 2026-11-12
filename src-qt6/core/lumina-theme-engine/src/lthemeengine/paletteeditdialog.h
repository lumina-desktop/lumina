#ifndef PALETTEEDITDIALOG_H
#define PALETTEEDITDIALOG_H

#include <QDialog>
#include <QPalette>

class QTableWidgetItem;
class QStyle;

namespace Ui {
class PaletteEditDialog;
}

class PaletteEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteEditDialog(const QPalette &palette, QStyle *currentStyle, QWidget *parent = 0);
    ~PaletteEditDialog();

    QPalette selectedPalette() const;

signals:
    void paletteChanged(const QPalette &p);

private slots:
    void on_tableWidget_itemClicked(QTableWidgetItem *item);
    void on_resetPaletteButton_clicked();
    void on_buildInactiveButton_clicked();
    void on_buildDisabledButton_clicked();

private:
    void setPalette(const QPalette &palette);
    void hideEvent(QHideEvent *);
    Ui::PaletteEditDialog *m_ui;
    QStyle *m_currentStyle;
};

#endif // PALETTEEDITDIALOG_H
