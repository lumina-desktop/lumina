#include <QSettings>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QTreeWidgetItem>
#include <QImageReader>
#include <QLocale>
#include "lthemeengine.h"
#include "iconthemepage.h"
#include "ui_iconthemepage.h"

IconThemePage::IconThemePage(QWidget *parent) : TabPage(parent), m_ui(new Ui::IconThemePage){
  m_ui->setupUi(this);
  loadThemes();
  readSettings();
}

IconThemePage::~IconThemePage(){
  delete m_ui;
}

void IconThemePage::writeSettings(){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  QTreeWidgetItem *item = m_ui->treeWidget->currentItem();
  if(item){ settings.setValue("Appearance/icon_theme", item->data(3, Qt::UserRole)); }
}

void IconThemePage::readSettings(){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  QString name = settings.value("Appearance/icon_theme").toString();
  if(name.isEmpty()){ return; }
  for(int i = 0; i < m_ui->treeWidget->topLevelItemCount(); ++i){
    QTreeWidgetItem *item = m_ui->treeWidget->topLevelItem(i);
    if(item->data(3, Qt::UserRole).toString() == name){
      m_ui->treeWidget->setCurrentItem(item);
      break;
      }
    }
}

void IconThemePage::loadThemes(){
  QFileInfoList themeFileList;
  foreach(QString path, lthemeengine::iconPaths()){
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::NoDotDot | QDir::NoDot);
    foreach (QFileInfo info, dir.entryInfoList()){
      QDir themeDir(info.absoluteFilePath());
      themeDir.setFilter(QDir::Files);
      themeFileList << themeDir.entryInfoList(QStringList() << "index.theme");
      }
    }
  foreach(QFileInfo info, themeFileList){
    loadTheme(info.canonicalFilePath());
    }
}

void IconThemePage::loadTheme(const QString &path){
  QSettings config(path, QSettings::IniFormat);
  config.setIniCodec("UTF-8");
  config.beginGroup("Icon Theme");
  QStringList dirs = config.value("Directories").toStringList();
  if(dirs.isEmpty() || config.value("Hidden", false).toBool()){ return; }
  QString name, comment;
  QString lang = QLocale::system().name();
  name = config.value(QString("Name[%1]").arg(lang)).toString();
  comment = config.value(QString("Comment[%1]").arg(lang)).toString();
  if(lang.contains("_")){ lang = lang.split("_").first(); }
  if(name.isEmpty()){ name = config.value(QString("Name[%1]").arg(lang)).toString(); }
  if(comment.isEmpty()){ comment = config.value(QString("Comment[%1]").arg(lang)).toString(); }
  if(name.isEmpty()){ name = config.value("Name").toString(); }
  if(comment.isEmpty()){ comment = config.value("Comment").toString(); }
  config.endGroup();
  QIcon icon1 = findIcon(path, 24, "document-save");
  QIcon icon2 = findIcon(path, 24, "document-print");
  QIcon icon3 = findIcon(path, 24, "document-edit");
  QTreeWidgetItem *item = new QTreeWidgetItem();
  item->setIcon(0, icon1);
  item->setIcon(1, icon2);
  item->setIcon(2, icon3);
  item->setText(3, name);
  item->setData(3, Qt::UserRole, QFileInfo(path).path().section("/", -1));
  item->setToolTip(3, comment);
  item->setSizeHint(0, QSize(24,24));
  m_ui->treeWidget->addTopLevelItem(item);
  m_ui->treeWidget->resizeColumnToContents(0);
  m_ui->treeWidget->resizeColumnToContents(1);
  m_ui->treeWidget->resizeColumnToContents(2);
  m_ui->treeWidget->resizeColumnToContents(3);
}

QIcon IconThemePage::findIcon(const QString &themePath, int size, const QString &name){
  QSettings config(themePath, QSettings::IniFormat);
  config.beginGroup("Icon Theme");
  QStringList dirs = config.value("Directories").toStringList();
  QStringList parents = config.value("Inherits").toStringList();
  bool haveInherits = config.contains("Inherits");
  config.endGroup();
  foreach (QString dir, dirs){
    config.beginGroup(dir);
    if(config.value("Size").toInt() == size || config.value("Type").toString().toLower()=="scalable"){
      QDir iconDir = QFileInfo(themePath).path() + "/" + dir;
      iconDir.setFilter(QDir::Files);
      iconDir.setNameFilters(QStringList () << name + ".*");
      if(iconDir.entryInfoList().isEmpty())
        continue;
        return QIcon(iconDir.entryInfoList().first().absoluteFilePath());
      }
      config.endGroup();
    }
  foreach (QString dir, dirs){
  config.beginGroup(dir);
  if(abs(config.value("Size").toInt() - size) < 4){
    QDir iconDir = QFileInfo(themePath).path() + "/" + dir;
    iconDir.setFilter(QDir::Files);
    iconDir.setNameFilters(QStringList () << name + ".*");
    if(iconDir.entryInfoList().isEmpty())
      continue;
      return QIcon(iconDir.entryInfoList().first().absoluteFilePath());
    }
    config.endGroup();
    }
  if (!haveInherits){ return QIcon(); }
  parents.append("hicolor"); //add fallback themes
  parents.append("gnome");
  parents.removeDuplicates();
  foreach (QString parent, parents){
    QString parentThemePath = QDir(QFileInfo(themePath).path() + "/../" + parent).canonicalPath() + "/index.theme";
    if(!QFile::exists(parentThemePath) || parentThemePath == themePath){ continue; }
    QIcon icon = findIcon(parentThemePath, size, name);
    if(!icon.isNull()){ return icon; }
    }
  return QIcon();
}
