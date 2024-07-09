#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id)
{
    //创建一个HTTP POST请求，并设置请求头和请求体
    QByteArray data = QJsonDocument(json).toJson();
    //通过url构造请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));
    //发送请求，并处理响应, 获取自己的智能指针，构造伪闭包并增加智能指针引用计数
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request, data);
    //设置信号和槽等待发送完成
    QObject::connect(reply, &QNetworkReply::finished, [reply, self, req_id](){
        //处理错误的情况
        if(reply->error() != QNetworkReply::NoError){
            qDebug() << reply->errorString();
            //发送信号通知完成
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK);
            reply->deleteLater();
            return;
        }

        //无错误则读回请求
        QString res = reply->readAll();

        // 解析 JSON 字符串,res需转化为QByteArray
        QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
        //json解析错误
        if(jsonDoc.isNull()){
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_JSON);
            return;
        }

        //json解析错误
        if(!jsonDoc.isObject()){
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_JSON);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        int error = jsonObj["error"].toInt();

        if(error != ErrorCodes::SUCCESS){
            emit self->sig_http_finish(req_id, "", ErrorCodes(error));
            return;
        }

        //发送信号通知完成
        emit self->sig_http_finish(req_id, res, ErrorCodes::SUCCESS);
        reply->deleteLater();
        return;
    });
}

HttpMgr::HttpMgr()
{

}
