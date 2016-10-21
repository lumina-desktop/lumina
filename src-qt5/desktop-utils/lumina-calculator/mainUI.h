//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CALCULATOR_MAIN_UI_H
#define _LUMINA_CALCULATOR_MAIN_UI_H

#include <QMainWindow>
#include <QString>
#include <QChar>
#include <QListWidgetItem>

namespace Ui{
	class mainUI;
};

class mainUI : public QMainWindow{
	Q_OBJECT
public:
	mainUI();
	~mainUI();

public slots:
	void updateIcons();

private slots:
	void start_calc();
	void clear_calc();
	void captureButton1();
	void captureButton2();
	void captureButton3();
	void captureButton4();
	void captureButton5();
	void captureButton6();
	void captureButton7();
	void captureButton8();
	void captureButton9();
	void captureButton0();
	void captureButtonSubtract();
	void captureButtonAdd();
	void captureButtonDivide();
	void captureButtonMultiply();
//    void captureButtonEqual();
	void captureButtonDecimal();
	void captureButtonPercent();

	void insert_history(QListWidgetItem *it);
	void copy_to_clipboard(QListWidgetItem *it);
	void checkInput(const QString&);

private:
	Ui::mainUI *ui;
	double performOperation(double LHS, double RHS, QChar symbol);
	double strToNumber(QString str); //this is highly-recursive
};
#endif
