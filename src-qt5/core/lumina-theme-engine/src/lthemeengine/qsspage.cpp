#include <QSettings>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QMenu>
#include "lthemeengine.h"
#include "qsseditordialog.h"
#include "qsspage.h"
#include "ui_qsspage.h"

#define QSS_FULL_PATH_ROLE (Qt::ItemDataRole(Qt::UserRole))
#define QSS_WRITABLE_ROLE (Qt::ItemDataRole(Qt::UserRole + 1))

QSSPage::QSSPage(QWidget *parent) : TabPage(parent), m_ui(new Ui::QSSPage){
  m_ui->setupUi(this);
  QDir("/").mkpath(lthemeengine::userStyleSheetPath());
  m_menu = new QMenu(this);
  m_menu->addAction(QIcon::fromTheme("accessories-text-editor"), tr("Edit"), this, SLOT(on_editButton_clicked()));
  m_menu->addAction(tr("Rename"), this, SLOT(on_renameButton_clicked()));
  m_menu->addSeparator();
  m_menu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, SLOT(on_removeButton_clicked()));
  readSettings();
  //icons
  m_ui->createButton->setIcon(QIcon::fromTheme("document-new"));
  m_ui->editButton->setIcon(QIcon::fromTheme("accessories-text-editor"));
  m_ui->removeButton->setIcon(QIcon::fromTheme("edit-delete"));
}

QSSPage::~QSSPage(){
  delete m_ui;
}

void QSSPage::writeSettings(){
  QStringList styleSheets;
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  for(int i = 0; i < m_ui->qssListWidget->count(); ++i){
    QListWidgetItem *item = m_ui->qssListWidget->item(i);
    if(item->checkState() == Qt::Checked)
      styleSheets << item->data(QSS_FULL_PATH_ROLE).toString();
    }
  settings.setValue("Interface/stylesheets", styleSheets);
}

void QSSPage::on_qssListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *){
  if(current){
    m_ui->editButton->setEnabled(current->data(QSS_WRITABLE_ROLE).toBool());
    m_ui->removeButton->setEnabled(current->data(QSS_WRITABLE_ROLE).toBool());
    m_ui->renameButton->setEnabled(current->data(QSS_WRITABLE_ROLE).toBool());
    }
  else{
    m_ui->editButton->setEnabled(false);
    m_ui->removeButton->setEnabled(false);
    m_ui->renameButton->setEnabled(false);
    }
}

void QSSPage::on_createButton_clicked(){
  QString name = QInputDialog::getText(this, tr("Enter Style Sheet Name"), tr("File name:"));
  if(name.isEmpty())
    return;
    if(!name.endsWith(".qss", Qt::CaseInsensitive))
      name.append(".qss");
      QString filePath = lthemeengine::userStyleSheetPath() + name;
      if(QFile::exists(filePath)){
        QMessageBox::warning(this, tr("Error"), tr("The file \"%1\" already exists").arg(filePath));
        return;
        }
      //creating empty file
      QFile file(filePath);
      file.open(QIODevice::WriteOnly);
      file.close();
      //creating item
      QFileInfo info(filePath);
      QListWidgetItem *item = new QListWidgetItem(info.fileName(),  m_ui->qssListWidget);
      item->setToolTip(info.filePath());
      item->setData(QSS_FULL_PATH_ROLE, info.filePath());
      item->setData(QSS_WRITABLE_ROLE, info.isWritable());
      item->setCheckState(Qt::Unchecked);
}

void QSSPage::on_editButton_clicked(){
  QListWidgetItem *item = m_ui->qssListWidget->currentItem();
  if(item){
    QSSEditorDialog dialog(item->data(QSS_FULL_PATH_ROLE).toString(), this);
    dialog.exec();
    }
}

void QSSPage::on_removeButton_clicked(){
  QListWidgetItem *item = m_ui->qssListWidget->currentItem();
  if(!item)
    return;
    int button = QMessageBox::question(this, tr("Confirm Remove"),tr("Are you sure you want to remove style sheet \"%1\"?").arg(item->text()), QMessageBox::Yes | QMessageBox::No);
    if(button == QMessageBox::Yes){
      QFile::remove(item->data(QSS_FULL_PATH_ROLE).toString());
      delete item;
      }
}

void QSSPage::readSettings(){
  //load stylesheets
  m_ui->qssListWidget->clear();
  findStyleSheets(lthemeengine::userStyleSheetPath());
  findStyleSheets(lthemeengine::sharedStyleSheetPath().join(", "));
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  QStringList styleSheets = settings.value("Interface/stylesheets").toStringList();
  for(int i = 0; i < m_ui->qssListWidget->count(); ++i){
    QListWidgetItem *item = m_ui->qssListWidget->item(i);
    if(styleSheets.contains(item->data(QSS_FULL_PATH_ROLE).toString()))
      item->setCheckState(Qt::Checked);
    else
       item->setCheckState(Qt::Unchecked);
    }
}

void QSSPage::findStyleSheets(const QString &path){
  QDir dir(path);
  dir.setFilter(QDir::Files);
  dir.setNameFilters(QStringList() << "*.qss");
  foreach (QFileInfo info, dir.entryInfoList()){
    QListWidgetItem *item = new QListWidgetItem(info.fileName(),  m_ui->qssListWidget);
    item->setToolTip(info.filePath());
    item->setData(QSS_FULL_PATH_ROLE, info.filePath());
    item->setData(QSS_WRITABLE_ROLE, info.isWritable());
    }
}

void QSSPage::on_renameButton_clicked(){
  QListWidgetItem *item = m_ui->qssListWidget->currentItem();
  if(!item)
    return;
    QString name = QInputDialog::getText(this, tr("Rename Style Sheet"), tr("Style sheet name:"), QLineEdit::Normal, item->text(), 0);
    if(name.isEmpty())
      return;
        if(!m_ui->qssListWidget->findItems(name, Qt::MatchExactly).isEmpty()){
          QMessageBox::warning(this, tr("Error"), tr("The style sheet \"%1\" already exists").arg(name));
          return;
          }
    if(!name.endsWith(".qss", Qt::CaseInsensitive)) name.append(".qss");
      QString newPath = lthemeengine::userStyleSheetPath() + name;
      if(!QFile::rename(item->data(QSS_FULL_PATH_ROLE).toString(), newPath)){
        QMessageBox::warning(this, tr("Error"), tr("Unable to rename file"));
         return;
         }
      item->setText(name);
      item->setData(QSS_FULL_PATH_ROLE, newPath);
      item->setToolTip(newPath);
}

void QSSPage::on_qssListWidget_customContextMenuRequested(const QPoint &pos){
  QListWidgetItem *item = m_ui->qssListWidget->currentItem();
    if(item && item->data(QSS_WRITABLE_ROLE).toBool()){
      m_menu->exec(m_ui->qssListWidget->viewport()->mapToGlobal(pos));
      }
}
