//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PlainTextEditor.h"

#include <QColor>
#include <QPainter>
#include <QTextBlock>
#include <QFileDialog>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QClipboard>
#include <QTimer>

#include <LUtils.h>

//==============
//       PUBLIC
//==============
PlainTextEditor::PlainTextEditor(QSettings *set, QWidget *parent) : QPlainTextEdit(parent){
  settings = set;
  LNW = new LNWidget(this);
  showLNW = true;
  watcher = new QFileSystemWatcher(this);
  hasChanges = readonly = false;
  lastSaveContents.clear();
  matchleft = matchright = -1;
  this->setTabStopWidth( 8 * this->fontMetrics().width(" ") ); //8 character spaces per tab (UNIX standard)
  //this->setObjectName("PlainTextEditor");
  //this->setStyleSheet("QPlainTextEdit#PlainTextEditor{ }");
  SYNTAX = new Custom_Syntax(settings, this->document());
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(LNW_updateWidth()) );
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(LNW_highlightLine()) );
  connect(this, SIGNAL(updateRequest(const QRect&, int)), this, SLOT(LNW_update(const QRect&, int)) );
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(checkMatchChar()) );
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(cursorMoved()) );
  connect(this, SIGNAL(textChanged()), this, SLOT(textChanged()) );
  connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged()) );
  LNW_updateWidth();
  LNW_highlightLine();
}

PlainTextEditor::~PlainTextEditor(){

}

void PlainTextEditor::showLineNumbers(bool show){
  showLNW = show;
  LNW->setVisible(show);
  LNW_updateWidth();
}

void PlainTextEditor::LoadSyntaxRule(QString type){
  QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){
    if(files[i].name() == type){
      files[i].SetupDocument(this);
      SYNTAX->loadRules(files[i]);
      break;
    }else if(i==files.length()-1){
      SyntaxFile dummy;
      SYNTAX->loadRules(dummy);
    }
  }
  SYNTAX->rehighlight();
}

void PlainTextEditor::updateSyntaxColors(){
  SYNTAX->reloadRules();
  SYNTAX->rehighlight();
}

//File loading/setting options
void PlainTextEditor::LoadFile(QString filepath){
  if( !watcher->files().isEmpty() ){  watcher->removePaths(watcher->files()); }
  bool diffFile = (filepath != this->whatsThis());
  this->setWhatsThis(filepath);
  this->clear();
  /*QList<SyntaxFile> files = SyntaxFile::availableFiles(settings);
  for(int i=0; i<files.length(); i++){
    if(files[i].supportsFile(filepath) ){
      files[i].SetupDocument(this);
      SYNTAX->loadRules(files[i]);
      break;
    }
  }*/
  //SYNTAX->loadRules( Custom_Syntax::ruleForFile(filepath.section("/",-1), settings) );
  lastSaveContents = LUtils::readFile(filepath).join("\n");
  if(diffFile){
    SYNTAX->loadRules( Custom_Syntax::ruleForFile(this->whatsThis().section("/",-1), settings) );
    if(SYNTAX->loadedRules().isEmpty()){
      SYNTAX->loadRules( Custom_Syntax::ruleForFirstLine( lastSaveContents.section("\n",0,0,QString::SectionSkipEmpty) , settings) );
    }
    SYNTAX->setupDocument(this);
    this->setPlainText( lastSaveContents );
  }else{
    //Try to keep the mouse cursor/scroll in the same position
    int curpos = this->textCursor().position();;
    this->setPlainText( lastSaveContents );
    QApplication::processEvents();
    QTextCursor cur = this->textCursor();
      cur.setPosition(curpos);
    this->setTextCursor( cur );
    this->centerCursor(); //scroll until cursor is centered (if possible)
  }
  hasChanges = false;
  readonly = false;
  if(QFile::exists(filepath)){
    readonly =  !QFileInfo(filepath).isWritable();
    watcher->addPath(filepath);
  }else if(filepath.startsWith("/")){
    //See if the containing directory is writable instead
    readonly =  !QFileInfo(filepath.section("/",0,-2)).isWritable();
  }
  emit FileLoaded(this->whatsThis());
}

