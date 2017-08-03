#ifndef APPEARANCEPAGE_H
#define APPEARANCEPAGE_H

#include "tabpage.h"

namespace Ui {
class AppearancePage;
class PreviewForm;
}

class QStyle;
class QAction;

class AppearancePage : public TabPage{
    Q_OBJECT

public:
    explicit AppearancePage(QWidget *parent = 0);
    ~AppearancePage();
    void writeSettings();

private slots:
    void on_styleComboBox_activated(const QString &text);
    void on_colorSchemeComboBox_activated(int);
    void createColorScheme();
    void changeColorScheme();
    void removeColorScheme();
    void copyColorScheme();
    void renameColorScheme();
    void updatePalette();
    void setPreviewPalette(const QPalette &p);
    void updateActions();

private:
    void readSettings();
    void setStyle(QWidget *w, QStyle *s);
    void setPalette(QWidget *w, QPalette p);
    void findColorSchemes(QStringList paths);
    QPalette loadColorScheme(const QString &filePath);
    void createColorScheme(const QString &name, const QPalette &palette);
    Ui::AppearancePage *m_ui;
    QStyle *m_selectedStyle = nullptr;
    QPalette m_customPalette;
    QWidget *m_previewWidget;
    QAction *m_changeColorSchemeAction, *m_renameColorSchemeAction, *m_removeColorSchemeAction;
    Ui::PreviewForm *m_previewUi;
};

#endif // APPEARANCEPAGE_H
