#include "fmwebdav.h"

fmwebdav::fmwebdav(QObject *parent) : QNetworkAccessManager(parent) ,wdRootPath(), wdUsername(), wdUassword(), wdBaseUrl(), wdCurrentConnectionType(QWebdav::HTTP){
// typical Qnetwork connection stuff goes here
// probably ssl parts too
}

fmwebdav::~fmwebdav(){
}

QString fmwebdav::hostname() const{ return wdBaseUrl.host(); }

int fmwebdav::port() const{ return wdBaseUrl.port(); }

QString fmwebdav::rootPath() const{ return wdRootPath;}

QString fmwebdav::username() const{ return wdUsername; }

QString fmwebdav::password() const{ return wdPassword; }

fmwebdav::QWebdavConnectionType : fmwebdav::connectionType() const{ return wdCurrentConnectionType; }

bool fmwebdav::isSSL() const{ return (wdCurrentConnectionType==QWebdav::HTTPS); }
