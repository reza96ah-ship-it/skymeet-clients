#include "ApiClient.h"

#include <QUrl>
#include <QUrlQuery>
#include <QVariantList>
#include <QDebug>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

void ApiClient::setBaseUrl(const QString &url)
{
    QString clean = url.trimmed();
    while (clean.endsWith('/')) {
        clean.chop(1);
    }
    if (clean.isEmpty()) {
        clean = "http://127.0.0.1:8088";
    }
    if (baseUrl_ == clean) {
        return;
    }
    baseUrl_ = clean;
    emit baseUrlChanged();
}

void ApiClient::loadSettings()
{
    QSettings s;
    setBaseUrl(s.value("api/baseUrl", baseUrl_).toString());
    setToken(s.value("api/token", "").toString());
}

void ApiClient::saveSettings()
{
    QSettings s;
    s.setValue("api/baseUrl", baseUrl_);
    s.setValue("api/token", token_);
}

void ApiClient::clearError()
{
    if (!lastError_.isEmpty()) {
        lastError_.clear();
        emit lastErrorChanged();
    }
}

void ApiClient::setLoading(bool value)
{
    if (loading_ == value) {
        return;
    }
    loading_ = value;
    emit loadingChanged();
}

void ApiClient::setError(const QString &error)
{
    lastError_ = error;
    emit lastErrorChanged();
}

void ApiClient::setToken(const QString &token)
{
    if (token_ == token) {
        return;
    }
    token_ = token;
    emit tokenChanged();
    emit authenticatedChanged();
    saveSettings();
}

QNetworkRequest ApiClient::makeRequest(const QString &path) const
{
    QUrl url(baseUrl_ + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Accept", "application/json");
    if (!token_.isEmpty()) {
        const QByteArray headerName = QByteArray("Author") + QByteArray("ization");
        const QByteArray headerValue = QByteArray("Bear") + QByteArray("er ") + token_.toUtf8();
        req.setRawHeader(headerName, headerValue);
    }
    return req;
}

void ApiClient::request(const QString &path,
                        Method method,
                        const QJsonObject &body,
                        std::function<void(const QJsonObject &)> onOk,
                        const QString &operationName)
{
    setLoading(true);
    clearError();

    QNetworkRequest req = makeRequest(path);
    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = nullptr;

    switch (method) {
    case Method::Get:
        reply = nam_.get(req);
        break;
    case Method::Post:
        reply = nam_.post(req, payload);
        break;
    case Method::Patch:
        reply = nam_.sendCustomRequest(req, "PATCH", payload);
        break;
    case Method::Delete:
        reply = nam_.deleteResource(req);
        break;
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, onOk, operationName]() {
        reply->deleteLater();
        setLoading(false);

        const QByteArray raw = reply->readAll();
        const bool networkOk = reply->error() == QNetworkReply::NoError;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);

        if (!networkOk) {
            QString msg = QString("%1: %2").arg(operationName, reply->errorString());
            if (!raw.isEmpty()) {
                QJsonParseError errorParse;
                QJsonDocument errorDoc = QJsonDocument::fromJson(raw, &errorParse);
                if (errorParse.error == QJsonParseError::NoError && errorDoc.isObject()) {
                    msg += " - " + errorMessage(errorDoc.object(), QString::fromUtf8(raw.left(300)));
                } else {
                    msg += " - " + QString::fromUtf8(raw.left(300));
                }
            }
            setError(msg);
            emit operationFailed(operationName, msg);
            return;
        }
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            const QString msg = QString("%1: invalid JSON response: %2").arg(operationName, QString::fromUtf8(raw.left(180)));
            setError(msg);
            emit operationFailed(operationName, msg);
            return;
        }

        QJsonObject root = doc.object();
        if (root.contains("ok") && !root.value("ok").toBool()) {
            const QString msg = errorMessage(root, operationName + " failed");
            setError(msg);
            emit operationFailed(operationName, msg);
            return;
        }

        onOk(root);
        emit operationOk(operationName, operationName + " completed");
    });
}

QString ApiClient::tokenFromResponse(const QJsonObject &root)
{
    QJsonObject data = dataObject(root);
    return data.value("access_token").toString(data.value("token").toString());
}

QJsonArray ApiClient::dataArray(const QJsonObject &root)
{
    if (root.value("data").isArray()) {
        return root.value("data").toArray();
    }
    return {};
}

