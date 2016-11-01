//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CALCULATOR_VALIDATOR_H
#define _LUMINA_CALCULATOR_VALIDATOR_H

#define VALIDCHARS QString("x*+-/^%eE().0123456789#acosinthqrlog\u03C0")
#define NOSTARTCHARS QString("x*/^%)eE.")
#define NOENDCHARS QString("x*/^(eE.#")
#define NOCHANGE QString("().#")

#include <QValidator>
#include <QString>
#include <QDebug>

class EqValidator : public QValidator{
	Q_OBJECT
public:
	EqValidator(QObject *parent = 0) : QValidator(parent){}
	~EqValidator(){}

	virtual void fixup(QString &input) const{
	  if(input.isEmpty()){ return; }
	  if( NOSTARTCHARS.contains(input.left(1)) && !NOCHANGE.contains(input.left(1)) ){ input.prepend("1"); }
	  if( NOENDCHARS.contains(input.right(1)) && !NOCHANGE.contains(input.right(1)) ){ input.append("1"); }
	}

	virtual QValidator::State validate(QString &input, int&pos) const {
	  //qDebug() << "Got validate:" << input << pos;
	  if(pos>0 && !VALIDCHARS.contains(input[pos-1])){ return QValidator::Invalid; }
	  if(!input.isEmpty() && NOSTARTCHARS.contains(input.left(1))){ return QValidator::Intermediate; }
	  if(!input.isEmpty() && NOENDCHARS.contains(input.right(1))){ return QValidator::Intermediate; }
	  return QValidator::Acceptable;
	}
};
#endif
