#ifndef _LUMINA_DESKTOP_BOOT_SPLASHSCREEN_H
#define _LUMINA_DESKTOP_BOOT_SPLASHSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPixmap>
#include <QPoint>
#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif

namespace Ui{
	class BootSplash;
};

class BootSplash : public QWidget{
	Q_OBJECT
private:
	Ui::BootSplash *ui;

	void generateTipOfTheDay();

public:
	BootSplash();
	~BootSplash(){}

	void showScreen(QString loading); //update icon, text, and progress
	void showText(QString txt); //will only update the text, not the icon/progress
};

#endif
