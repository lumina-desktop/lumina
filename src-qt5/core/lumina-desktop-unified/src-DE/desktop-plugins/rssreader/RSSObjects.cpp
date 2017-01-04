//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RSSObjects.h"
#include <QNetworkRequest>
#include <QXmlStreamReader>

#include "LSession.h"

//============
//    PUBLIC
//============
RSSReader::RSSReader(QObject *parent, QString settingsPrefix) : QObject(parent){
  NMAN = new QNetworkAccessManager(this);
  connect(NMAN, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
  connect(NMAN, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)) );

  setprefix = settingsPrefix;
  syncTimer = new QTimer(this);
    syncTimer->setInterval(300000); //5 minutes
    connect(syncTimer, SIGNAL(timeout()), this, SLOT(checkTimes()));
  syncTimer->start();
}

RSSReader::~RSSReader(){

}

//Information retrieval
QStringList RSSReader::channels(){
  QStringList urls = hash.keys();
  QStringList ids;
  //sort all the channels by title before output
  for(int  i=0; i<urls.length(); i++){
    QString title = hash[urls[i]].title;
    if(title.isEmpty()){ title = "ZZZ"; } //put currently-invalid ones at the end of the list
    ids << title+" : "+hash[urls[i]].originalURL;
  }
  ids.sort();
  //Now strip off all the titles again to just get the IDs
  for(int i=0; i<ids.length(); i++){
    ids[i] = ids[i].section(" : ",-1);
  }
  return ids;
}

RSSchannel RSSReader::dataForID(QString ID){
  QString key =  keyForUrl(ID);
  if(hash.contains(key)){ return hash[key]; }
  else{ return RSSchannel(); }
}

//Initial setup
void RSSReader::addUrls(QStringList urls){
  //qDebug() << "Add URLS:" << urls;
  for(int i=0; i<urls.length(); i++){
    //Note: Make sure we get the complete URL form for accurate comparison later
    QString url = QUrl(urls[i]).toString();
    QString key = keyForUrl(url);
    if(hash.contains(key)){ continue; } //already handled
    RSSchannel blank;
      blank.originalURL = url;
    hash.insert(url, blank); //put the empty struct into the hash for now
    requestRSS(url); //startup the initial request for this url
  }
  emit newChannelsAvailable();
}

void RSSReader::removeUrl(QString ID){
  QString key = keyForUrl(ID);
  if(hash.contains(key)){ hash.remove(key); }
  emit newChannelsAvailable();
}

//=================
//    PUBLIC SLOTS
//=================
void RSSReader::syncNow(){
  QStringList urls = hash.keys();
  for(int i=0; i<urls.length(); i++){
    requestRSS(hash[urls[i]].originalURL);
  }
}

//=================
//         PRIVATE
//=================
QString RSSReader::keyForUrl(QString url){ 
  //get current hash key for this URL
  QStringList keys = hash.keys();
  if(keys.contains(url)){ return url; } //this is already a valid key
  for(int i=0; i<keys.length(); i++){
    if(hash[keys[i]].originalURL == url){ return keys[i]; } //this was an original URL
  }
  return "";
}

void RSSReader::requestRSS(QString url){
  if(!outstandingURLS.contains(url)){
    //qDebug() << "Request URL:" << url;
    NMAN->get( QNetworkRequest( QUrl(url) ) );
    outstandingURLS << url;
  }
}

