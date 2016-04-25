//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for showing a lot of text in a scrollable format (instead of QMessageBox)
//===========================================
#ifndef _LUMINA_FILE_MANAGER_SCROLL_DIALOG_H
#define _LUMINA_FILE_MANAGER_SCROLL_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>

class ScrollDialog : public QDialog{
	Q_OBJECT
	
private:
	QDialogButtonBox *buttons;
	QTextEdit *label;
	QVBoxLayout *layout1;

public:
	ScrollDialog(QWidget *parent = 0) : QDialog(parent){
	  //Create the widgets
	  buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	  label = new QTextEdit(this);
	    label->setReadOnly(true);
	    label->setWordWrapMode(QTextOption::NoWrap);
	  layout1 = new QVBoxLayout(this);
	  //Put them in the dialog
	  layout1->addWidget(label);
	  layout1->addWidget(buttons);
	  this->setLayout(layout1);
	  //Connect signals/slots
	  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()) );
	  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()) );
	  //Set a useful size/position
	  this->resize(400,200);
	  if(parent!=0){
	    QPoint ctr = parent->mapToGlobal(parent->geometry().center());
	    this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
	  }
	}
	~ScrollDialog(){}

	void setText(QString txt){
	  label->setPlainText(txt);
	  //this->resize( label->fontMetrics().width(txt.section("\n",0,0))+30, this->height());
	}

};
#endif