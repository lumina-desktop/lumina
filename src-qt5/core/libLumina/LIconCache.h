//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for loading/serving icon files 
// from the icon theme or local filesystem
//===========================================
#include <QHash>
#include <QIcon>
#include <QPixmap>
#include <QFileSystemWatcher>
#include <QString>
#include <QFile>
#include <QDateTime>

//Data structure for saving the icon/information internally
struct icon_data{
  QString fullpath;
  QDateTime lastread;
  QList<QLabel*> pendingLabels;
  QList<QAbstractButton*> pendingButtons;
  QIcon icon;
  QIcon thumbnail;
};

class LIconCache : public QObject{
	Q_OBJECT
public:
	LIconCache(QObject *parent = 0);
	~LIconCache();
	//Icon Checks
	bool exists(QString icon);
	bool isLoaded(QString icon);
	QString findFile(QString icon); //find the full path of a given file/name (searching the current Icon theme)
	
	void loadIcon(QAbstractButton *button, QString icon, bool noThumb = false);
	void loadIcon(QLabel *label, QString icon, bool noThumb = false);

private:
	QHash<QString, icon_data> HASH;
	QFileSystemWatcher *WATCHER;

	icon_data createData(QString icon);
	QStringList getChildIconDirs(QString path); //recursive function to find directories with icons in them

	static void ReadFile(LIconCache *obj, QString id, QString path);

private slots:
	void IconLoaded(QString id, QDateTime sync, QByteArray *data);
 
signals:
	void InternalIconLoaded(QString, QDateTime, QByteArray*); //INTERNAL SIGNAL - DO NOT USE in other classes/objects
	void IconAvailable(QString); //way for classes to listen/reload icons as they change
};
