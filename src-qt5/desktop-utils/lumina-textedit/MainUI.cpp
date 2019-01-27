//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include "syntaxSupport.h"

#include <LuminaXDG.h>
#include <LUtils.h>

#include <QFileDialog>
#include <QDir>
#include <QKeySequence>
#include <QTimer>
#include <QMessageBox>
#include <QActionGroup>
#include <QPrinter>
#include <QPrintDialog>
#include <QClipboard>
#include <QInputDialog>
#include "PlainTextEditor.h"

#define DEBUG 0

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  fontbox = new QFontComboBox(this);
    fontbox->setFocusPolicy(Qt::NoFocus);
    fontbox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QWidget *spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  label_readonly = new QAction(tr("Read-Only File"), this);
    label_readonly->setEnabled(false); //not an actual button
    label_readonly->setToolTip("");
  QFont fnt = this->font();
    fnt.setItalic(true);
    fnt.setBold(true);
    label_readonly->setFont(fnt);
  fontSizes = new QSpinBox(this);
    fontSizes->setRange(5, 72);
    fontSizes->setValue(this->font().pointSize());
   fontSizes->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //For some reason, the FontComboBox is always 2 pixels taller than the SpinBox - manually fix that here
  fontbox->setFixedHeight(ui->toolBar->iconSize().height()-2);
  fontSizes->setFixedHeight(ui->toolBar->iconSize().height());

  ui->toolBar->addWidget(spacer);
  ui->toolBar->addAction(label_readonly);
  ui->toolBar->addWidget(spacer2);
  ui->toolBar->addWidget(fontbox);
  ui->toolBar->addWidget(fontSizes);
  //Load the special Drag and Drop QTabWidget
  tabWidget = new DnDTabWidget(this);
  static_cast<QVBoxLayout*>(ui->centralwidget->layout())->insertWidget(0,tabWidget, 1);
  //Setup the action group for the tab location options
  QActionGroup *agrp = new QActionGroup(this);
    agrp->setExclusive(true);
    agrp->addAction(ui->action_tabsTop);
    agrp->addAction(ui->action_tabsBottom);
    agrp->addAction(ui->action_tabsLeft);
    agrp->addAction(ui->action_tabsRight);
  //Load settings
  settings = LUtils::openSettings("lumina-desktop","lumina-textedit", this);
  if(settings->contains("lastfont")){
    QFont oldfont;
    if(oldfont.fromString(settings->value("lastfont").toString() ) ){
      fontbox->setCurrentFont( oldfont );
      fontChanged(oldfont); //load it right now
    }
  }
  Custom_Syntax::SetupDefaultColors(settings); //pre-load any color settings as needed
  colorDLG = new ColorDialog(settings, this);
  this->setWindowTitle(tr("Text Editor"));
  tabWidget->clear();
  //Add keyboard shortcuts
  closeFindS = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(closeFindS, SIGNAL(activated()), this, SLOT(closeFindReplace()) );
  ui->groupReplace->setVisible(false);
  nextTabS = new QShortcut(QKeySequence(QKeySequence::Forward), this);
  prevTabS = new QShortcut(QKeySequence(QKeySequence::Back), this);
  closeTabS = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4), this);
  connect(nextTabS, SIGNAL(activated()), this, SLOT(nextTab()) );
  connect(prevTabS, SIGNAL(activated()), this, SLOT(prevTab()) );
  connect(closeTabS, SIGNAL(activated()), this, SLOT(CloseFile()) );

  //Update the menu of available syntax highlighting modes
  QStringList smodes = Custom_Syntax::availableRules(settings);
  for(int i=0; i<smodes.length(); i++){
    ui->menuSyntax_Highlighting->addAction(smodes[i]);
  }

  bool toolbarVisible = settings->value("showToolbar",true).toBool();
  ui->toolBar->setHidden(!toolbarVisible);
  ui->actionShow_Toolbar->setChecked(toolbarVisible);
  ui->actionLine_Numbers->setChecked( settings->value("showLineNumbers",true).toBool() );
  ui->actionWrap_Lines->setChecked( settings->value("wrapLines",true).toBool() );
  ui->actionShow_Popups->setChecked( settings->value("showPopupWarnings",true).toBool() );
  ui->actionEnable_Spellcheck->setChecked( settings->value("enableSpellcheck",true).toBool() );
  QString tabLoc = settings->value("tabsLocation","top").toString().toLower();
  if(tabLoc=="bottom"){ ui->action_tabsBottom->setChecked(true); tabWidget->setTabPosition(QTabWidget::South);}
  else if(tabLoc=="left"){ ui->action_tabsLeft->setChecked(true); tabWidget->setTabPosition(QTabWidget::West);}
  else if(tabLoc=="right"){ ui->action_tabsRight->setChecked(true); tabWidget->setTabPosition(QTabWidget::East);}
  else{ ui->action_tabsTop->setChecked(true); tabWidget->setTabPosition(QTabWidget::North); }

  //Setup any connections
  connect(agrp, SIGNAL(triggered(QAction*)), this, SLOT(changeTabsLocation(QAction*)) );
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->actionNew_File, SIGNAL(triggered()), this, SLOT(NewFile()) );
  connect(ui->actionOpen_File, SIGNAL(triggered()), this, SLOT(OpenFile()) );
  connect(ui->actionClose_File, SIGNAL(triggered()), this, SLOT(CloseFile()) );
  connect(ui->actionSave_File, SIGNAL(triggered()), this, SLOT(SaveFile()) );
  connect(ui->actionSave_File_As, SIGNAL(triggered()), this, SLOT(SaveFileAs()) );
  connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(Print()) );
  connect(ui->menuSyntax_Highlighting, SIGNAL(triggered(QAction*)), this, SLOT(UpdateHighlighting(QAction*)) );
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()) );
  connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClosed(int)) );
  connect(tabWidget->dndTabBar(), SIGNAL(DetachTab(int)), this, SLOT(tabDetached(int)) );
  connect(tabWidget->dndTabBar(), SIGNAL(DroppedIn(QStringList)), this, SLOT(LoadArguments(QStringList)) );
  connect(tabWidget->dndTabBar(), SIGNAL(DraggedOut(int, Qt::DropAction)), this, SLOT(tabDraggedOut(int, Qt::DropAction)) );
  connect(ui->actionShow_Toolbar, SIGNAL(toggled(bool)), this, SLOT(showToolbar(bool)) );
  connect(ui->actionLine_Numbers, SIGNAL(toggled(bool)), this, SLOT(showLineNumbers(bool)) );
  connect(ui->actionWrap_Lines, SIGNAL(toggled(bool)), this, SLOT(wrapLines(bool)) );
  connect(ui->actionShow_Popups, SIGNAL(toggled(bool)), this, SLOT(showPopupWarnings(bool)) );
  connect(ui->actionEnable_Spellcheck, SIGNAL(toggled(bool)), this, SLOT(enableSpellcheck(bool)) );
  connect(ui->actionCustomize_Colors, SIGNAL(triggered()), this, SLOT(ModifyColors()) );
  connect(ui->actionSet_Language, SIGNAL(triggered()), this, SLOT(SetLanguage()) );
  connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(openFind()) );
  connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(openReplace()) );
  connect(ui->tool_find_next, SIGNAL(clicked()), this, SLOT(findNext()) );
  connect(ui->tool_find_prev, SIGNAL(clicked()), this, SLOT(findPrev()) );
  connect(ui->tool_replace, SIGNAL(clicked()), this, SLOT(replaceOne()) );
  connect(ui->tool_replace_all, SIGNAL(clicked()), this, SLOT(replaceAll()) );
  connect(ui->tool_hideReplaceGroup, SIGNAL(clicked()), this, SLOT(closeFindReplace()) );
  connect(ui->line_find, SIGNAL(returnPressed()), this, SLOT(findNext()) );
  connect(ui->line_replace, SIGNAL(returnPressed()), this, SLOT(replaceOne()) );
  connect(colorDLG, SIGNAL(colorsChanged()), this, SLOT(UpdateHighlighting()) );
  connect(fontbox, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)) );
  connect(fontSizes, SIGNAL(valueChanged(int)), this, SLOT(changeFontSize(int)));

  updateIcons();
  //Now load the initial size of the window
  QSize lastSize = settings->value("lastSize",QSize()).toSize();
  if(lastSize.width() > this->sizeHint().width() && lastSize.height() > this->sizeHint().height() ){
    this->resize(lastSize);
  }

  ui->actionEnable_Spellcheck->setEnabled(DEBUG);
  ui->actionSet_Language->setEnabled(DEBUG);
  ui->actionEnable_Spellcheck->setVisible(DEBUG);
  ui->actionSet_Language->setVisible(DEBUG);
}

