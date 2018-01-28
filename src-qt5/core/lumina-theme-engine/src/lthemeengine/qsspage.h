#ifndef QSSPAGE_H
#define QSSPAGE_H

#include "tabpage.h"

namespace Ui {
class QSSPage;
}

class QListWidgetItem;
class QMenu;

class QSSPage : public TabPage
{
    Q_OBJECT

public:
    explicit QSSPage(QWidget *parent = 0, bool desktop = false);
    ~QSSPage();

    void writeSettings();

private slots:
    void on_qssListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *);
    void on_list_disabled_currentItemChanged(QListWidgetItem *current, QListWidgetItem *);
    void on_createButton_clicked();
    void on_copyButton_clicked();
    void on_editButton_clicked();
    void on_removeButton_clicked();
    void on_renameButton_clicked();
    void on_qssListWidget_customContextMenuRequested(const QPoint &pos);

	void on_tool_enable_clicked();
	void on_tool_disable_clicked();
	void on_tool_priority_up_clicked();
	void on_tool_priority_down_clicked();

private:
    void readSettings();
    void findStyleSheets(QStringList paths, QStringList enabled);
    Ui::QSSPage *m_ui;
    QMenu *m_menu;
    bool desktop_qss;

	QListWidgetItem* currentSelection();
};

#endif // QSSPAGE_H
