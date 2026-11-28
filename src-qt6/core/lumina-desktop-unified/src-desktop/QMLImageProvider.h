//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_QML_IMAGE_PROVIDER_H
#define _LUMINA_DESKTOP_QML_IMAGE_PROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QPixmap>
#include <QSize>

class QMLImageProvider : public QQuickImageProvider{
public:
	QMLImageProvider(QQmlImageProviderBase::ImageType);
	~QMLImageProvider();

	//static QMLImageProvider* instance();

	virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
	virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
};

#endif
