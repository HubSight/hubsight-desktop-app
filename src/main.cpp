#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Basic"));
    QGuiApplication application(argc, argv);

    QGuiApplication::setApplicationName(QStringLiteral("HubSight"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QGuiApplication::setOrganizationName(QStringLiteral("HubSight"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/hubsight-512.png")));

    HubSight::Admin::AdminApplicationClient client;
    hubsight::AppController controller(&client);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    QObject *mainWindow = engine.rootObjects().constFirst();
    const int splashRootIndex = engine.rootObjects().size();
    engine.load(QUrl(QStringLiteral("qrc:/qml/Splash.qml")));
    if (engine.rootObjects().size() <= splashRootIndex) {
        mainWindow->setProperty("visible", true);
        return application.exec();
    }

    QObject *splashWindow = engine.rootObjects().at(splashRootIndex);
    application.processEvents();

    QTimer::singleShot(550, &application, [mainWindow, splashWindow] {
        mainWindow->setProperty("visible", true);
        splashWindow->setProperty("visible", false);
    });

    return application.exec();
}
