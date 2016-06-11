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
#include <QTimer>
#include <QXmlStreamReader> //Contained in the Qt "core" module - don't need the full "xml" module for this
#include <QSslError>

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
  QString icon_url, icon_title, icon_link, icon_description;
  QSize icon_size;
  //All items within this channel
  QList<RSSitem> items;

  //Optional RSS2 elements ignored:
  // "cloud", "textInput", "rating", "language", "copyright", "managingEditor", "webMaster",
  // "category", "generator", "docs"

  //Internal data for bookkeeping
  QDateTime lastsync, nextsync;
  QString originalURL; //in case it was redirected to some "fixed" url later
};

class RSSReader : public QObject{
	Q_OBJECT
public:
	RSSReader(QObject *parent, QString settingsPrefix);
	~RSSReader();

	//Information retrieval
	QStringList channels(); //returns all ID's
	RSSchannel dataForID(QString ID);

	//Initial setup
	void addUrls(QStringList urls);
	void removeUrl(QString ID);
	
public slots:
	void syncNow(); //not generally needed

private:
	//Internal data objects
	QHash<QString, RSSchannel> hash; // ID/data
        QString setprefix;
	QTimer *syncTimer;	
	QNetworkAccessManager *NMAN;
        QStringList outstandingURLS;


	//Simple hash data search functions
        QString keyForUrl(QString url);

	//Network request function
	void requestRSS(QString url);
	
	//RSS parsing functions
	RSSchannel readRSS(QByteArray bytes);
	RSSchannel readRSSChannel(QXmlStreamReader *rss);
	RSSitem readRSSItem(QXmlStreamReader *rss);
        void readRSSImage(RSSchannel *item, QXmlStreamReader *rss);
	QDateTime RSSDateTime(QString datetime);

private slots:
	void replyFinished(QNetworkReply *reply);
	void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
	void checkTimes();

signals:
	void rssChanged(QString); //ID
	void newChannelsAvailable();
};

#endif
