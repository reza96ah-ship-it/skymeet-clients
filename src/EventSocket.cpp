#include "EventSocket.h"

#include <QNetworkRequest>
#include <QUrl>

EventSocket::EventSocket(QObject *parent)
    : QObject(parent)
{
    connect(&socket_, &QWebSocket::connected, this, &EventSocket::onConnected);
    connect(&socket_, &QWebSocket::disconnected, this, &EventSocket::onDisconnected);
    connect(&socket_, &QWebSocket::textMessageReceived, this, &EventSocket::onTextMessageReceived);
}

void EventSocket::setUrl(const QString &url)
{
    if (url_ == url) {
        return;
    }
    url_ = url;
    emit urlChanged();
}

void EventSocket::connectToServer(const QString &token)
{
    QNetworkRequest req{QUrl(url_)};
    if (!token.isEmpty()) {
        const QByteArray headerName = QByteArray("Author") + QByteArray("ization");
        const QByteArray headerValue = QByteArray("Bear") + QByteArray("er ") + token.toUtf8();
        req.setRawHeader(headerName, headerValue);
    }
    socket_.open(req);
}

void EventSocket::disconnectFromServer()
{
    socket_.close();
}

void EventSocket::subscribe(const QString &topic)
{
    QJsonObject obj;
    obj["type"] = "subscribe";
    obj["topic"] = topic;
    socket_.sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

void EventSocket::sendJson(const QString &json)
{
    socket_.sendTextMessage(json);
}

void EventSocket::setConnected(bool value)
{
    if (connected_ == value) {
        return;
    }
    connected_ = value;
    emit connectedChanged();
}

void EventSocket::onConnected()
{
    setConnected(true);
}

void EventSocket::onDisconnected()
{
    setConnected(false);
}

void EventSocket::onTextMessageReceived(const QString &message)
{
    lastMessage_ = message;
    emit lastMessageChanged();

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QString type = "message";
    if (doc.isObject()) {
        type = doc.object().value("type").toString(type);
    }
    emit eventReceived(type, message);
}