MainUI::~MainUI(){

}

void MainUI::LoadArguments(QStringList args){ //CLI arguments
  for(int i=0; i<args.length(); i++){
    OpenFile( LUtils::PathToAbsolute(args[i]) );
    if(ui->groupReplace->isVisible()){ ui->line_find->setFocus(); }
    else{ currentEditor()->setFocus(); }
  }
  if(tabWidget->count()<1){
    NewFile();
  }
}

// =================
//      PUBLIC SLOTS
//=================
void MainUI::updateIcons(){
  this->setWindowIcon( LXDG::findIcon("document-edit") );
  ui->actionClose->setIcon(LXDG::findIcon("application-exit") );
  ui->actionNew_File->setIcon(LXDG::findIcon("document-new") );
  ui->actionOpen_File->setIcon(LXDG::findIcon("document-open") );
  ui->actionClose_File->setIcon(LXDG::findIcon("document-close") );
  ui->actionSave_File->setIcon(LXDG::findIcon("document-save") );
  ui->actionSave_File_As->setIcon(LXDG::findIcon("document-save-as") );
  ui->actionPrint->setIcon(LXDG::findIcon("printer") );
  ui->actionFind->setIcon(LXDG::findIcon("edit-find") );
  ui->actionReplace->setIcon(LXDG::findIcon("edit-find-replace") );
  ui->menuSyntax_Highlighting->setIcon( LXDG::findIcon("format-text-color") );
  ui->actionCustomize_Colors->setIcon( LXDG::findIcon("format-fill-color") );
  ui->menuTabs_Location->setIcon( LXDG::findIcon("tab-detach") );
  //icons for the special find/replace groupbox
  ui->tool_find_next->setIcon(LXDG::findIcon("go-down-search"));
  ui->tool_find_prev->setIcon(LXDG::findIcon("go-up-search"));
  ui->tool_find_casesensitive->setIcon(LXDG::findIcon("format-text-italic"));
  ui->tool_replace->setIcon(LXDG::findIcon("arrow-down"));
  ui->tool_replace_all->setIcon(LXDG::findIcon("arrow-down-double"));
  ui->tool_hideReplaceGroup->setIcon(LXDG::findIcon("dialog-close",""));
  //ui->tool_find_next->setIcon(LXDG::findIcon(""));

  QTimer::singleShot(0,colorDLG, SLOT(updateIcons()) );
}