bool PlainTextEditor::SaveFile(bool newname){
  //NOTE: This returns true for proper behaviour, and false for a user-cancelled process
  //qDebug() << "Save File:" << this->whatsThis();
  //Quick check for a non-editable file
  if(!newname && this->whatsThis().startsWith("/")){
    if(!QFileInfo(this->whatsThis()).isWritable()){ newname = true; } //cannot save the current file name/location
  }
  if( !this->whatsThis().startsWith("/") || newname ){
    //prompt for a filename/path
    QString file = QFileDialog::getSaveFileName(this, tr("Save File"), this->whatsThis(), tr("Text File (*)"));
    if(file.isEmpty()){ return false; } //cancelled
    this->setWhatsThis(file);
    SYNTAX->loadRules( Custom_Syntax::ruleForFile(this->whatsThis().section("/",-1), settings) );
    if(SYNTAX->loadedRules().isEmpty()){
      SYNTAX->loadRules( Custom_Syntax::ruleForFirstLine( this->toPlainText().section("\n",0,0,QString::SectionSkipEmpty) , settings) );
    }
    SYNTAX->setupDocument(this);
    SYNTAX->rehighlight();
  }
  if( !watcher->files().isEmpty() ){ watcher->removePaths(watcher->files()); }
  bool ok = LUtils::writeFile(this->whatsThis(), this->toPlainText().split("\n"), true);
  hasChanges = !ok;
  if(ok){ lastSaveContents = this->toPlainText(); emit FileLoaded(this->whatsThis()); }
  watcher->addPath(currentFile());
  readonly = !QFileInfo(this->whatsThis()).isWritable(); //update this flag
  return true;
  //qDebug() << " - Success:" << ok << hasChanges;
}

QString PlainTextEditor::currentFile(){
  return this->whatsThis();
}

bool PlainTextEditor::hasChange(){
  return hasChanges;
}

bool PlainTextEditor::readOnlyFile(){
  //qDebug() << "Read Only File:" << readonly << this->whatsThis();
  return readonly;
}

//Functions for managing the line number widget
int PlainTextEditor::LNWWidth(){
  //Get the number of chars we need for line numbers
  int lines = this->blockCount();
  if(lines<1){ lines = 1; }
  int chars = 1;
  //qDebug() << "point 1" << this->document()->defaultFont();
  while(lines>=10){ chars++; lines/=10; }
  QFontMetrics metrics(this->document()->defaultFont());
  return (metrics.width("9")*chars); //make sure to add a tiny bit of padding
}

void PlainTextEditor::paintLNW(QPaintEvent *ev){
  //qDebug() << "Paint LNW Event:" << ev->rect() << LNW->geometry();
  //if(ev->rect().height() < (QFontMetrics(this->document()->defaultFont()).height() *1.5) ){ return; }
  //qDebug() << " -- paint line numbers";
  QPainter P(LNW);
  //First set the background color
  P.fillRect(ev->rect(), QColor("lightgrey"));
  //Now determine which line numbers to show (based on the current viewport)
  QTextBlock block = this->firstVisibleBlock();
  int bTop = blockBoundingGeometry(block).translated(contentOffset()).top();
  int bBottom;
//  QFont font = P.font();
//  font.setPointSize(this->document()->defaultFont().pointSize());
  P.setFont(this->document()->defaultFont());
  //Now loop over the blocks (lines) and write in the numbers
  QFontMetrics metrics(this->document()->defaultFont());
  //qDebug() << "point 2" << this->document()->defaultFont();
  P.setPen(Qt::black); //setup the font color
  while(block.isValid() && bTop<=ev->rect().bottom()){ //ensure block below top of viewport
    bBottom = bTop+blockBoundingRect(block).height();
    if(block.isVisible() && bBottom >= ev->rect().top()){ //ensure block above bottom of viewport
      P.drawText(0,bTop, LNW->width(), metrics.height(), Qt::AlignRight, QString::number(block.blockNumber()+1) );
      //qDebug() << "bTop" << bTop;
      //qDebug() << "LNW->width()" << LNW->width();
      //qDebug() << "metrics.height()" << metrics.height();
    }
    //Go to the next block
    block = block.next();
    bTop = bBottom;
  }
}

//==============
//       PRIVATE
//==============
void PlainTextEditor::clearMatchData(){
  if(matchleft>=0 || matchright>=0){
    QList<QTextEdit::ExtraSelection> sel = this->extraSelections();
    for(int i=0; i<sel.length(); i++){
      if(sel[i].cursor.selectedText().length()==1){ sel.takeAt(i); i--; }
    }
    this->setExtraSelections(sel);
    matchleft = -1;
    matchright = -1;
  }
}

