#include <QDebug>
#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QObject>
#include <QStringList>
#include <QTimer>

class ListData : public QObject{
	Q_OBJECT
	Q_PROPERTY(QList<QScreen*> list READ list NOTIFY listchanged);
signals:
	void listchanged();

private:
	QStringList m_list;
	//QTimer *timer;
	//bool increasing;

private slots:
	void changeList(){
         /*if(m_list.isEmpty()){ increasing = true; }
         else if(m_list.length() == 5){ increasing = false; }
         if(increasing){  m_list << "Text - "+QString::number( m_list.length()+1 ); }
         else{ m_list.takeAt(m_list.count()-1); }
	  emit listchanged();*/
	}

public:
	ListData() : QObject(){
          /*increasing = true;
	  timer = new QTimer(this);
	  timer->setInterval(1000);
	  timer->setSingleShot(false);
	  connect(timer, SIGNAL(timeout()), this, SLOT(changeList()) );
          timer->start();*/
	}

	QList<QScreen*> list() { return QApplication::screens(); } //m_list; }
};