QJsonObject ApiClient::dataObject(const QJsonObject &root)
{
    if (root.value("data").isObject()) {
        return root.value("data").toObject();
    }
    return {};
}

QString ApiClient::errorMessage(const QJsonObject &root, const QString &fallback)
{
    QJsonObject err = root.value("error").toObject();
    QString msg = err.value("message").toString();
    QString code = err.value("code").toString();
    if (!code.isEmpty() && !msg.isEmpty()) {
        return code + ": " + msg;
    }
    if (!msg.isEmpty()) {
        return msg;
    }
    return fallback;
}

void ApiClient::registerUser(const QString &organizationName,
                             const QString &organizationSlug,
                             const QString &displayName,
                             const QString &email,
                             const QString &password,
                             const QString &role)
{
    QJsonObject body;
    body["organization_name"] = organizationName;
    body["organization_slug"] = organizationSlug;
    body["display_name"] = displayName;
    body["email"] = email;
    body["password"] = password;
    body["role"] = role;

    request("/api/v1/auth/register", Method::Post, body, [this](const QJsonObject &root) {
        const QString t = tokenFromResponse(root);
        if (!t.isEmpty()) {
            setToken(t);
        }
        currentUser_ = dataObject(root).value("user").toObject();
        emit currentUserChanged();
        fetchUsers();
        fetchSessions();
        fetchStreams();
    }, "register");
}

void ApiClient::login(const QString &organizationSlug, const QString &email, const QString &password)
{
    QJsonObject body;
    body["organization_slug"] = organizationSlug;
    body["email"] = email;
    body["password"] = password;

    request("/api/v1/auth/login", Method::Post, body, [this](const QJsonObject &root) {
        const QString t = tokenFromResponse(root);
        if (!t.isEmpty()) {
            setToken(t);
        }
        currentUser_ = dataObject(root).value("user").toObject();
        emit currentUserChanged();
        fetchMe();
        fetchUsers();
        fetchSessions();
        fetchStreams();
    }, "login");
}

void ApiClient::createUser(const QString &displayName,
                           const QString &email,
                           const QString &password,
                           const QString &role)
{
    QJsonObject body;
    body["display_name"] = displayName;
    body["email"] = email;
    body["password"] = password;
    body["role"] = role.isEmpty() ? "user" : role;

    request("/api/v1/users", Method::Post, body, [this](const QJsonObject &) {
        fetchUsers();
    }, "create user");
}

void ApiClient::logout()
{
    request("/api/v1/auth/logout", Method::Post, {}, [this](const QJsonObject &) {
        setToken("");
        currentUser_ = {};
        users_ = {};
        sessions_ = {};
        streams_ = {};
        emit currentUserChanged();
        emit usersChanged();
        emit sessionsChanged();
        emit streamsChanged();
    }, "logout");
}

void ApiClient::fetchMe()
{
    request("/api/v1/auth/me", Method::Get, {}, [this](const QJsonObject &root) {
        currentUser_ = dataObject(root);
        if (currentUser_.contains("user") && currentUser_.value("user").isObject()) {
            currentUser_ = currentUser_.value("user").toObject();
        }
        emit currentUserChanged();
    }, "auth/me");
}

void ApiClient::fetchUsers()
{
    request("/api/v1/users", Method::Get, {}, [this](const QJsonObject &root) {
        users_ = dataArray(root);
        if (users_.isEmpty() && dataObject(root).contains("users")) {
            users_ = dataObject(root).value("users").toArray();
        }
        if (users_.isEmpty() && dataObject(root).contains("items")) {
            users_ = dataObject(root).value("items").toArray();
        }
        emit usersChanged();
    }, "users");
}

void ApiClient::fetchSessions()
{
    request("/api/v1/sessions", Method::Get, {}, [this](const QJsonObject &root) {
        sessions_ = dataArray(root);
        if (sessions_.isEmpty() && dataObject(root).contains("sessions")) {
            sessions_ = dataObject(root).value("sessions").toArray();
        }
        if (sessions_.isEmpty() && dataObject(root).contains("items")) {
            sessions_ = dataObject(root).value("items").toArray();
        }
        emit sessionsChanged();
    }, "sessions");
}

