#include <QDebug>
#include <QApplication>

#include <framework-OSInterface.h>

/*
class tester : public QObject{
	Q_OBJECT
public slots:
	void finished(){ QApplication::exit(0); }

public:
	QTimer *timer;

	tester(){
	  timer = new QTimer(this);
	    timer->setInterval(5000);
	    timer->setSingleShot(true);
	    connect(timer, SIGNAL(timeout()), this, SLOT(finished()) );
	}

};
*/

int main(int argc, char** argv){

  QApplication A(argc,argv);
  OSInterface OS;
  OS.start();
  QTimer *timer = new QTimer();
	    timer->setInterval(5000);
	    timer->setSingleShot(true);
	    QObject::connect(timer, SIGNAL(timeout()), &A, SLOT(quit()) );
  timer->start();
  int ret = A.exec();
    qDebug() << " - Finished";
  return ret;
}
