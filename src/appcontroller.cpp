#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>
#include <hubsight/admin/admin_types.h>
#include <hubsight/admin/config/hscfg_importer.h>

#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QTimer>

namespace hubsight {

AppController::AppController(HubSight::Admin::AdminApplicationClient *client,
                             QObject *parent)
    : QObject(parent),
      m_client(client)
{
    if (!m_client) {
        return;
    }

    connect(m_client, &HubSight::Admin::AdminApplicationClient::twoFactorRequired,
            this, &AppController::handleTwoFactor);
    connect(m_client, &HubSight::Admin::AdminApplicationClient::authenticated,
            this, &AppController::handleAuthenticated);
    connect(m_client, &HubSight::Admin::AdminApplicationClient::errorOccurred,
            this, &AppController::handleError);
}

bool AppController::darkMode() const
{
    const auto scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme == Qt::ColorScheme::Dark) {
        return true;
    }
    if (scheme == Qt::ColorScheme::Light) {
        return false;
    }
    return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}

QString AppController::stepTitle() const
{
    switch (m_importStep) {
    case WelcomeStep: return tr("Configuration setup");
    case FileStep: return tr("Local file import");
    case PinStep: return tr("Decrypt configuration");
    case SummaryStep: return tr("Review and activate");
    }
    return tr("Configuration setup");
}

void AppController::setPin(const QString &pin)
{
    if (m_pin == pin) {
        return;
    }
    m_pin = pin;
    clearError();
    emit pinChanged();
}

void AppController::setScreen(Screen screen)
{
    if (m_screen == screen) {
        return;
    }
    m_screen = screen;
    emit screenChanged();
}

void AppController::setImportStep(ImportStep step)
{
    if (m_importStep == step) {
        return;
    }
    m_importStep = step;
    clearError();
    emit importStepChanged();
}

void AppController::setError(const QString &message)
{
    if (m_errorMessage == message) {
        return;
    }
    m_errorMessage = message;
    emit errorMessageChanged();
}

void AppController::clearError()
{
    setError({});
}

void AppController::setBusy(bool busy, const QString &message)
{
    if (m_busy == busy && m_busyMessage == message) {
        return;
    }
    m_busy = busy;
    m_busyMessage = busy ? message : QString();
    emit busyChanged();
}

void AppController::setAuthStatus(const QString &message, bool error)
{
    if (m_authStatus == message && m_authError == error) {
        return;
    }
    m_authStatus = message;
    m_authError = error;
    emit authStateChanged();
}

void AppController::startSetup()
{
    setScreen(ImportScreen);
    setImportStep(FileStep);
}

void AppController::goBack()
{
    if (m_screen != ImportScreen) {
        return;
    }

    switch (m_importStep) {
    case WelcomeStep: break;
    case FileStep: setImportStep(WelcomeStep); break;
    case PinStep: setImportStep(FileStep); break;
    case SummaryStep: setImportStep(PinStep); break;
    }
}

void AppController::continueToPin()
{
    if (m_screen == ImportScreen && !m_configBytes.isEmpty()) {
        setImportStep(PinStep);
    }
}

bool AppController::loadConfigFile(const QUrl &url)
{
    QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    if (path.startsWith(QStringLiteral("file://"))) {
        path = QUrl(path).toLocalFile();
    }
    if (path.isEmpty()) {
        setError(tr("No configuration file was selected."));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(tr("Unable to read the selected configuration file."));
        return false;
    }
    if (file.size() > 64 * 1024 * 1024) {
        setError(tr("The configuration file is larger than the 64 MB limit."));
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        setError(tr("The selected configuration is empty."));
        return false;
    }

    m_configBytes = bytes;
    m_fileName = QFileInfo(path).fileName();
    m_fileMeta = tr("%1 • ready to decrypt").arg(formatSize(bytes.size()));
    m_config = {};
    m_integrity = HubSight::Admin::HscfgIntegrityState::NotChecked;
    clearError();
    emit fileChanged();
    return true;
}

void AppController::validatePin()
{
    clearError();
    if (m_configBytes.isEmpty()) {
        setError(tr("No configuration file has been selected."));
        return;
    }
    if (m_pin.size() != 6) {
        setError(tr("Enter exactly 6 digits."));
        return;
    }

    const QString pin = m_pin;
    setBusy(true, tr("Decrypting and validating configuration…"));
    QTimer::singleShot(0, this, [this, pin] {
        HubSight::Admin::HscfgImporter importer;
        const auto result = importer.importAdmin(m_configBytes, pin);
        setBusy(false);
        if (!result.success()) {
            setError(formatImportError(result));
            return;
        }
        m_config = result.config;
        m_integrity = result.integrity;
        validateAndBuildSummary();
    });
}