void PlainTextEditor::highlightMatch(QChar ch, bool forward, int fromPos, QChar startch){
  if(forward){ matchleft = fromPos;  }
  else{ matchright = fromPos; }

  int nested = 1; //always start within the first nest (the primary nest)
  int tmpFromPos = fromPos;
  //if(!forward){ tmpFromPos++; } //need to include the initial location
  QString doc = this->toPlainText();
  while( nested>0 && tmpFromPos<doc.length() && ( (tmpFromPos>=fromPos && forward) || ( tmpFromPos<=fromPos && !forward ) ) ){
    if(forward){
      QTextCursor cur = this->document()->find(ch, tmpFromPos);
      if(!cur.isNull()){
	nested += doc.mid(tmpFromPos+1, cur.position()-tmpFromPos).count(startch) -1;
	if(nested==0){ matchright = cur.position(); }
	else{ tmpFromPos = cur.position(); }
      }else{ break; }
    }else{
      QTextCursor cur = this->document()->find(ch, tmpFromPos, QTextDocument::FindBackward);
      if(!cur.isNull()){
        QString mid = doc.mid(cur.position()-1, tmpFromPos-cur.position()+1);
        //qDebug() << "Found backwards match:" << nested << startch << ch << mid;
        //qDebug() << doc.mid(cur.position(),1) << doc.mid(tmpFromPos,1);
	nested += (mid.count(startch) - mid.count(ch));
	if(nested==0){ matchleft = cur.position(); }
	else{ tmpFromPos = cur.position()-1; }
      }else{ break; }
    }
  }

  //Now highlight the two characters
  QList<QTextEdit::ExtraSelection> sels = this->extraSelections();
  if(matchleft>=0){
    QTextEdit::ExtraSelection sel;
    if(matchright>=0){ sel.format.setBackground( QColor(settings->value("colors/bracket-found").toString()) ); }
    else{ sel.format.setBackground( QColor(settings->value("colors/bracket-missing").toString()) ); }
    QTextCursor cur = this->textCursor();
      cur.setPosition(matchleft);
      if(forward){ cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor); }
      else{ cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor); }
    sel.cursor = cur;
    sels << sel;
  }
  if(matchright>=0){
    QTextEdit::ExtraSelection sel;
    if(matchleft>=0){ sel.format.setBackground( QColor(settings->value("colors/bracket-found").toString()) ); }
    else{ sel.format.setBackground( QColor(settings->value("colors/bracket-missing").toString()) ); }
    QTextCursor cur = this->textCursor();
      cur.setPosition(matchright);
      if(!forward){ cur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor); }
      else{ cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor); }
    sel.cursor = cur;
    sels << sel;
  }
  this->setExtraSelections(sels);
}

//===================
//       PRIVATE SLOTS
//===================
//Functions for managing the line number widget
void PlainTextEditor::LNW_updateWidth(){
  if(showLNW){
    this->setViewportMargins( LNWWidth(), 0, 0, 0); //the LNW is contained within the left margin
  }else{
    this->setViewportMargins( 0, 0, 0, 0); //the LNW is contained within the left margin
  }
}

void PlainTextEditor::LNW_highlightLine(){
  QList<QTextEdit::ExtraSelection> sels;
  foreach(Word *word, wordList) { sels.append(word->sel); };
  if(this->isReadOnly()){ return; }
  QColor highC = QColor(0,0,0,50); //just darken the line a bit
  QTextEdit::ExtraSelection sel;
  sel.format.setBackground(highC);
  sel.format.setProperty(QTextFormat::FullWidthSelection, true);
  sel.cursor = this->textCursor();
  sel.cursor.clearSelection(); //just in case it already has one
  setExtraSelections( sels << sel);
}

void PlainTextEditor::LNW_update(const QRect &rect, int dy){
  if(dy!=0){ LNW->scroll(0,dy); } //make sure to scroll the line widget the same amount as the editor
  else{
    //Some other reason we need to repaint the widget
    LNW->update(0,rect.y(), LNW->width(), rect.height()); //also repaint the LNW in the same area
  }
  if(rect.contains(this->viewport()->rect())){
    //Something in the currently-viewed area needs updating - make sure the LNW width is still correct
    LNW_updateWidth();
  }
}

//Function for running the matching routine
void PlainTextEditor::checkMatchChar(){
  clearMatchData();
  int pos = this->textCursor().position();
  QChar ch = this->document()->characterAt(pos);
  bool tryback = true;
  while(tryback){
    tryback = false;
    if(ch==QChar('(')){ highlightMatch(QChar(')'),true, pos, QChar('(') ); }
    else if(ch==QChar(')')){ highlightMatch(QChar('('),false, pos, QChar(')') ); }
    else if(ch==QChar('{')){ highlightMatch(QChar('}'),true, pos, QChar('{') ); }
    else if(ch==QChar('}')){ highlightMatch(QChar('{'),false, pos, QChar('}') ); }
    else if(ch==QChar('[')){ highlightMatch(QChar(']'),true, pos, QChar('[') ); }
    else if(ch==QChar(']')){ highlightMatch(QChar('['),false, pos, QChar(']') ); }
    else if(pos==this->textCursor().position()){
      //Try this one more time - using the previous character instead of the current character
      tryback = true;
      pos--;
      ch = this->document()->characterAt(pos);
    }
  } //end check for next/previous char
}

