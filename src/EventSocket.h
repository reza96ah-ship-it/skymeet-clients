#pragma once

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>

class EventSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString lastMessage READ lastMessage NOTIFY lastMessageChanged)

public:
    explicit EventSocket(QObject *parent = nullptr);

    QString url() const { return url_; }
    void setUrl(const QString &url);
    bool connected() const { return connected_; }
    QString lastMessage() const { return lastMessage_; }

    Q_INVOKABLE void connectToServer(const QString &token = QString());
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void subscribe(const QString &topic);
    Q_INVOKABLE void sendJson(const QString &json);

signals:
    void urlChanged();
    void connectedChanged();
    void lastMessageChanged();
    void eventReceived(const QString &type, const QString &json);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    void setConnected(bool value);

    QWebSocket socket_;
    QString url_ = "ws://127.0.0.1:8088/ws/v1/streams";
    bool connected_ = false;
    QString lastMessage_;
};
