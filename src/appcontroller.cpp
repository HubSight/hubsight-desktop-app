#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>
#include <hubsight/admin/admin_types.h>
#include <hubsight/admin/config/hscfg_importer.h>

#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QLocale>
#include <QPalette>
#include <QSettings>
#include <QStyleHints>
#include <QTimer>

namespace hubsight {
namespace {

bool systemPrefersDarkMode()
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

} // namespace

AppController::AppController(HubSight::Admin::AdminApplicationClient *client,
                             QObject *parent)
    : QObject(parent),
      m_client(client)
{
    QSettings settings;
    settings.sync();
    m_darkMode = settings.value(QStringLiteral("ui/darkMode"),
                                systemPrefersDarkMode()).toBool();
    const QString defaultLanguage = QLocale::system().language() == QLocale::Vietnamese
                                        ? QStringLiteral("vi")
                                        : QStringLiteral("en");
    m_language = settings.value(QStringLiteral("ui/language"),
                                defaultLanguage).toString();
    if (m_language != QStringLiteral("vi") && m_language != QStringLiteral("en")) {
        m_language = QStringLiteral("en");
    }

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

QString AppController::localized(const char *english, const char *vietnamese) const
{
    return QString::fromUtf8(m_language == QStringLiteral("vi") ? vietnamese : english);
}

QString AppController::stepTitle() const
{
    switch (m_importStep) {
    case WelcomeStep: return localized("Configuration setup", "Thiết lập cấu hình");
    case FileStep: return localized("Local file import", "Nhập tệp cấu hình");
    case PinStep: return localized("Decrypt configuration", "Giải mã cấu hình");
    case SummaryStep: return localized("Review and activate", "Kiểm tra và kích hoạt");
    }
    return localized("Configuration setup", "Thiết lập cấu hình");
}

QString AppController::stepSubtitle() const
{
    return localized("HUBSIGHT SECURITY ENCLAVE", "VÙNG BẢO MẬT HUBSIGHT");
}

QString AppController::appVersion() const
{
    return QCoreApplication::applicationVersion();
}

QString AppController::fileMeta() const
{
    if (m_configBytes.isEmpty()) {
        return {};
    }
    return localized("%1 • ready to decrypt", "%1 • sẵn sàng giải mã")
        .arg(formatSize(m_configBytes.size()));
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
    emit uiTextChanged();
}

void AppController::toggleTheme()
{
    m_darkMode = !m_darkMode;
    QSettings settings;
    settings.setValue(QStringLiteral("ui/darkMode"), m_darkMode);
    settings.sync();
    emit darkModeChanged();
}

void AppController::toggleLanguage()
{
    m_language = m_language == QStringLiteral("en") ? QStringLiteral("vi")
                                                     : QStringLiteral("en");
    QSettings settings;
    settings.setValue(QStringLiteral("ui/language"), m_language);
    settings.sync();
    emit languageChanged();
    emit uiTextChanged();
    emit fileChanged();
    if (m_importStep == SummaryStep && !m_summaryRows.isEmpty()) {
        validateAndBuildSummary();
    }
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
        setError(localized("No configuration file was selected.",
                           "Chưa chọn tệp cấu hình."));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(localized("Unable to read the selected configuration file.",
                           "Không thể đọc tệp cấu hình đã chọn."));
        return false;
    }
    if (file.size() > 64 * 1024 * 1024) {
        setError(localized("The configuration file is larger than the 64 MB limit.",
                           "Tệp cấu hình vượt quá giới hạn 64 MB."));
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        setError(localized("The selected configuration is empty.",
                           "Tệp cấu hình đã chọn đang trống."));
        return false;
    }

    m_configBytes = bytes;
    m_fileName = QFileInfo(path).fileName();
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
        setError(localized("No configuration file has been selected.",
                           "Chưa chọn tệp cấu hình."));
        return;
    }
    if (m_pin.size() != 6) {
        setError(localized("Enter exactly 6 digits.",
                           "Hãy nhập chính xác 6 chữ số."));
        return;
    }

    const QString pin = m_pin;
    setBusy(true, localized("Decrypting and validating configuration…",
                            "Đang giải mã và xác thực cấu hình…"));
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
        QVariantMap{{QStringLiteral("label"), localized("Profile", "Hồ sơ")},
                    {QStringLiteral("value"), m_config.metadata.profile.isEmpty()
                                                    ? QStringLiteral("HubSight Admin")
                                                    : m_config.metadata.profile}},
        QVariantMap{{QStringLiteral("label"), localized("Config ID", "ID cấu hình")},
                    {QStringLiteral("value"), m_config.identity.clientId}},
        QVariantMap{{QStringLiteral("label"), localized("Gateway", "Cổng kết nối")},
                    {QStringLiteral("value"), m_config.urls.gatewayUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), localized("API base URL", "URL API gốc")},
                    {QStringLiteral("value"), m_config.urls.apiBaseUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), localized("Relay WebSocket", "WebSocket chuyển tiếp")},
                    {QStringLiteral("value"), m_config.urls.relayWebSocketUrl.toString()}},
        QVariantMap{{QStringLiteral("label"), localized("Client", "Ứng dụng khách")},
                    {QStringLiteral("value"), m_config.identity.clientName}},
        QVariantMap{{QStringLiteral("label"), localized("Integrity", "Tính toàn vẹn")},
                    {QStringLiteral("value"), formatIntegrity(m_integrity)}},
    };
    emit summaryChanged();
    setImportStep(SummaryStep);
}

