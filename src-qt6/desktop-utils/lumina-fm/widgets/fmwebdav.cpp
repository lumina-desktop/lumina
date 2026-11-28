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

void QWebdav::setConnectionSettings(const QWebdavConnectionType connectionType, const QString *hostname, const QString *rootPath, const QString *username, const QString *password, int *port){
  wdRootPath = rootPath;
  if ((wdRootPath.endsWith("/")){ wdRootPath.chop(1); }
  wdCurrentConnectionType = connectionType;
  wdBaseUrl.setScheme();
  wdBaseUrl.setHost(hostname);
  wdBaseUrl.setPath(rootPath);
  if (port != 0) { // use user-defined port number if not 80 or 443
    if ( ! ( ( (port == 80) && (wdCurrentConnectionType==QWebdav::HTTP) ) || ( (port == 443) && (wdCurrentConnectionType==QWebdav::HTTPS) ) ) ){ wdBaseUrl.setPort(port); }
  wdUsername = username;
  wdPassword = password;
}
