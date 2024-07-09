#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include "global.h"
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>

class HttpMgr:public QObject, public Singleton<HttpMgr>,
        public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT

public:
    ~HttpMgr();
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id);
private:
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _manager;
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