//Functions for notifying the parent widget of changes
void PlainTextEditor::textChanged(){
  //qDebug() << " - Got Text Changed signal";
  bool changed = (lastSaveContents != this->toPlainText());
  emit CheckSpelling(this->textCursor().position(), -1);
  if(changed == hasChanges){ return; } //no change
  hasChanges = changed; //save for reading later
  if(hasChanges){  emit UnsavedChanges( this->whatsThis() ); }
  else{ emit FileLoaded(this->whatsThis()); }
}

void PlainTextEditor::cursorMoved(){
  //Update the status tip for the editor to show the row/column number for the cursor
  QTextCursor cur = this->textCursor();
  QString stat = tr("Row Number: %1, Column Number: %2");
  this->setStatusTip(stat.arg(QString::number(cur.blockNumber()+1) , QString::number(cur.columnNumber()) ) );
  emit statusTipChanged();
}

//Function for prompting the user if the file changed externally
void PlainTextEditor::fileChanged(){
  qDebug() << "File Changed:" << currentFile();
  bool update = !hasChanges; //Go ahead and reload the file automatically - no custom changes in the editor
  QString text = tr("The following file has been changed by some other utility. Do you want to re-load it?");
  text.append("\n");
  text.append( tr("(Note: You will lose all currently-unsaved changes)") );
  text.append("\n\n%1");

  if(!update){
    update = (QMessageBox::Yes == QMessageBox::question(this, tr("File Modified"),text.arg(currentFile()) , QMessageBox::Yes | QMessageBox::No, QMessageBox::No) );
  }
  //Now update the text in the editor as needed
  if(update){
    LoadFile( currentFile() );
  }
}

//==================
//       PROTECTED
//==================
void PlainTextEditor::resizeEvent(QResizeEvent *ev){
  QPlainTextEdit::resizeEvent(ev); //do the normal resize processing
  //Now re-adjust the placement of the LNW (within the left margin area)
  QRect cGeom = this->contentsRect();
  LNW->setGeometry( QRect(cGeom.left(), cGeom.top(), LNWWidth(), cGeom.height()) );
}

void PlainTextEditor::updateLNW(){
    LNW_updateWidth();
}

Word *PlainTextEditor::wordAtPosition(int blockNum, int pos) {
  foreach(Word *word, wordList) {
    //qDebug() << word->word << "WordBlock:" << word->blockNum << "Block:" << blockNum << "WordPos:" << word->position << "Pos:" << pos;
    if(word->blockNum == blockNum and pos >= word->position and pos <= word->position+word->word.length())
      return word;
  }
  return NULL;
}

void PlainTextEditor::contextMenuEvent(QContextMenuEvent *ev){
    QMenu *menu = createStandardContextMenu();
    /*qDebug() << this->textCursor().blockNumber() << this->textCursor().positionInBlock();
    Word *word = wordAtPosition(this->textCursor().blockNumber(), this->textCursor().positionInBlock());
    QList<QAction*> suggestionList;
    if(word != NULL) {
      foreach(QString word, word->suggestions) {
        QAction *suggestionAction = menu->addAction(word);
        connect(suggestionAction, &QAction::triggered, this, [=]() { Something });
        suggestionList.append(suggestionAction);
      }
      menu->addSeparator();
      QAction *ignore = menu->addAction(tr("Ignore"));
      connect(ignore, &QAction::triggered, this, [word]() { word->ignore(); });
      QAction *ignoreAll = menu->addAction(tr("Ignore All"));
      connect(ignoreAll, &QAction::triggered, this, [=]() { foreach(Word *wordP, wordList) { if(wordP->word == word->word) { wordP->ignore(); }} });
      QAction *addToDictionary = menu->addAction(tr("Add to Dictionary"));
      connect(addToDictionary, &QAction::triggered, this, [word, this]() { hunspell->add(word->word.toStdString()); });
    }*/
    menu->exec(ev->globalPos());
    delete menu;
}

void PlainTextEditor::keyPressEvent(QKeyEvent *ev) {
  //Check spelling when copy/paste
  if(ev->matches(QKeySequence::Paste) or ev->matches(QKeySequence::Cut)) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    int epos = this->textCursor().position() + clipboard->text().size();
    //qDebug() << this->textCursor().position() << epos;
    QTimer::singleShot(100, this, [=]() { emit CheckSpelling(this->textCursor().position(), epos); });
  }

  QPlainTextEdit::keyPressEvent(ev);
}
