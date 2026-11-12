#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QFontDialog>
#include <QDir>
#include <QFile>
#include "lthemeengine.h"
#include "fontspage.h"
#include "fontconfigdialog.h"
#include "ui_fontspage.h"

FontsPage::FontsPage(QWidget *parent) : TabPage(parent), m_ui(new Ui::FontsPage){
  m_ui->setupUi(this);

  connect(m_ui->changeGeneralFontButton, &QToolButton::clicked, [this](){onFontChangeRequested(m_ui->generalFontLabel);});
  connect(m_ui->changeFixedWidthFontButton, &QToolButton::clicked, [this](){onFontChangeRequested(m_ui->fixedFontLabel);});
  readSettings();

  //icons
  m_ui->createFontsConfButton->setIcon(QIcon::fromTheme("document-new"));
  m_ui->removeFontsConfButton->setIcon(QIcon::fromTheme("edit-delete"));
}

FontsPage::~FontsPage(){
  delete m_ui;
}

void FontsPage::writeSettings(){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.beginGroup("Fonts");
  settings.setValue("general", m_ui->generalFontLabel->font());
  settings.setValue("fixed", m_ui->fixedFontLabel->font());
  settings.endGroup();
}

void FontsPage::onFontChangeRequested(QWidget *widget){
  bool ok = false;
  QFont font = QFontDialog::getFont (&ok, widget->font(), this);
  if(ok){
    widget->setFont(font);
    qobject_cast<QLabel*>(widget)->setText(font.family () + " " + QString::number(font.pointSize ()));
    }
}

void FontsPage::readSettings(){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.beginGroup("Fonts");
  loadFont(&settings, m_ui->generalFontLabel, "general");
  loadFont(&settings, m_ui->fixedFontLabel, "fixed");
  settings.endGroup();
}

void FontsPage::loadFont(QSettings *settings, QLabel *label, const QString &key){
  QFont font = settings->value(key, QApplication::font()).value<QFont>();
  label->setText(font.family () + " " + QString::number(font.pointSize ()));
  label->setFont(font);
}

void FontsPage::on_createFontsConfButton_clicked(){
  FontConfigDialog d(this);
  d.exec();
}

void FontsPage::on_removeFontsConfButton_clicked(){
  QString path = QDir::homePath() + "/.config/fontconfig/fonts.conf";
  if(QFile::exists(path)){
  if(QMessageBox::question(this, tr("Remove Font Configuration"), tr("Are you sure you want to delete <i>%1</i>?").arg(path), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
    return;
    }
  QFile::remove(path + ".back");
  QFile::copy(path, path + ".back");
  QFile::remove(path);
  }
}
