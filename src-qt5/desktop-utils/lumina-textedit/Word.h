#include <QTextEdit>

#pragma once

class Word {
  public:
  Word(QString _word, QStringList _sugg, QTextEdit::ExtraSelection _sel, int _blockNum, int _position) : word(_word), suggestions(_sugg), sel(_sel), blockNum(_blockNum), position(_position) {
    ignored = false;
  }

  void ignore() { ignored = true; }
  bool isIgnored() { return ignored; }

  QString word;
  QStringList suggestions;
  QTextEdit::ExtraSelection sel;
  bool ignored;
  int blockNum;
  int position;
};
