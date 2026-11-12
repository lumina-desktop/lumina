#include <QSettings>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QMenu>
#include <QDebug>
#include <QTimer>

#include "lthemeengine.h"
#include "qsseditordialog.h"
#include "qsspage.h"
#include "ui_qsspage.h"

#define QSS_FULL_PATH_ROLE (Qt::ItemDataRole(Qt::UserRole))
#define QSS_WRITABLE_ROLE (Qt::ItemDataRole(Qt::UserRole + 1))

QSSPage::QSSPage(QWidget *parent, bool desktop) : TabPage(parent), m_ui(new Ui::QSSPage){
  m_ui->setupUi(this);
  desktop_qss = desktop;
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
  m_ui->tool_enable->setEnabled(false);
  m_ui->tool_disable->setEnabled(false);
  m_ui->copyButton->setEnabled(false);
}

QSSPage::~QSSPage(){
  delete m_ui;
}

void QSSPage::writeSettings(){
  QStringList styleSheets;
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  for(int i = m_ui->qssListWidget->count()-1; i>=0; i--){
    QListWidgetItem *item = m_ui->qssListWidget->item(i);
    styleSheets << item->data(QSS_FULL_PATH_ROLE).toString();
  }
  if(desktop_qss){ settings.setValue("Interface/desktop_stylesheets", styleSheets); }
  else{ settings.setValue("Interface/stylesheets", styleSheets); }
}

