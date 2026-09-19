#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>

#include <QGuiApplication>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
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

#include <functional>
#include <utility>

namespace {

QString configurationPath(const QString &value)
{
    const QUrl inputUrl = QUrl::fromUserInput(value);
    const QString path = inputUrl.isLocalFile() ? inputUrl.toLocalFile() : value;
    const QFileInfo fileInfo(path);
    if (fileInfo.suffix().compare(QStringLiteral("hscfg"), Qt::CaseInsensitive) != 0) {
        return {};
    }
    return fileInfo.absoluteFilePath();
}

QStringList configurationPaths(const QStringList &arguments)
{
    QStringList paths;
    for (const QString &argument : arguments) {
        const QString path = configurationPath(argument);
        if (!path.isEmpty()) {
            paths.append(path);
        }
    }
    return paths;
}

class HubSightApplication final : public QGuiApplication
{
public:
    using QGuiApplication::QGuiApplication;

    void setFileOpenHandler(std::function<void(const QString &)> handler)
    {
        m_fileOpenHandler = std::move(handler);
        if (!m_fileOpenHandler) {
            return;
        }
        const QStringList pendingPaths = std::exchange(m_pendingFilePaths, {});
        for (const QString &path : pendingPaths) {
            m_fileOpenHandler(path);
        }
    }

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::FileOpen) {
            const auto *fileEvent = static_cast<QFileOpenEvent *>(event);
            QString path = fileEvent->file();
            if (path.isEmpty() && fileEvent->url().isLocalFile()) {
                path = fileEvent->url().toLocalFile();
            }
            path = configurationPath(path);
            if (path.isEmpty()) {
                return true;
            }
            if (m_fileOpenHandler) {
                m_fileOpenHandler(path);
            } else {
                m_pendingFilePaths.append(path);
            }
            return true;
        }
        return QGuiApplication::event(event);
    }

private:
    std::function<void(const QString &)> m_fileOpenHandler;
    QStringList m_pendingFilePaths;
};

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

bool notifyRunningInstance(const QString &serverName, const QStringList &filePaths)
{
    QLocalSocket socket;
    socket.connectToServer(serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(250)) {
        return false;
    }

    QJsonObject request;
    request.insert(QStringLiteral("command"), QStringLiteral("activate"));
    if (!filePaths.isEmpty()) {
        request.insert(QStringLiteral("file"), filePaths.constFirst());
    }
    socket.write(QJsonDocument(request).toJson(QJsonDocument::Compact));
    socket.flush();
    socket.waitForBytesWritten(250);
    socket.disconnectFromServer();
    return true;
}

} // namespace

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArrayLiteral("Basic"));
    HubSightApplication application(argc, argv);

    QGuiApplication::setApplicationName(QStringLiteral("HubSight"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QGuiApplication::setOrganizationName(QStringLiteral("HubSight"));
    application.setWindowIcon(QIcon(QStringLiteral(":/icons/hubsight-512.png")));

    const QStringList filePaths = configurationPaths(application.arguments().mid(1));

    const QString lockPath = singleInstanceLockPath();
    const QFileInfo lockInfo(lockPath);
    if (!QDir().mkpath(lockInfo.absolutePath())) {
        qCritical() << "Unable to create HubSight single-instance directory:"
                    << lockInfo.absolutePath();
        return -1;
    }

    QLockFile instanceLock(lockPath);
    if (!instanceLock.tryLock(0)) {
        notifyRunningInstance(QStringLiteral("HubSightDesktopSingleInstance"), filePaths);
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
    QStringList pendingFilePaths;
    std::function<void(const QString &)> openConfigurationFile;
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
                             socket->waitForReadyRead(250);
                             const QJsonDocument request =
                                 QJsonDocument::fromJson(socket->readAll());
                             activateMainWindow();
                             const QString filePath = request.isObject()
                                                          ? configurationPath(
                                                                request.object()
                                                                    .value(QStringLiteral("file"))
                                                                    .toString())
                                                          : QString();
                             if (!filePath.isEmpty()) {
                                 if (openConfigurationFile) {
                                     openConfigurationFile(filePath);
                                 } else {
                                     pendingFilePaths.append(filePath);
                                 }
                             }
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

    openConfigurationFile = [&controller](const QString &path) {
        controller.startSetup();
        controller.loadConfigFile(QUrl::fromLocalFile(path));
    };
    application.setFileOpenHandler(openConfigurationFile);
    for (const QString &path : filePaths) {
        openConfigurationFile(path);
    }
    const QStringList queuedFilePaths = std::exchange(pendingFilePaths, {});
    for (const QString &path : queuedFilePaths) {
        openConfigurationFile(path);
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