//RSS parsing functions
RSSchannel RSSReader::readRSS(QByteArray bytes){
  //Note: We could expand this later to support multiple "channel"s per Feed
  //   but it seems like there is normally only one channel anyway
  //qDebug() << "Read RSS:" << bytes.left(100);
  QXmlStreamReader xml(bytes);
  RSSchannel rssinfo;
  //qDebug() << "Can Read XML Stream:" << !xml.hasError();
  if(xml.readNextStartElement()){
    //qDebug() << " - RSS Element:" << xml.name();
    if(xml.name() == "rss" && (xml.attributes().value("version") =="2.0" || xml.attributes().value("version") =="0.91") ){
      while(xml.readNextStartElement()){
        //qDebug() << " - RSS Element:" << xml.name();
        if(xml.name()=="channel"){ rssinfo = readRSSChannel(&xml); }
        else{ xml.skipCurrentElement(); }
      }
    }
  }
  if(xml.hasError()){ qDebug() << " - XML Read Error:" << xml.errorString() << "\n" << bytes; }
  return rssinfo;
}
RSSchannel RSSReader::readRSSChannel(QXmlStreamReader *rss){
  RSSchannel info;
  info.timetolive = -1;
  while(rss->readNextStartElement()){
    //qDebug() << " - RSS Element (channel):" <<rss->name();
    if(rss->name()=="item"){ info.items << readRSSItem(rss); }
    else if(rss->name()=="title"){ info.title = rss->readElementText(); }
    else if(rss->name()=="link"){ 
      QString txt = rss->readElementText(); 
      if(!txt.isEmpty()){ info.link = txt; } 
    }
    else if(rss->name()=="description"){ info.description = rss->readElementText(); }
    else if(rss->name()=="lastBuildDate"){ info.lastBuildDate = RSSDateTime(rss->readElementText()); }
    else if(rss->name()=="pubDate"){ info.lastPubDate = RSSDateTime(rss->readElementText()); }
    else if(rss->name()=="image"){ readRSSImage(&info, rss); }
    //else if(rss->name()=="skipHours"){ info.link = rss->readElementText(); }
    //else if(rss->name()=="skipDays"){ info.link = rss->readElementText(); }
    else if(rss->name()=="ttl"){ info.timetolive = rss->readElementText().toInt(); }
    else{ rss->skipCurrentElement(); }
  }
  return info;
}

RSSitem RSSReader::readRSSItem(QXmlStreamReader *rss){
  RSSitem item;
  while(rss->readNextStartElement()){
    //qDebug() << " - RSS Element (Item):" << rss->name();
    if(rss->name()=="title"){ item.title = rss->readElementText(); }
    else if(rss->name()=="link"){ item.link = rss->readElementText(); }
    else if(rss->name()=="description"){ item.description = rss->readElementText(); }
    else if(rss->name()=="comments"){ item.comments_url = rss->readElementText(); }
    else if(rss->name()=="author"){ 
      //Special handling - this field can contain both email and name
      QString raw = rss->readElementText(); 
      if(raw.contains("@")){  
        item.author_email = raw.split(" ").filter("@").first(); 
        item.author = raw.remove(item.author_email).remove("(").remove(")").simplified(); //the name is often put within parentheses after the email
      }else{ item.author = raw; }
    }
    else if(rss->name()=="guid"){ item.guid = rss->readElementText(); }
    else if(rss->name()=="pubDate"){ item.pubdate = RSSDateTime(rss->readElementText()); }
    else{ rss->skipCurrentElement(); }
  }
  return item;
}

void RSSReader::readRSSImage(RSSchannel *item, QXmlStreamReader *rss){
  while(rss->readNextStartElement()){
    //qDebug() << " - RSS Element (Image):" << rss->name();
    if(rss->name()=="url"){ item->icon_url = rss->readElementText(); }
    else if(rss->name()=="title"){ item->icon_title = rss->readElementText(); }
    else if(rss->name()=="link"){ item->icon_link = rss->readElementText(); }
    else if(rss->name()=="width"){ item->icon_size.setWidth(rss->readElementText().toInt()); }
    else if(rss->name()=="height"){ item->icon_size.setHeight(rss->readElementText().toInt()); }
    else if(rss->name()=="description"){ item->icon_description = rss->readElementText(); }
  }
  //Go ahead and kick off the request for the icon
  if(!item->icon_url.isEmpty()){ requestRSS(item->icon_url); }
}

QDateTime RSSReader::RSSDateTime(QString datetime){
  return QDateTime::fromString(datetime, Qt::RFC2822Date);
}