// =================
//          PRIVATE
//=================
PlainTextEditor* MainUI::currentEditor(){
  if(tabWidget->count()<1){ return 0; }
  return static_cast<PlainTextEditor*>( tabWidget->currentWidget() );
}

QString MainUI::currentFile(){
  PlainTextEditor* cur = currentEditor();
  if(cur!=0){
    return cur->currentFile();
  }
  return "";
}

QString MainUI::currentFileDir(){
  PlainTextEditor* cur = currentEditor();
  QString dir;
  if(cur!=0){
    if(cur->currentFile().startsWith("/")){
      dir = cur->currentFile().section("/",0,-2);
    }
  }
  return dir;
}

QStringList MainUI::unsavedFiles(){
  QStringList unsaved;
  for(int i=0; i<tabWidget->count(); i++){
    PlainTextEditor *tmp = static_cast<PlainTextEditor*>(tabWidget->widget(i));
    if(tmp->hasChange()){
      unsaved << tmp->currentFile();
    }
  }
  return unsaved;
}

// =================
//    PRIVATE SLOTS
//=================
//Main Actions
void MainUI::NewFile(){
  OpenFile(QString::number(tabWidget->count()+1)+"/"+tr("New File"));
}

void MainUI::OpenFile(QString file){
  QStringList files;
  if(file.isEmpty()){
    //Prompt for a file to open
    files = QFileDialog::getOpenFileNames(this, tr("Open File(s)"), currentFile(), tr("Text Files (*)") );
    if(files.isEmpty()){ return; } //cancelled
  }else{
    files << file;
  }
  for(int i=0; i<files.length(); i++){
    PlainTextEditor *edit = 0;
    //Try to see if this file is already opened first
    for(int j=0; j<tabWidget->count(); j++){
      PlainTextEditor *tmp = static_cast<PlainTextEditor*>(tabWidget->widget(j));
      if(tmp->currentFile()==files[i]){ edit = tmp; break; }
    }
    if(edit ==0){
      //New file - need to create a new editor for it
      edit = new PlainTextEditor(settings, this);
      connect(edit, SIGNAL(FileLoaded(QString)), this, SLOT(updateTab(QString)) );
      connect(edit, SIGNAL(UnsavedChanges(QString)), this, SLOT(updateTab(QString)) );
      if(DEBUG)
        connect(edit, SIGNAL(CheckSpelling(int, int)), this, SLOT(checkSpelling(int, int)));
      connect(edit, SIGNAL(statusTipChanged()), this, SLOT(updateStatusTip()) );
      tabWidget->addTab(edit, files[i].section("/",-1));
      edit->showLineNumbers(ui->actionLine_Numbers->isChecked());
      edit->setLineWrapMode( ui->actionWrap_Lines->isChecked() ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
      edit->setFocusPolicy(Qt::ClickFocus); //no "tabbing" into this widget
      QFont font = fontbox->currentFont();
      font.setPointSize( fontSizes->value() );
      edit->document()->setDefaultFont(font);
      /*QStringList applicationDirs = LXDG::systemApplicationDirs();*/
      if(ui->actionEnable_Spellcheck->isChecked()) {
        /*QStringList dirs = QString(getenv("XDG_DATA_DIRS")).split(":");
        foreach(QString dir, dirs) {
          if(QDir(dir).exists("hunspell")) {
            //Default to US English Dictionary
            hunspellPath = dir+"/hunspell/";
            hunspell = new Hunspell(QString(hunspellPath + "en_US.aff").toLocal8Bit(), QString(hunspellPath + "en_US.dic").toLocal8Bit());
            edit->setDictionary(hunspell);
          }
        }*/
      }
    }
    tabWidget->setCurrentWidget(edit);
    edit->LoadFile(files[i]);
    if(DEBUG)
      checkSpelling(-1);
    edit->setFocus();
    QApplication::processEvents(); //to catch the fileLoaded() signal
  }
}

void MainUI::CloseFile(){
  int index = tabWidget->currentIndex();
  if(index>=0){ tabClosed(index); }
}

bool MainUI::SaveFile(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return true; } //nothing to do
  return cur->SaveFile();
}

