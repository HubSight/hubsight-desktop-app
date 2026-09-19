#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>

#include <QGuiApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QPointer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>
#include <QWindow>

namespace {

QString singleInstanceLockPath()
{
    const QString appDataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (!appDataDirectory.isEmpty()) {
        return QDir(appDataDirectory).filePath(QStringLiteral("instance.lock"));
    }
    QString tempDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDirectory.isEmpty()) {
        tempDirectory = QDir::tempPath();
    }
    return QDir(tempDirectory)
        .filePath(QStringLiteral("hubsight-desktop-instance.lock"));
}

bool notifyRunningInstance(const QString &serverName)
{
    QLocalSocket socket;
    socket.connectToServer(serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(250)) {
        return false;
    }

    socket.write("activate", 8);
    socket.flush();
    socket.waitForBytesWritten(250);
    socket.disconnectFromServer();
    return true;
}

} // namespace

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Basic"));
    QGuiApplication application(argc, argv);

    QGuiApplication::setApplicationName(QStringLiteral("HubSight"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QGuiApplication::setOrganizationName(QStringLiteral("HubSight"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/hubsight-512.png")));

    const QString lockPath = singleInstanceLockPath();
    const QFileInfo lockInfo(lockPath);
    if (!QDir().mkpath(lockInfo.absolutePath())) {
        qCritical() << "Unable to create HubSight single-instance directory:"
                    << lockInfo.absolutePath();
        return -1;
    }

    QLockFile instanceLock(lockPath);
    if (!instanceLock.tryLock(0)) {
        notifyRunningInstance(QStringLiteral("HubSightDesktopSingleInstance"));
        return 0;
    }

    const QString serverName = QStringLiteral("HubSightDesktopSingleInstance");
    QLocalServer::removeServer(serverName);
    QLocalServer instanceServer;
    if (!instanceServer.listen(serverName)) {
        qWarning() << "Unable to start HubSight single-instance server:"
                   << instanceServer.errorString();
    }

    QPointer<QWindow> mainWindow;
    QPointer<QWindow> splashWindow;
    bool startupInProgress = true;
    bool activationPending = false;
    const auto activateMainWindow = [&]() {
        if (!mainWindow || startupInProgress) {
            activationPending = true;
            return;
        }
        if (splashWindow && splashWindow->isVisible()) {
            activationPending = true;
            return;
        }
        mainWindow->showNormal();
        mainWindow->raise();
        mainWindow->requestActivate();
    };
    QObject::connect(&instanceServer, &QLocalServer::newConnection,
                     &application, [&]() {
                         while (instanceServer.hasPendingConnections()) {
                             QLocalSocket *socket = instanceServer.nextPendingConnection();
                             activateMainWindow();
                             socket->disconnectFromServer();
                             socket->deleteLater();
                         }
                     });

    HubSight::Admin::AdminApplicationClient client;
    hubsight::AppController controller(&client);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    QObject::connect(&controller, &hubsight::AppController::languageChanged,
                     &engine, &QQmlEngine::retranslate);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    mainWindow = qobject_cast<QWindow *>(engine.rootObjects().constFirst());
    if (!mainWindow) {
        return -1;
    }
    const int splashRootIndex = engine.rootObjects().size();
    engine.load(QUrl(QStringLiteral("qrc:/qml/Splash.qml")));
    if (engine.rootObjects().size() <= splashRootIndex) {
        startupInProgress = false;
        mainWindow->show();
        if (activationPending) {
            activateMainWindow();
        }
        return application.exec();
    }

    splashWindow = qobject_cast<QWindow *>(engine.rootObjects().at(splashRootIndex));
    application.processEvents();

    QTimer::singleShot(550, &application, [&, mainWindow, splashWindow] {
        startupInProgress = false;
        mainWindow->show();
        splashWindow->hide();
        if (activationPending) {
            activateMainWindow();
        }
    });

    return application.exec();
}
