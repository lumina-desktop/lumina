//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RSSObjects.h"
#include <QNetworkRequest>
#include <QXmlStreamReader>

//============
//    PUBLIC
//============
RSSReader::RSSReader(QObject *parent) : QObject(parent){
  NMAN = new QNetworkAccessManager(this);
}

RSSReader::~RSSReader(){

}

//Information retrieval
QStringList RSSReader::channels(){
  return QStringList( data.keys() );
}

Rsschannel RSSReader::dataForID(QString ID){

}

//Initial setup
void RSSReader::addUrls(){

}

void RSSReader::removeUrl(QString ID){

}

//=================
//    PUBLIC SLOTS
//=================
void RSSReader::syncNow(){

}

//=================
//         PRIVATE
//=================
void RSSchannel::requestRSS(QString url){
  NMAN->get( QNetworkRequest( QUrl(url) ) );
}

//RSS parsing functions
RSSchannel RSSReader::readRSS(QIODevice *device){
  QXmlStreamReader xml(device);
  RSSchannel rssinfo;
  if(xml.readNextStartElement()){
    if(xml.name() = "rss" && xml.attributes().value("version" =="2.0")){
      while(xml.readNextStartElement()){
        if(xml.name()=="channel"){ rssinfo = readRSSChannel(&xml); }
      }
    }
  }
  return rssinfo;
}
RSSchannel RSSReader::readRSSChannel(QXmlStreamReader *rss){
  RSSchannel info;
  while(rss.readNextStartElement()){
    if(rss.name()=="item"){ info.items << readRSSItem(rss); }
    else if(rss.name()=="title"){ info.title = rss.readElementText(); }
    else if(rss.name()=="link"){ info.link = rss.readElementText(); }
    else if(rss.name()=="description"){ info.description = rss.readElementText(); }
    else if(rss.name()=="lastBuildDate"){ info.lastBuildDate = RSSDateTime(rss.readElementText()); }
    else if(rss.name()=="pubDate"){ info.lastPubDate = RSSDateTime(rss.readElementText()); }
    else if(rss.name()=="image"){ info.link = rss.readElementText(); }
    else if(rss.name()=="guid"){ info.guid = rss.readElementText(); }
    //else if(rss.name()=="skipHours"){ info.link = rss.readElementText(); }
    //else if(rss.name()=="skipDays"){ info.link = rss.readElementText(); }
    else if(rss.name()=="ttl"){ info.timetolive = rss.readElementText().toInt(); }
  }
  return info;
}
RSSitem RSSReader::readRSSItem(QXmlStreamReader *rss){
  
}

QDateTime RSSReader::RSSDateTime(QString datetime){

}

//=================
//    PRIVATE SLOTS
//=================
void RSSReader::replyFinished(QNetworkReply *reply){
  RSSchannel info = ReadRSS(reply); //QNetworkReply can be used as QIODevice
  reply->deleteLater(); //clean up
  //Validate the info and announce any changes


}
