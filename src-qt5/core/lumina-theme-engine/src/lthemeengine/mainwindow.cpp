#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QTimer>

#include "lthemeengine.h"
#include "mainwindow.h"
#include "appearancepage.h"
#include "fontspage.h"
#include "iconthemepage.h"
#include "cursorthemepage.h"
#include "interfacepage.h"
#include "qsspage.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), m_ui(new Ui::MainWindow){
  m_ui->setupUi(this);
  bgroup = new QButtonGroup(this);
  bgroup->setExclusive(true);
  //Clear out any pages in the stacked widget
  while(m_ui->stackedWidget->count()>0){
    m_ui->stackedWidget->removeWidget(m_ui->stackedWidget->widget(0));
  }
  //Now add all the pages into the widget
  bgroup->addButton(m_ui->tool_page_general, m_ui->stackedWidget->addWidget(new AppearancePage(this)) );
  bgroup->addButton(m_ui->tool_page_effects, m_ui->stackedWidget->addWidget(new InterfacePage(this)));
  bgroup->addButton(m_ui->tool_page_fonts, m_ui->stackedWidget->addWidget(new FontsPage(this)));
  bgroup->addButton(m_ui->tool_page_icons, m_ui->stackedWidget->addWidget(new IconThemePage(this)));
  bgroup->addButton(m_ui->tool_page_styles, m_ui->stackedWidget->addWidget(new QSSPage(this, false)));
  bgroup->addButton(m_ui->tool_page_deskstyles, m_ui->stackedWidget->addWidget(new QSSPage(this, true)));
  bgroup->addButton(m_ui->tool_page_cursors, m_ui->stackedWidget->addWidget(new CursorThemePage(this)) );
  connect(bgroup, SIGNAL(buttonClicked(int)), m_ui->stackedWidget, SLOT(setCurrentIndex(int)) );

  QTimer::singleShot(10, m_ui->tool_page_general, SLOT(toggle()));
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
    for(int i = 0; i < m_ui->stackedWidget->count(); ++i){
      TabPage *p = qobject_cast<TabPage*>(m_ui->stackedWidget->widget(i));
      if(p) { p->writeSettings(); }
      }
    }
  if(id == QDialogButtonBox::Ok || id == QDialogButtonBox::Cancel || id== QDialogButtonBox::Close){
    close();
    qApp->quit();
    }
}