void AppController::confirmImport()
{
    if (!m_client || m_configBytes.isEmpty()) {
        setError(localized("The HubSight SDK is not available.",
                           "HubSight SDK không khả dụng."));
        return;
    }

    clearError();
    setBusy(true, localized("Activating configuration…", "Đang kích hoạt cấu hình…"));
    const bool success = m_client->importHscfg(m_configBytes, m_pin);
    setBusy(false);
    if (!success) {
        setError(localized(
            "The HubSight SDK could not activate this configuration. Please verify the file and PIN.",
            "HubSight SDK không thể kích hoạt cấu hình này. Hãy kiểm tra lại tệp và mã PIN."));
        return;
    }

    m_configBytes.clear();
    m_pin.clear();
    emit fileChanged();
    emit pinChanged();
    setAuthStatus(localized("Configuration imported. Sign in to continue.",
                            "Đã nhập cấu hình. Hãy đăng nhập để tiếp tục."));
    setScreen(AuthScreen);
}

void AppController::signIn(const QString &username, const QString &password)
{
    if (!m_client) {
        setAuthStatus(localized("The HubSight SDK is not available.",
                                "HubSight SDK không khả dụng."), true);
        return;
    }
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        setAuthStatus(localized("Enter both username and password.",
                                "Hãy nhập tên đăng nhập và mật khẩu."), true);
        return;
    }

    m_signingIn = true;
    setAuthStatus(localized("Signing in…", "Đang đăng nhập…"));
    emit authStateChanged();
    m_client->signIn(username.trimmed(), password);
}

void AppController::submitTwoFactor(const QString &code)
{
    if (!m_client || code.trimmed().isEmpty()) {
        setAuthStatus(localized("Enter your two-factor code.",
                                "Hãy nhập mã xác thực hai bước."), true);
        return;
    }

    m_twoFactorVisible = false;
    emit twoFactorChanged();
    m_signingIn = true;
    setAuthStatus(localized("Verifying two-factor code…",
                            "Đang xác minh mã hai bước…"));
    emit authStateChanged();
    m_client->verifyTwoFactor(code.trimmed());
}

void AppController::cancelTwoFactor()
{
    m_twoFactorVisible = false;
    emit twoFactorChanged();
    m_signingIn = false;
    setAuthStatus(localized("Two-factor verification was cancelled.",
                            "Đã hủy xác thực hai bước."), true);
}

void AppController::handleTwoFactor()
{
    m_signingIn = false;
    m_twoFactorVisible = true;
    emit twoFactorChanged();
    emit authStateChanged();
    setAuthStatus(localized("Two-factor verification is required.",
                            "Yêu cầu xác thực hai bước."));
}

void AppController::handleAuthenticated(HubSight::Admin::AdminUser user)
{
    m_signingIn = false;
    m_userDisplayName = user.fullName.isEmpty() ? user.username : user.fullName;
    setAuthStatus(localized("Signed in as %1.", "Đã đăng nhập với tên %1.")
                      .arg(m_userDisplayName));
    emit authStateChanged();
    setScreen(WorkspaceScreen);
}

void AppController::handleError(HubSight::Admin::AdminError error)
{
    const QString message = error.developerMessage.isEmpty()
                                ? localized("Sign-in failed (%1).",
                                            "Đăng nhập thất bại (%1).")
                                      .arg(error.serverCode)
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
        return localized("Configuration import failed (%1).",
                         "Nhập cấu hình thất bại (%1).")
            .arg(code);
    }
    return localized("Configuration import failed (%1): %2",
                     "Nhập cấu hình thất bại (%1): %2")
        .arg(code, result.message);
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
        return localized("Fully verified", "Đã xác minh đầy đủ");
    case HubSight::Admin::HscfgIntegrityState::ContentHashVerified:
        return localized("Content hash verified", "Đã xác minh mã băm nội dung");
    case HubSight::Admin::HscfgIntegrityState::SignatureUnavailable:
        return localized("Content hash verified; signature unavailable",
                         "Đã xác minh mã băm; không có chữ ký");
    case HubSight::Admin::HscfgIntegrityState::NotChecked:
        return localized("Not checked", "Chưa kiểm tra");
    }
    return localized("Unknown", "Không xác định");
}

} // namespace hubsight
