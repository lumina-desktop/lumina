//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class creates and manages a virtual "root" window
//   for all monitors at all times.
//===========================================
#ifndef _LUMINA_ROOT_WINDOW_H
#define _LUMINA_ROOT_WINDOW_H



class RootWindow : public QWidget{
	Q_OBJECT
public:
	enum ScaleType{ SingleColor, Stretch, Full, Fit, Center, Tile, BottomLeft, BottomRight, BottomCenter, \
	  TopLeft, TopRight, TopCenter, CenterLeft, CenterRight};

	RootWindow();
	~RootWindow();
	
private:
	struct screeninfo{
	  QString id;
	  QRect area;
	  QString file;
	  ScaleType scale;
	  QPixmap wallpaper;
	};

	QList<screeninfo> WALLPAPERS;
	void updateScreenPixmap(ScreenInfo *info); //used for recalculating the wallpaper pixmap based on file/area/scale as needed

public slots:
	void ResizeRoot();
	void ChangeWallpaper(QString id, RootWindow::ScaleType scale, QString file);
	    //Note: for "SingleColor" scaling the "file" variable should be "rgb(R,G,B)" or "#hexcode"

private slots:

protected:
	void paintEvent(QPaintEvent *ev);

signals:
	void RootResized();
	void NewScreens(QStringList); // [screen_id_1, screen_id_2, etc..]
	void RemovedScreens(QStringList); // [screen_id_1, screen_id_2, etc..]

};

#endif