bool MainUI::SaveFileAs(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return true; } //nothing to do
  return cur->SaveFile(true);
}

bool MainUI::SaveAllFiles(){
  bool ok = true;
  for(int i=0; i<tabWidget->count() && ok; i++){
    PlainTextEditor *tmp = static_cast<PlainTextEditor*>(tabWidget->widget(i));
    if(tmp->hasChange()){
      ok = ok && tmp->SaveFile();
    }
  }
  return ok;
}

void MainUI::Print() {
	QPrinter *print = new QPrinter(QPrinter::HighResolution);
	QPrintDialog dialog(print, NULL);
	dialog.setWindowTitle(tr("Print Content"));
	dialog.setOptions( QAbstractPrintDialog::PrintCurrentPage | QAbstractPrintDialog::PrintToFile );
	if(currentEditor()->textCursor().hasSelection()) {
		dialog.setOptions( QAbstractPrintDialog::PrintCurrentPage | QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintSelection );
	}
	if(dialog.exec() == QDialog::Accepted) {
		currentEditor()->print(print);
	}
}

void MainUI::fontChanged(const QFont&){
  if(currentEditor()==0){ return; }
  //Save this font for later
  QFont font = fontbox->currentFont();
  font.setPointSize( fontSizes->value() );
  settings->setValue("lastfont", font.toString());
   currentEditor()->document()->setDefaultFont(font);
}

void MainUI::changeFontSize(int newFontSize){
  if(currentEditor()==0){ return; }
    QFont currentFont = currentEditor()->document()->defaultFont();
    currentFont.setPointSize(newFontSize);
    currentEditor()->document()->setDefaultFont(currentFont);
    currentEditor()->updateLNW();
}

