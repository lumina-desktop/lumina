#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QColor>

#include "../glw-base.h"

class colorchange : public QObject{
	Q_OBJECT
private:
  GLW_Base *base;
  QTimer *timer;

public slots:
  void toggle(){
    static bool current = false;
    base->setBackgroundColor( current ? QColor(Qt::blue) : QColor(Qt::red));
    current = !current;
  }

public:
  colorchange(GLW_Base *parent) : QObject(){
    base = parent;
    timer = new QTimer(this);
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(toggle()) );
    timer->start();
  }

};
