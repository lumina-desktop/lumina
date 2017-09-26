#ifndef FMWEBDAV_H
#define FMWEBDAV_H

#include <QtCore>
#include <QtNetwork>
#include <QNetworkAccessManager>

public:

    enum QWebdavConnectionType {HTTP = 1, HTTPS};

    QString hostname() const;
    int port() const;
    QString rootPath() const;
    QString username() const;
    QString password() const;
    QWebdavConnectionType connectionType() const;
    bool isSSL() const;

 void setConnectionSettings( const QWebdavConnectionType connectionType, const QString *hostname, const QString *rootPath = "/", const QString *username = "", const QString *password = "", int *port = 0;


private:

    QString wdRootPath;
    QString wdUsername;
    QString wdPassword;
    QUrl wdBaseUrl;
    QWebdavConnectionType wdCurrentConnectionType;






#endif // FMWEBDAV