void AppController::validateAndBuildSummary()
{
    m_summaryRows = {
        QVariantMap{{QStringLiteral("label"), tr("Profile")},
                    {QStringLiteral("value"), m_config.metadata.profile.isEmpty()
                                                    ? QStringLiteral("HubSight Admin")
                                                    : m_config.metadata.profile}},
        QVariantMap{{QStringLiteral("label"), tr("Config ID")},
                    {QStringLiteral("value"), m_config.identity.clientId}},
        QVariantMap{{QStringLiteral("label"), tr("Gateway")},
                    {QStringLiteral("value"), m_config.urls.gatewayUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), tr("API base URL")},
                    {QStringLiteral("value"), m_config.urls.apiBaseUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), tr("Relay WebSocket")},
                    {QStringLiteral("value"), m_config.urls.relayWebSocketUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), tr("Client")},
                    {QStringLiteral("value"), m_config.identity.clientName}},
        QVariantMap{{QStringLiteral("label"), tr("Integrity")},
                    {QStringLiteral("value"), formatIntegrity(m_integrity)}},
    };
    emit summaryChanged();
    setImportStep(SummaryStep);
}

void AppController::confirmImport()
{
    if (!m_client || m_configBytes.isEmpty()) {
        setError(tr("The HubSight SDK is not available."));
        return;
    }

    clearError();
    setBusy(true, tr("Activating configuration…"));
    const bool success = m_client->importHscfg(m_configBytes, m_pin);
    setBusy(false);
    if (!success) {
        setError(tr("The HubSight SDK could not activate this configuration. "
                    "Please verify the file and PIN."));
        return;
    }

    m_configBytes.clear();
    m_pin.clear();
    emit fileChanged();
    emit pinChanged();
    setAuthStatus(tr("Configuration imported. Sign in to continue."));
    setScreen(AuthScreen);
}

void AppController::signIn(const QString &username, const QString &password)
{
    if (!m_client) {
        setAuthStatus(tr("The HubSight SDK is not available."), true);
        return;
    }
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        setAuthStatus(tr("Enter both username and password."), true);
        return;
    }

    m_signingIn = true;
    setAuthStatus(tr("Signing in…"));
    emit authStateChanged();
    m_client->signIn(username.trimmed(), password);
}

void AppController::submitTwoFactor(const QString &code)
{
    if (!m_client || code.trimmed().isEmpty()) {
        setAuthStatus(tr("Enter your two-factor code."), true);
        return;
    }

    m_twoFactorVisible = false;
    emit twoFactorChanged();
    m_signingIn = true;
    setAuthStatus(tr("Verifying two-factor code…"));
    emit authStateChanged();
    m_client->verifyTwoFactor(code.trimmed());
}

void AppController::cancelTwoFactor()
{
    m_twoFactorVisible = false;
    emit twoFactorChanged();
    m_signingIn = false;
    setAuthStatus(tr("Two-factor verification was cancelled."), true);
}

void AppController::handleTwoFactor()
{
    m_signingIn = false;
    m_twoFactorVisible = true;
    emit twoFactorChanged();
    emit authStateChanged();
    setAuthStatus(tr("Two-factor verification is required."));
}

void AppController::handleAuthenticated(HubSight::Admin::AdminUser user)
{
    m_signingIn = false;
    m_userDisplayName = user.fullName.isEmpty() ? user.username : user.fullName;
    setAuthStatus(tr("Signed in as %1.").arg(m_userDisplayName));
    emit authStateChanged();
    setScreen(WorkspaceScreen);
}

void AppController::handleError(HubSight::Admin::AdminError error)
{
    const QString message = error.developerMessage.isEmpty()
                                ? tr("Sign-in failed (%1).").arg(error.serverCode)
                                : error.developerMessage;
    if (m_screen == AuthScreen || m_signingIn || m_twoFactorVisible) {
        m_signingIn = false;
        setAuthStatus(message, true);
        emit authStateChanged();
        return;
    }
    setError(message);
}

QString AppController::formatImportError(
    const HubSight::Admin::HscfgImportResult &result) const
{
    const QString code = HubSight::Admin::toString(result.error).toUpper();
    if (result.message.isEmpty()) {
        return tr("Configuration import failed (%1).").arg(code);
    }
    return tr("Configuration import failed (%1): %2").arg(code, result.message);
}

QString AppController::formatSize(qsizetype size) const
{
    return size < 1024 * 1024
               ? tr("%1 KB").arg(QString::number(size / 1024.0, 'f', 1))
               : tr("%1 MB").arg(QString::number(size / (1024.0 * 1024.0), 'f', 1));
}

QString AppController::formatIntegrity(
    HubSight::Admin::HscfgIntegrityState state) const
{
    switch (state) {
    case HubSight::Admin::HscfgIntegrityState::FullyVerified:
        return tr("Fully verified");
    case HubSight::Admin::HscfgIntegrityState::ContentHashVerified:
        return tr("Content hash verified");
    case HubSight::Admin::HscfgIntegrityState::SignatureUnavailable:
        return tr("Content hash verified; signature unavailable");
    case HubSight::Admin::HscfgIntegrityState::NotChecked:
        return tr("Not checked");
    }
    return tr("Unknown");
}

} // namespace hubsight