void MainUI::changeTabsLocation(QAction *act){
  QString set;
  if(act==ui->action_tabsTop){
	set = "top"; tabWidget->setTabPosition(QTabWidget::North);
  }else if(act==ui->action_tabsBottom){
	set = "bottom"; tabWidget->setTabPosition(QTabWidget::South);
  }else if(act==ui->action_tabsLeft){
	set = "left"; tabWidget->setTabPosition(QTabWidget::West);
  }else if(act==ui->action_tabsRight){
	set = "right"; tabWidget->setTabPosition(QTabWidget::East);
  }
  if(!set.isEmpty()){ settings->setValue("tabsLocation",set); }
}

void MainUI::updateStatusTip(){
  QString msg = currentEditor()->statusTip();
  //ui->statusbar->clearMessage();
  ui->statusbar->showMessage(msg);
}

void MainUI::UpdateHighlighting(QAction *act){
  if(act!=0){
    //Single-editor change
    PlainTextEditor *cur = currentEditor();
    if(cur==0){ return; }
    cur->LoadSyntaxRule(act->text());
  }else{
    //Have every editor reload the syntax rules (color changes)
    for(int i=0; i<tabWidget->count(); i++){
      static_cast<PlainTextEditor*>(tabWidget->widget(i))->updateSyntaxColors();
    }
  }
}

void MainUI::showLineNumbers(bool show){
  settings->setValue("showLineNumbers",show);
  for(int i=0; i<tabWidget->count(); i++){
    PlainTextEditor *edit = static_cast<PlainTextEditor*>(tabWidget->widget(i));
    edit->showLineNumbers(show);
  }
}

void MainUI::wrapLines(bool wrap){
  settings->setValue("wrapLines",wrap);
  if(currentEditor() == NULL){ return; }
  currentEditor()->setLineWrapMode( wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
  /*for(int i=0; i<tabWidget->count(); i++){
    PlainTextEditor *edit = static_cast<PlainTextEditor*>(tabWidget->widget(i));
    edit->setLineWrapMode( wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
  }*/
}

void MainUI::ModifyColors(){
  colorDLG->LoadColors();
  colorDLG->showNormal();
}

void MainUI::SetLanguage() {
  /*QDir dir(hunspellPath);
  QStringList files = dir.entryList(QStringList() << "*.dic", QDir::Files);
  QStringList items;
  int defaultDic = 0;
  for(int i = 0; i < files.size(); i++) {
    QString item = files[i].split('.')[0];
    if(item == settings->value("language/").toString()) {
      defaultDic = i;
    }
    items.append(item);
  }
  QString dic = QInputDialog::getItem(this, "Set Language for Document", "Language:", items, defaultDic);
  settings->setValue("language/", dic);
  if(hunspell)
    delete hunspell;

  hunspell = new Hunspell(QString(hunspellPath+dic+".aff").toLocal8Bit(), QString(hunspellPath+dic+".dic").toLocal8Bit());

  checkSpelling(-1);*/
}

void MainUI::showPopupWarnings(bool show){
  settings->setValue("showPopupWarnings",show);
}

void MainUI::enableSpellcheck(bool show){
  /*qDebug() << "Enabling Spellcheck";
  settings->setValue("enableSpellcheck",show);
  if(currentEditor() != NULL and hunspell == NULL) {
    //QStringList applicationDirs = LXDG::systemApplicationDirs();
    hunspell = new Hunspell(QString(hunspellPath + "en_US.aff").toLocal8Bit(), QString(hunspellPath + "en_US.dic").toLocal8Bit());
    qDebug() << "Hunspell Created";
  }*/
}

void MainUI::showToolbar(bool show){
  settings->setValue("showToolbar",show);
  ui->toolBar->setHidden(!show);
}

void MainUI::updateTab(QString file){
  PlainTextEditor *cur = 0;
  int index = -1;
  for(int i=0; i<tabWidget->count(); i++){
    PlainTextEditor *tmp = static_cast<PlainTextEditor*>(tabWidget->widget(i));
    if(tmp->currentFile()==file){
	cur = tmp;
	index = i;
	break;
    }
  }
  if(cur==0){ return; } //should never happen
  bool changes = cur->hasChange();
  //qDebug() << "Update Tab:" << file << cur << changes;
  tabWidget->setTabText(index,(changes ? "*" : "") + file.section("/",-1));
  tabWidget->setTabToolTip(index, file);
  tabWidget->setTabWhatsThis(index, file); //needed for drag/drop functionality
  ui->actionSave_File->setEnabled(changes);
  this->setWindowTitle( (changes ? "*" : "") + file.section("/",-2) );
  label_readonly->setVisible( cur->readOnlyFile() );
}

void MainUI::tabChanged(){
  if(tabWidget->count()<1){ return; }
  //update the buttons/menus based on the current widget
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; } //should never happen though
  bool changes = cur->hasChange();
  ui->actionSave_File->setEnabled(changes);
  //this->setWindowTitle( tabWidget->tabText( tabWidget->currentIndex() ) );
  this->setWindowTitle( (changes ? "*" : "") + tabWidget->tabToolTip( tabWidget->currentIndex() ).section("/",-2) );
  if(!ui->line_find->hasFocus() && !ui->line_replace->hasFocus()){ tabWidget->currentWidget()->setFocus(); }
  QFont font = cur->document()->defaultFont();
  //Update the font/size widgets to reflect what is set on this tab
  fontbox->setCurrentFont(font);
  fontSizes->setValue( font.pointSize() );
  ui->actionWrap_Lines->setChecked( cur->lineWrapMode()==QPlainTextEdit::WidgetWidth );
  label_readonly->setVisible( cur->readOnlyFile() );
}

