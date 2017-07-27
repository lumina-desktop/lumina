#ifndef FONTSPAGE_H
#define FONTSPAGE_H

#include "tabpage.h"

namespace Ui {
class FontsPage;
}

class QLabel;
class QSettings;

class FontsPage : public TabPage
{
    Q_OBJECT

public:
    explicit FontsPage(QWidget *parent = 0);
    ~FontsPage();

    void writeSettings();

private slots:
    void onFontChangeRequested(QWidget *widget);
    void on_createFontsConfButton_clicked();
    void on_removeFontsConfButton_clicked();

private:
    void readSettings();
    void loadFont(QSettings *settings, QLabel *label, const QString &key);
    Ui::FontsPage *m_ui;
};

#endif // FONTSPAGE_H
