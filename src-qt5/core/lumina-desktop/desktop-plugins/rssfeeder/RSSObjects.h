//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_RSS_READER_PLUGIN_OBJECT_H
#define _LUMINA_DESKTOP_RSS_READER_PLUGIN_OBJECT_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QIcon>

struct RSSitem{
  //Required Fields
  QString title, link, description;

  //Optional Fields
  QString comments_url, author_email, author, guid;
  QDateTime pubdate; //when the item was published
  //IGNORED INFO from RSS2 spec: "category", "source", "enclosure"
};

struct RSSchannel{
  //Required fields
  QString title, link, description;

  //Optional Fields
  QDateTime lastBuildDate, lastPubDate; //last build/publish dates
  // - channel refresh information
  int timetolive; //in minutes - time until next sync should be performed
  //QList<int> skiphours;
  //QStringList skipdays;
  // - icon info
  QIcon icon;
  QString iconurl, icontitle, iconlink;
  QSize iconsize;
  //All items within this channel
  QList<RSSitem> items;

  //Optional RSS2 elements ignored:
  // "cloud", "textInput", "rating", "language", "copyright", "managingEditor", "webMaster",
  // "category", "generator", "docs"
};

class RSSReader : public QObject{
	Q_OBJECT
public:
	RSSReader(QObject *parent);
	~RSSReader();

	//Information retrieval
	QStringList channels(); //returns all ID's
	Rsschannel dataForID(QString ID);

	//Initial setup
	void addUrls();
	void removeUrl(QString ID);
	
public slots:
	void syncNow(); //not generally needed

private:
	QHash<QString, RSSchannel> data; // ID/data
	//Network request functions
	QNetworkAccessManager *NMAN;
	void requestRSS(QString url);
	
	//RSS parsing functions
	RSSchannel readRSS(QByteArray rss);
	RSSchannel readRSSChannel(QXmlStreamReader *rss)
	RSSitem readRSSItem(QXmlStreamReader *rss);
	QDateTime RSSDateTime(QString datetime);

private slots:
	void replyFinished(QNetworkReply *reply);

signals:
	void rssChanged(QString); //ID
	void newChannelsAvailable();
};

#endif