void MainUI::tabClosed(int tab){
  PlainTextEditor *edit = static_cast<PlainTextEditor*>(tabWidget->widget(tab));
  if(edit==0){ return; } //should never happen
  if(edit->hasChange() && ui->actionShow_Popups->isChecked() ){
    //Verify if the user wants to lose any unsaved changes
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Lose Unsaved Changes?"), QString(tr("This file has unsaved changes.\nDo you want to close it anyway?\n\n%1")).arg(edit->currentFile()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){ return; }
  }
  tabWidget->removeTab(tab);
  edit->deleteLater();
}

void MainUI::tabDetached(int tab){
  PlainTextEditor *edit = static_cast<PlainTextEditor*>(tabWidget->widget(tab));
  if(edit==0){ return; } //should never happen
  if(edit->hasChange() && ui->actionShow_Popups->isChecked() ){
    //Verify if the user wants to lose any unsaved changes
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Lose Unsaved Changes?"), QString(tr("This file has unsaved changes.\nDo you want to close it anyway?\n\n%1")).arg(edit->currentFile()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){ return; }
  }
  //Launch this file with a new LTE process
  QProcess::startDetached("lumina-textedit \""+edit->currentFile()+"\"");
  tabWidget->removeTab(tab);
  edit->deleteLater();
}

void MainUI::tabDraggedOut(int tab, Qt::DropAction act){
  qDebug() << "Tab Dragged Out:" << tab << act;
  if(act == Qt::MoveAction){
    tabClosed(tab);
    if(tabWidget->count()==0){ this->close(); } //merging two windows together?
  }
}

void MainUI::nextTab(){
  //qDebug() << "Next Tab";
  if(tabWidget->count()<1){ return; } //no tabs
  int cur = tabWidget->currentIndex();
  cur++;
  if(cur>=tabWidget->count()){ cur = 0; }
  tabWidget->setCurrentIndex(cur);
}

void MainUI::prevTab(){
  //qDebug() << "Previous Tab";
  if(tabWidget->count()<1){ return; } //no tabs
  int cur = tabWidget->currentIndex();
  cur--;
  if(cur<0){ cur = tabWidget->count()-1; }
  tabWidget->setCurrentIndex(cur);
}

//Find/Replace functions
void MainUI::closeFindReplace(){
  ui->groupReplace->setVisible(false);
  PlainTextEditor *cur = currentEditor();
  if(cur!=0){ cur->setFocus(); }
}

void MainUI::openFind(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  ui->groupReplace->setVisible(true);
  ui->line_find->setText( cur->textCursor().selectedText() );
  ui->line_replace->setText("");
  ui->line_find->setFocus();
}

void MainUI::openReplace(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  ui->groupReplace->setVisible(true);
  ui->line_find->setText( cur->textCursor().selectedText() );
  ui->line_replace->setText("");
  ui->line_replace->setFocus();
}

void MainUI::findNext(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  bool found = cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags() );
  if(!found){
    //Try starting back at the top of the file
    cur->moveCursor(QTextCursor::Start);
    cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags() );
  }
}

