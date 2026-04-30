#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ApiClient.h"
#include "EventSocket.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("SkyMeet Client");
    app.setOrganizationName("SkyMeet");

    qmlRegisterType<ApiClient>("SkyMeet.Backend", 1, 0, "ApiClient");
    qmlRegisterType<EventSocket>("SkyMeet.Backend", 1, 0, "EventSocket");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
