//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is the generic container layout for all desktop plugins
//  Simply subclass this when creating a new plugin to enable correct
//    visibility and usage within the desktop window
//===========================================
//  WARNING: Do *not* setup a custom context menu for the entire plugins area!
//     This can prevent access to the general desktop context menu if
//     the plugin was maximized to fill the desktop area!
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_H

#include <QObject>
#include <QFrame>
#include <QWidget>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>

class LDPlugin : public QFrame{
	Q_OBJECT

private:
	QString PLUGID, prefix;
	QSettings *settings;
	QMenu *menu, *contextM;
	QTimer *dragTimer;
	//QRect gridRect; //Current Geometry in grid coordinates.

	void setupMenu();

public:
	LDPlugin(QWidget *parent = 0, QString id="unknown");

	~LDPlugin(){}

	QString ID(){
	  return PLUGID;
	}

	void setContextMenu(QMenu *cmen){ contextM = cmen; }

	QMenu* contextMenu(){ return contextM; }

	virtual QSize defaultPluginSize(){
	  //This needs to be re-implemented in the subclassed plugin
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(1,1); //1x1 grid size
	}

	void savePluginGeometry(QRect geom){
	  settings->setValue(prefix+"geometry/desktopGridPoints", geom);
	  settings->sync();
	}

	QRect loadPluginGeometry(){
	  return settings->value(prefix+"geometry/desktopGridPoints", QRect()).toRect();
	}

	void saveSetting(QString var, QVariant val){
	  //qDebug() << "Saving Setting:" << prefix+var+QString(" = ")+val.toString();
	  settings->setValue(prefix+var, val);
	  settings->sync();
	}

	QVariant readSetting(QString var, QVariant defaultval){
	  return settings->value(prefix+var, defaultval);
	}

	virtual void Cleanup(){
	  //This needs to be re-implemented in the subclassed plugin
	   //This is where any last-minute changes are performed before a plugin is removed permanently
	   //Note1: This is *not* called if the plugin is being temporarily closed
	   //Note2: All the settings for this plugin will be automatically removed after this is finished
	}

	void removeSettings(bool permanent = false){ //such as when a plugin is deleted
	  if(permanent){ Cleanup(); }
	  QStringList list = settings->allKeys().filter(prefix);
	   for(int i=0; i<list.length(); i++){ settings->remove(list[i]); }

	}

	void setGridGeometry(QRect grid){ settings->setValue(prefix+"geometry/gridLocation", grid); }
	QRect gridGeometry(){ return settings->value(prefix+"geometry/gridLocation",QRect()).toRect(); }

public slots:
	virtual void LocaleChange(){
	  //This needs to be re-implemented in the subclassed plugin
	    //This is where all text is set/translated
	  setupMenu();
	}
	virtual void ThemeChange(){
	  //This needs to be re-implemented in the subclassed plugin
	    //This is where all the visuals are set if using Theme-dependant icons.
	  setupMenu();
	}
	void showPluginMenu();

signals:
	void OpenDesktopMenu();
	void CloseDesktopMenu();
	void PluginResized();
	void PluginActivated();

	//Signals for communication with the desktop layout system (not generally used by hand)
	void StartMoving(QString); //ID of plugin
	void StartResizing(QString); //ID of plugin
	void RemovePlugin(QString); //ID of plugin
	void IncreaseIconSize(); // only used for desktop icons
	void DecreaseIconSize(); // only used for desktop icons

private slots:
	void slotStartMove(){
	  QCursor::setPos( this->mapToGlobal(QPoint(this->width()/2, this->height()/2)) );
	  emit StartMoving(PLUGID);
	}

	void slotStartResize(){
	  QCursor::setPos( this->mapToGlobal(QPoint(this->width()/2, this->height()/2)) );
	  emit StartResizing(PLUGID);
	}

	void slotRemovePlugin(){
	  removeSettings(true);
	  emit RemovePlugin(PLUGID);
	}

protected:
	void mousePressEvent(QMouseEvent *ev){
	  if(!dragTimer->isActive() && ev->buttons().testFlag(Qt::LeftButton) ){ dragTimer->start(); }
	  QWidget::mousePressEvent(ev);
	}
	void mouseReleaseEvent(QMouseEvent *ev){
	  if(dragTimer->isActive()){ dragTimer->stop(); }
	  QWidget::mouseReleaseEvent(ev);
	}
	void mouseMoveEvent(QMouseEvent *ev){
	  if(ev->buttons().testFlag(Qt::LeftButton)){
	    if(dragTimer->isActive()){ dragTimer->stop(); }
	    slotStartMove();
	  }
	  QWidget::mouseMoveEvent(ev);
	}
	void resizeEvent(QResizeEvent *ev){
	  emit PluginResized();
	  QFrame::resizeEvent(ev); //do normal processing
	}
};

#endif