void MainUI::findPrev(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  bool found = cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively | QTextDocument::FindBackward : QTextDocument::FindBackward );
  if(!found){
    //Try starting back at the bottom of the file
    cur->moveCursor(QTextCursor::End);
    cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively | QTextDocument::FindBackward : QTextDocument::FindBackward );
  }
}

void MainUI::replaceOne(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  //See if the current selection matches the find field first
  if(cur->textCursor().selectedText()==ui->line_find->text()){
    cur->insertPlainText(ui->line_replace->text());
  }
  cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags() );
}

void MainUI::replaceAll(){
PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  //See if the current selection matches the find field first
  bool done = false;
  if(cur->textCursor().selectedText()==ui->line_find->text()){
    cur->insertPlainText(ui->line_replace->text());
    done = true;
  }
  while( cur->find( ui->line_find->text(), ui->tool_find_casesensitive->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags() ) ){
    //Find/replace every occurance of the string
    cur->insertPlainText(ui->line_replace->text());
    done = true;
  }
  if(done){
    //Re-highlight the newly-inserted text
    cur->find( ui->line_replace->text(), QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
  }
}

void MainUI::checkWord(QTextBlock block) {
  /*PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  if(block.text().simplified().isEmpty()){ return; }

  foreach(Word *word, wordList) {
    if(word->blockNum == block.blockNumber()){
      qDebug() << "Remove Word";
      wordList.removeOne(word);
    }
  }

  QStringList words = block.text().split(QRegExp("\\W+"));
  qDebug() << "Got Words:" << words;
  QTextCursor cursor(block);

  foreach(QString word, words) {
    qDebug() << "Check Word:" << word;
    if(!hunspell->spell(word.toStdString())) {
      qDebug() << "Not a word";
      QStringList suggestions;
      foreach(std::string newWord, hunspell->suggest(word.toStdString())){
        suggestions.append(QString::fromStdString(newWord));
      }
      qDebug() << "Got Suggestions:" << suggestions;
      QTextEdit::ExtraSelection sel;
      sel.format.setBackground(QColor("Red"));
      sel.cursor = cur->document()->find(word, cursor.position());
      Word *wordC = new Word(word, suggestions, sel, block.blockNumber(), cursor.positionInBlock());
      wordList.append(wordC);
    }
    cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
  }*/
}

void MainUI::checkSpelling(int bpos, int epos) {
  qDebug() << "Checking spelling on" << bpos << epos;
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  static int numBlocks = cur->blockCount();
  //qDebug() << " - numblocks:" << numBlocks;
  if(bpos == -1 || numBlocks != cur->blockCount()) { //When opening a file or loading a new dictionary
    for(QTextBlock block = cur->document()->begin(); block != cur->document()->end(); block = block.next()){
      //qDebug() << " - Check Block:" << block.text();
      checkWord(block);
    }
    numBlocks = cur->blockCount();
  }else if(epos == -1){ //Normal checking of one block from typing
    QTextBlock block = cur->document()->findBlock(bpos);
    //qDebug() << " - Check Block:" << block.text();
    checkWord(block);
  }else { //Check blocks after copy/paste
    for(QTextBlock block = cur->document()->findBlock(0); block != cur->document()->findBlock(epos); block = block.next()) {
      checkWord(block);
    }
  }
  //qDebug() << " - set Word List:" << wordList;

  cur->setWordList(wordList);
}

//=============
//   PROTECTED
//=============
void MainUI::closeEvent(QCloseEvent *ev){
  //See if any of the open editors have unsaved changes first
  QStringList unsaved = unsavedFiles();
  if(unsaved.isEmpty() || !ui->actionShow_Popups->isChecked()){
    QMainWindow::closeEvent(ev);
    return;
  }

  //Otherwise, ask the user what to do.
  QMessageBox::StandardButton but = QMessageBox::question(
          this,
          tr("Save Changes before closing?"),
          QString(tr("There are unsaved changes.\nDo you want save them before you close the editor?\n\n%1")).arg(unsaved.join("\n")),
          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
          QMessageBox::No);

  if(but == QMessageBox::Cancel){
    ev->ignore();
    return;
  }
  else if(but == QMessageBox::Yes){
    if( !SaveAllFiles() ){
      //cancelled by user
      ev->ignore();
      return;
    }

  }
  QMainWindow::closeEvent(ev);
}
