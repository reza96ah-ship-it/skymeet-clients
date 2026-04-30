#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSettings>
#include <functional>

class ApiClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)
    Q_PROPERTY(bool authenticated READ authenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QJsonObject currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(QJsonArray users READ users NOTIFY usersChanged)
    Q_PROPERTY(QJsonArray sessions READ sessions NOTIFY sessionsChanged)
    Q_PROPERTY(QJsonArray streams READ streams NOTIFY streamsChanged)

public:
    explicit ApiClient(QObject *parent = nullptr);

    QString baseUrl() const { return baseUrl_; }
    void setBaseUrl(const QString &url);

    QString token() const { return token_; }
    bool authenticated() const { return !token_.isEmpty(); }
    bool loading() const { return loading_; }
    QString lastError() const { return lastError_; }
    QJsonObject currentUser() const { return currentUser_; }
    QJsonArray users() const { return users_; }
    QJsonArray sessions() const { return sessions_; }
    QJsonArray streams() const { return streams_; }

    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void clearError();

    Q_INVOKABLE void registerUser(const QString &organizationName,
                                   const QString &organizationSlug,
                                   const QString &displayName,
                                   const QString &email,
                                   const QString &password,
                                   const QString &role = "admin");

    Q_INVOKABLE void login(const QString &organizationSlug,
                           const QString &email,
                           const QString &password);

    Q_INVOKABLE void createUser(const QString &displayName,
                                const QString &email,
                                const QString &password,
                                const QString &role = "user");

    Q_INVOKABLE void logout();
    Q_INVOKABLE void fetchMe();
    Q_INVOKABLE void fetchUsers();
    Q_INVOKABLE void fetchSessions();
    Q_INVOKABLE void fetchStreams();
    Q_INVOKABLE void syncZlmStreams();

    Q_INVOKABLE void createStream(const QString &name,
                                  const QString &app,
                                  const QString &stream,
                                  const QString &sourceType,
                                  const QString &sourceUrl,
                                  const QString &protocol);

    Q_INVOKABLE void createSession(const QString &title,
                                   const QString &description,
                                   const QString &app,
                                   const QString &stream,
                                   const QString &schema,
                                   const QVariantList &userIds);

    Q_INVOKABLE void assignUsersToSession(const QString &sessionId,
                                           const QVariantList &userIds,
                                           const QString &role = "participant");

    Q_INVOKABLE void stopSession(const QString &sessionId,
                                  const QString &reason = "client_stop");

    Q_INVOKABLE QString jsonText(const QJsonObject &obj) const;
    Q_INVOKABLE QString arrayText(const QJsonArray &arr) const;
    Q_INVOKABLE QVariantList selectedUserIdsFromIndexes(const QVariantList &indexes) const;

signals:
    void baseUrlChanged();
    void tokenChanged();
    void authenticatedChanged();
    void loadingChanged();
    void lastErrorChanged();
    void currentUserChanged();
    void usersChanged();
    void sessionsChanged();
    void streamsChanged();
    void operationOk(const QString &name, const QString &message);
    void operationFailed(const QString &name, const QString &message);

private:
    enum class Method { Get, Post, Patch, Delete };

    void request(const QString &path,
                 Method method,
                 const QJsonObject &body,
                 std::function<void(const QJsonObject &)> onOk,
                 const QString &operationName);

    QNetworkRequest makeRequest(const QString &path) const;
    void setLoading(bool value);
    void setError(const QString &error);
    void setToken(const QString &token);
    static QString tokenFromResponse(const QJsonObject &root);
    static QJsonArray dataArray(const QJsonObject &root);
    static QJsonObject dataObject(const QJsonObject &root);
    static QString errorMessage(const QJsonObject &root, const QString &fallback);

    QNetworkAccessManager nam_;
    QString baseUrl_ = "http://127.0.0.1:8088";
    QString token_;
    bool loading_ = false;
    QString lastError_;
    QJsonObject currentUser_;
    QJsonArray users_;
    QJsonArray sessions_;
    QJsonArray streams_;
};