void ApiClient::fetchStreams()
{
    request("/api/v1/streams", Method::Get, {}, [this](const QJsonObject &root) {
        streams_ = dataArray(root);
        if (streams_.isEmpty() && dataObject(root).contains("streams")) {
            streams_ = dataObject(root).value("streams").toArray();
        }
        if (streams_.isEmpty() && dataObject(root).contains("items")) {
            streams_ = dataObject(root).value("items").toArray();
        }
        emit streamsChanged();
    }, "streams");
}

void ApiClient::syncZlmStreams()
{
    request("/api/v1/streams/sync-zlm", Method::Post, {}, [this](const QJsonObject &) {
        fetchStreams();
    }, "sync zlm streams");
}

void ApiClient::createStream(const QString &name,
                             const QString &appName,
                             const QString &streamName,
                             const QString &sourceType,
                             const QString &sourceUrl,
                             const QString &protocol)
{
    QJsonObject body;
    const QString cleanApp = appName.trimmed().isEmpty() ? QStringLiteral("meeting") : appName.trimmed();
    const QString cleanStream = streamName.trimmed().isEmpty() ? QStringLiteral("live") : streamName.trimmed();

    body["name"] = name.trimmed().isEmpty() ? QString("%1/%2").arg(cleanApp, cleanStream) : name.trimmed();
    body["app"] = cleanApp;
    body["stream"] = cleanStream;
    body["source_type"] = sourceType.trimmed().isEmpty() ? QStringLiteral("rtsp_push") : sourceType.trimmed();
    body["source_url"] = sourceUrl.trimmed();
    body["protocol"] = protocol.trimmed().isEmpty() ? QStringLiteral("rtsp") : protocol.trimmed();

    QJsonObject zlm;
    zlm["vhost"] = "__defaultVhost__";
    zlm["app"] = cleanApp;
    zlm["stream"] = cleanStream;
    body["zlm"] = zlm;

    request("/api/v1/streams", Method::Post, body, [this](const QJsonObject &) {
        fetchStreams();
    }, "create stream");
}

void ApiClient::createSession(const QString &title,
                              const QString &description,
                              const QString &appName,
                              const QString &stream,
                              const QString &schema,
                              const QVariantList &userIds)
{
    QJsonArray ids;
    for (const QVariant &v : userIds) {
        const QString id = v.toString();
        if (!id.isEmpty()) {
            ids.append(id);
        }
    }

    QJsonObject body;
    body["title"] = title;
    body["description"] = description;
    body["session_type"] = "meeting";
    body["direction"] = "internal";
    body["app"] = appName.isEmpty() ? "meeting" : appName;
    body["stream"] = stream;
    body["schema"] = schema.isEmpty() ? "webrtc" : schema;
    body["user_ids"] = ids;

    request("/api/v1/sessions", Method::Post, body, [this](const QJsonObject &) {
        fetchSessions();
    }, "create session");
}

void ApiClient::assignUsersToSession(const QString &sessionId, const QVariantList &userIds, const QString &role)
{
    QJsonArray ids;
    for (const QVariant &v : userIds) {
        const QString id = v.toString();
        if (!id.isEmpty()) {
            ids.append(id);
        }
    }

    QJsonObject body;
    body["role"] = role;
    body["user_ids"] = ids;

    request(QString("/api/v1/sessions/%1/users").arg(sessionId), Method::Post, body, [this](const QJsonObject &) {
        fetchSessions();
    }, "assign users");
}

void ApiClient::stopSession(const QString &sessionId, const QString &reason)
{
    QJsonObject body;
    body["reason"] = reason;
    body["force"] = true;

    request(QString("/api/v1/sessions/%1/stop").arg(sessionId), Method::Post, body, [this](const QJsonObject &) {
        fetchSessions();
    }, "stop session");
}

QString ApiClient::jsonText(const QJsonObject &obj) const
{
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

QString ApiClient::arrayText(const QJsonArray &arr) const
{
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

QVariantList ApiClient::selectedUserIdsFromIndexes(const QVariantList &indexes) const
{
    QVariantList ids;
    for (const QVariant &v : indexes) {
        const int index = v.toInt();
        if (index >= 0 && index < users_.size()) {
            QJsonObject user = users_.at(index).toObject();
            QString id = user.value("id").toString();
            if (id.isEmpty() && user.value("_id").isObject()) {
                id = user.value("_id").toObject().value("$oid").toString();
            }
            if (id.isEmpty()) {
                id = user.value("_id").toString();
            }
            if (!id.isEmpty()) {
                ids.push_back(id);
            }
        }
    }
    return ids;
}