void QSSPage::on_qssListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *){
  if(current!=0){
    m_ui->list_disabled->clearSelection(); //clear any current selection on the other widget
    m_ui->list_disabled->setCurrentRow(-1);
    m_ui->tool_enable->setEnabled(false);
  }
  //qDebug() << "Got Current Item Changed";
  m_ui->tool_disable->setEnabled(current!=0);
  m_ui->copyButton->setEnabled(current!=0);
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

void QSSPage::on_list_disabled_currentItemChanged(QListWidgetItem *current, QListWidgetItem *){
  if(current!=0){
    m_ui->qssListWidget->clearSelection(); //clear any current selection on the other widget
    m_ui->qssListWidget->setCurrentRow(-1);
    m_ui->tool_disable->setEnabled(false);
  }
  //qDebug() << "Got Current Item Changed";
  m_ui->tool_enable->setEnabled(current!=0);
  m_ui->copyButton->setEnabled(current!=0);
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
  if(name.isEmpty()){ return; }
  if(!name.endsWith(".qss", Qt::CaseInsensitive)){ name.append(".qss"); }
  QString filePath;
    if(desktop_qss){ filePath = lthemeengine::userDesktopStyleSheetPath() + name; }
    else{ filePath = lthemeengine::userStyleSheetPath() + name; }
  if(QFile::exists(filePath)){
    QMessageBox::warning(this, tr("Error"), tr("The file \"%1\" already exists").arg(filePath));
    return;
    }
  // Make sure the directory exists
  QString dir = filePath.section("/",0,-2);
  if(!QFile::exists(dir)){
    QDir D(dir);
    D.mkpath(dir);
  }
  //creating empty file
  QFile file(filePath);
  file.open(QIODevice::WriteOnly);
  file.close();
  //creating item
  QFileInfo info(filePath);
  QListWidgetItem *item = new QListWidgetItem(info.fileName(),  m_ui->list_disabled);
  item->setToolTip(info.filePath());
  item->setData(QSS_FULL_PATH_ROLE, info.filePath());
  item->setData(QSS_WRITABLE_ROLE, info.isWritable());
  m_ui->list_disabled->setCurrentRow(m_ui->list_disabled->count()-1);
  QTimer::singleShot(10, this, SLOT(on_editButton_clicked()) );
}

void QSSPage::on_editButton_clicked(){
  QListWidgetItem *item = currentSelection();
  if(item){
    QSSEditorDialog dialog(item->data(QSS_FULL_PATH_ROLE).toString(), this);
    dialog.exec();
    }
}

void QSSPage::on_copyButton_clicked(){
 QListWidgetItem *sel = currentSelection();
  if(sel==0){ return; }
 QString name = QInputDialog::getText(this, tr("Enter Style Sheet Name"), tr("File name:"), QLineEdit::Normal, sel->text().section(".qss",0,0)+"_copy");
  if(name.isEmpty()){ return; }
  if(!name.endsWith(".qss", Qt::CaseInsensitive)){ name.append(".qss"); }
  QString filePath;
    if(desktop_qss){ filePath = lthemeengine::userDesktopStyleSheetPath() + name; }
    else{ filePath = lthemeengine::userStyleSheetPath() + name; }
  if(QFile::exists(filePath)){
    QMessageBox::warning(this, tr("Error"), tr("The file \"%1\" already exists").arg(filePath));
    return;
    }
  // Make sure the directory exists
  QString dir = filePath.section("/",0,-2);
  if(!QFile::exists(dir)){
    QDir D(dir);
    D.mkpath(dir);
  }
  //Copy the file over
  QFile::copy(sel->data(QSS_FULL_PATH_ROLE).toString(), filePath);
  //creating item
  QFileInfo info(filePath);
  QListWidgetItem *item = new QListWidgetItem(info.fileName(),  m_ui->list_disabled);
  item->setToolTip(info.filePath());
  item->setData(QSS_FULL_PATH_ROLE, info.filePath());
  item->setData(QSS_WRITABLE_ROLE, info.isWritable());
  m_ui->list_disabled->setCurrentRow(m_ui->list_disabled->count()-1);
}

void QSSPage::on_removeButton_clicked(){
  QListWidgetItem *item = currentSelection();
  if(!item){ return; }
  int button = QMessageBox::question(this, tr("Confirm Remove"),tr("Are you sure you want to remove style sheet \"%1\"?").arg(item->text()), QMessageBox::Yes | QMessageBox::No);
  if(button == QMessageBox::Yes){ QFile::remove(item->data(QSS_FULL_PATH_ROLE).toString()); }
  delete item;
}

void QSSPage::on_tool_enable_clicked(){
  QList<QListWidgetItem*> sel = m_ui->list_disabled->selectedItems();
  //qDebug() << "Got Selection:" << sel.count();
  for(int i=0; i<sel.length(); i++){
    m_ui->qssListWidget->addItem(sel[i]->clone());
    delete sel[i];
    //QCoreApplication::processEvents();
    m_ui->qssListWidget->setCurrentRow(m_ui->qssListWidget->count()-1);
  }

}

void QSSPage::on_tool_disable_clicked(){
  QList<QListWidgetItem*> sel = m_ui->qssListWidget->selectedItems();
  //qDebug() << "Got Selection:" << sel.count();
  for(int i=0; i<sel.length(); i++){
    m_ui->list_disabled->addItem(sel[i]->clone());
    delete sel[i];
    //QCoreApplication::processEvents();
    m_ui->list_disabled->setCurrentRow(m_ui->list_disabled->count()-1);
  }
}

void QSSPage::on_tool_priority_up_clicked(){
  QList<QListWidgetItem*> sel = m_ui->qssListWidget->selectedItems();
  for(int i=0; i<sel.length(); i++){
    int index = m_ui->qssListWidget->row(sel[i]);
    //qDebug() << "Move Item Up:" << index;
    if(index>0){
      m_ui->qssListWidget->insertItem(index-1, m_ui->qssListWidget->takeItem(index));
      m_ui->qssListWidget->setCurrentRow(index-1);
    }
  }
}

void QSSPage::on_tool_priority_down_clicked(){
  QList<QListWidgetItem*> sel = m_ui->qssListWidget->selectedItems();
  for(int i=0; i<sel.length(); i++){
    int index = m_ui->qssListWidget->row(sel[i]);
    //qDebug() << "Move Item Down:" << index;
    if(index<(m_ui->qssListWidget->count()-1) ){
      m_ui->qssListWidget->insertItem(index+1, m_ui->qssListWidget->takeItem(index));
      m_ui->qssListWidget->setCurrentRow(index+1);
    }
  }
}

void QSSPage::readSettings(){
  //load stylesheets
  m_ui->qssListWidget->clear();
  m_ui->list_disabled->clear();
  //Read the currently-enabled settings
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  QStringList styleSheets;
  if(desktop_qss){ styleSheets = settings.value("Interface/desktop_stylesheets").toStringList(); }
  else{ styleSheets = settings.value("Interface/stylesheets").toStringList(); }
  for(int i=0; i<styleSheets.length(); i++){
    if(styleSheets[i].contains("..") || styleSheets[i].contains("//") ){
      //Get the absolute path for matching later
      styleSheets[i] = QFileInfo(styleSheets[i]).absoluteFilePath();
    }
  }
  //Now load the items into list widgets
  //qDebug() << "Found Stylesheets" << styleSheets;
  if(desktop_qss){ findStyleSheets(QStringList() << lthemeengine::userDesktopStyleSheetPath() << lthemeengine::sharedDesktopStyleSheetPath(), styleSheets); }
  else{findStyleSheets(QStringList() << lthemeengine::userStyleSheetPath() << lthemeengine::sharedStyleSheetPath(), styleSheets); }

}

void QSSPage::findStyleSheets(QStringList paths, QStringList enabled){
  paths.removeDuplicates();
  std::reverse(enabled.begin(), enabled.end()); // reverse for proper order
  QMap<int, QString> sortedStylesSheets;
  for(int i=0; i<paths.length(); i++){
    if(!QFile::exists(paths[i])){ continue; }
    QDir dir(paths[i]);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << "*.qss");
    foreach (QFileInfo info, dir.entryInfoList()){
      if(enabled.contains(info.filePath())){ sortedStylesSheets[enabled.indexOf(info.filePath())] = info.filePath(); }
      else{
        QListWidgetItem *item = new QListWidgetItem(info.fileName());
        item->setToolTip(info.filePath());
        item->setData(QSS_FULL_PATH_ROLE, info.filePath());
        item->setData(QSS_WRITABLE_ROLE, info.isWritable());
        m_ui->list_disabled->addItem(item);
      }
    }
  }
  QMapIterator<int, QString> i(sortedStylesSheets);
  while (i.hasNext()) {
    i.next();
    QFileInfo info(i.value());
    if (info.isFile()) {
      QListWidgetItem *item = new QListWidgetItem(info.fileName());
      item->setToolTip(info.filePath());
      item->setData(QSS_FULL_PATH_ROLE, info.filePath());
      item->setData(QSS_WRITABLE_ROLE, info.isWritable());
      m_ui->qssListWidget->addItem(item);
    }
  }
  m_ui->list_disabled->sortItems(Qt::AscendingOrder);

}

void QSSPage::on_renameButton_clicked(){
  QListWidgetItem *item = currentSelection();
  if(!item){ return; }
  QString name = QInputDialog::getText(this, tr("Rename Style Sheet"), tr("Style sheet name:"), QLineEdit::Normal, item->text(), 0);
  if(name.isEmpty()){ return; }
  if(!m_ui->qssListWidget->findItems(name, Qt::MatchExactly).isEmpty() || !m_ui->list_disabled->findItems(name, Qt::MatchExactly).isEmpty()){
    QMessageBox::warning(this, tr("Error"), tr("The style sheet \"%1\" already exists").arg(name));
    return;
    }
  if(!name.endsWith(".qss", Qt::CaseInsensitive)){ name.append(".qss"); }
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
  if(item && item->data(QSS_WRITABLE_ROLE).toBool()){ m_menu->exec(m_ui->qssListWidget->viewport()->mapToGlobal(pos)); }
}

QListWidgetItem* QSSPage::currentSelection(){
  QListWidgetItem *item = m_ui->qssListWidget->currentItem();
  if(item==0){ item = m_ui->list_disabled->currentItem(); }
  return item;
}
