//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_FM_SLIDESHOW_WIDGET_H
#define _LUMINA_FM_SLIDESHOW_WIDGET_H

#include <QList>
#include <QWidget>
#include <QObject>

#include "../DirData.h"

namespace Ui{
	class SlideshowWidget;
};

class SlideshowWidget : public QWidget{
	Q_OBJECT
public:
	SlideshowWidget(QWidget *parent = 0);
	~SlideshowWidget();

public slots:
	void ClearImages();
	void LoadImages(QList<LFileInfo> list);

	void refresh(); //Note: This should be called right after the widget becomes visible

	//Theme change functions
	void UpdateIcons();
	void UpdateText();

private:
	Ui::SlideshowWidget *ui;
	void UpdateImage();
	double zoom;
	
private slots:
	// Picture rotation options
	void on_combo_image_name_currentIndexChanged(int index);
	void on_tool_image_goEnd_clicked();
	void on_tool_image_goNext_clicked();
	void on_tool_image_goPrev_clicked();
	void on_tool_image_goBegin_clicked();

	// Picture modification options
	void on_tool_image_remove_clicked();
	void on_tool_image_rotateleft_clicked();
	void on_tool_image_rotateright_clicked();
	void on_tool_image_zoomin_clicked();
	void on_tool_image_zoomout_clicked();

};
#endif