//=================
//    PRIVATE SLOTS
//=================
void RSSReader::replyFinished(QNetworkReply *reply){
  QString url = reply->request().url().toString();
  //qDebug() << "Got Reply:" << url;
  QString key = keyForUrl(url); //current hash key for this URL
  QByteArray data = reply->readAll();
  outstandingURLS.removeAll(url);
  if(data.isEmpty()){
    //qDebug() << "No data returned:" << url;
    //see if the URL can be adjusted for known issues
    bool handled = false;
    QUrl redirecturl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if(redirecturl.isValid() && (redirecturl.toString() != url )){
      //New URL redirect - make the change and send a new request
      QString newurl = redirecturl.toString();
      //qDebug() << " - Redirect to:" << newurl;
      if(hash.contains(key) && !hash.contains(newurl)){
        hash.insert(newurl, hash.take(key) ); //just move the data over to the new url
        requestRSS(newurl);
        emit newChannelsAvailable();
        handled = true;
      }      
    }
    if(!handled && hash.contains(key) ){ 
      emit rssChanged(hash[key].originalURL);
    }
    return;
  }

  if(!hash.contains(key)){ 
    //qDebug() << " - hash does not contain URL:" << url;
    //URL removed from list while a request was outstanding?
    //Could also be an icon fetch response
    QStringList keys = hash.keys();
    for(int i=0; i<keys.length(); i++){
      //qDebug() << " - Check for icon URL:" << hash[keys[i]].icon_url;
      if(hash[keys[i]].icon_url.toLower() == url.toLower()){ //needs to be case-insensitive
        //Icon fetch response
        RSSchannel info = hash[keys[i]];
        QImage img = QImage::fromData(data);
        info.icon = QIcon( QPixmap::fromImage(img) );
        //qDebug() << "Got Icon response:" << url << info.icon;
        hash.insert(keys[i], info); //insert back into the hash
        emit rssChanged( hash[keys[i]].originalURL );
        break;
      }
    }
    reply->deleteLater();
  }else{
    //RSS reply
    RSSchannel info = readRSS(data); //QNetworkReply can be used as QIODevice
    reply->deleteLater(); //clean up
    //Validate the info and announce any changes
    if(info.title.isEmpty() || info.link.isEmpty() || info.description.isEmpty()){ 
      qDebug() << "Missing XML Information:" << url << info.title << info.link << info.description;
      return; 
    } //bad info/read
    //Update the bookkeeping elements of the info
    if(info.timetolive<=0){ info.timetolive = LSession::handle()->DesktopPluginSettings()->value(setprefix+"default_interval_minutes", 60).toInt(); }
    if(info.timetolive <=0){ info.timetolive = 60; } //error in integer conversion from settings?
    info.lastsync = QDateTime::currentDateTime(); info.nextsync = info.lastsync.addSecs(info.timetolive * 60); 
    //Now see if anything changed and save the info into the hash
    bool changed = (hash[key].lastBuildDate.isNull() || (hash[key].lastBuildDate < info.lastBuildDate) );
    bool newinfo = false;
    if(changed){ newinfo = hash[key].title.isEmpty(); } //no previous info from this URL
    info.originalURL = hash[key].originalURL; //make sure this info gets preserved across updates
    if(!hash[key].icon.isNull()){ info.icon = hash[key].icon; } //copy over the icon from the previous reply
    hash.insert(key, info);
    if(newinfo){ emit newChannelsAvailable(); } //new channel
    else if(changed){ emit rssChanged(info.originalURL); } //update to existing channel
  }
}

void RSSReader::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors){
  int ok = 0;
  qDebug() << "SSL Errors Detected (RSS Reader):" << reply->url();
  for(int i=0; i<errors.length(); i++){
    if(errors[i].error()==QSslError::SelfSignedCertificate || errors[i].error()==QSslError::SelfSignedCertificateInChain){ ok++; }
    else{ qDebug() << "Unhandled SSL Error:" << errors[i].errorString(); }
  }
  if(ok==errors.length()){  qDebug() << " - Permitted:" << reply->url(); reply->ignoreSslErrors(); }
  else{ qDebug() << " - Denied:" << reply->url(); }
}

void RSSReader::checkTimes(){
  if(LSession::handle()->DesktopPluginSettings()->value(setprefix+"manual_sync_only", false).toBool()){ return; }
  QStringList urls = hash.keys();
  QDateTime cdt = QDateTime::currentDateTime();
  for(int i=0; i<urls.length(); i++){
    if(hash[urls[i]].nextsync < cdt){ requestRSS(urls[i]); }
  }
}
