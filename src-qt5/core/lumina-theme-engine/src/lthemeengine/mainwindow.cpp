#include <QApplication>
#include <QSettings>
#include "lthemeengine.h"
#include "mainwindow.h"
#include "appearancepage.h"
#include "fontspage.h"
#include "iconthemepage.h"
#include "interfacepage.h"
#include "qsspage.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), m_ui(new Ui::MainWindow){
  m_ui->setupUi(this);
  m_ui->tabWidget->addTab(new AppearancePage(this), tr("Appearance"));
  m_ui->tabWidget->addTab(new FontsPage(this), tr("Fonts"));
  m_ui->tabWidget->addTab(new IconThemePage(this), tr("Icon Theme"));
  m_ui->tabWidget->addTab(new InterfacePage(this), tr("Interface"));
#ifdef USE_WIDGETS
  m_ui->tabWidget->addTab(new QSSPage(this, false), tr("Application Theme"));
  m_ui->tabWidget->addTab(new QSSPage(this, true), tr("Desktop Theme"));
#endif
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  restoreGeometry(settings.value("SettingsWindow/geometry").toByteArray());
  setWindowIcon(QIcon::fromTheme("preferences-desktop-theme"));
  m_ui->versionLabel->setText(tr("Version: %1").arg(LTHEMEENGINE_VERSION_STR));
}

MainWindow::~MainWindow(){
  delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent *){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.setValue("SettingsWindow/geometry", saveGeometry());
}

void MainWindow::on_buttonBox_clicked(QAbstractButton *button){
  int id = m_ui->buttonBox->standardButton(button);
  if(id == QDialogButtonBox::Ok || id == QDialogButtonBox::Apply){
    for(int i = 0; i < m_ui->tabWidget->count(); ++i){
      TabPage *p = qobject_cast<TabPage*>(m_ui->tabWidget->widget(i));
      if(p) { p->writeSettings(); }
      }
    }
  if(id == QDialogButtonBox::Ok || id == QDialogButtonBox::Cancel){
    close();
    qApp->quit();
    }
}
