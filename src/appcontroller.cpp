#include "appcontroller.hpp"

#include <hubsight/admin/admin_application_client.h>
#include <hubsight/admin/admin_types.h>
#include <hubsight/admin/config/hscfg_importer.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLocale>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleHints>
#include <QSysInfo>
#include <QTimer>

namespace hubsight {
namespace {

bool systemPrefersDarkMode() {
  const auto scheme = QGuiApplication::styleHints()->colorScheme();
  if (scheme == Qt::ColorScheme::Dark) {
    return true;
  }
  if (scheme == Qt::ColorScheme::Light) {
    return false;
  }
  return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}

QString normalizedThemeMode(const QString &themeMode) {
  if (themeMode == QStringLiteral("system") ||
      themeMode == QStringLiteral("light") ||
      themeMode == QStringLiteral("dark")) {
    return themeMode;
  }
  return {};
}

bool effectiveDarkMode(const QString &themeMode) {
  if (themeMode == QStringLiteral("dark")) {
    return true;
  }
  if (themeMode == QStringLiteral("light")) {
    return false;
  }
  return systemPrefersDarkMode();
}

} // namespace

AppController::AppController(HubSight::Admin::AdminApplicationClient *client,
                             QObject *parent)
    : QObject(parent), m_client(client) {
  initializePreferences();
  installLanguage(m_language);

  connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
          [this](Qt::ColorScheme) {
            if (m_themeMode != QStringLiteral("system")) {
              return;
            }
            const bool darkMode = effectiveDarkMode(m_themeMode);
            if (m_darkMode == darkMode) {
              return;
            }
            m_darkMode = darkMode;
            emit darkModeChanged();
          });

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

QString AppController::appVersion() const {
  return QCoreApplication::applicationVersion();
}

QString AppController::appVersionInfo() const {
  QString operatingSystem;
#if defined(Q_OS_MACOS)
  operatingSystem = QStringLiteral("macOS");
#elif defined(Q_OS_WINDOWS)
  operatingSystem = QStringLiteral("Windows");
#elif defined(Q_OS_LINUX)
  operatingSystem = QStringLiteral("Linux");
#elif defined(Q_OS_FREEBSD)
  operatingSystem = QStringLiteral("FreeBSD");
#else
  operatingSystem = QSysInfo::productType();
#endif

  return QStringLiteral("%1_%2.%3")
      .arg(appVersion(), operatingSystem, QSysInfo::currentCpuArchitecture());
}

QString AppController::fileMeta() const {
  if (m_configBytes.isEmpty()) {
    return {};
  }
  return tr("%1 • ready to decrypt").arg(formatSize(m_configBytes.size()));
}

void AppController::initializePreferences() {
  const QString storageDirectory =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  const QString storagePath =
      storageDirectory.isEmpty()
          ? QString()
          : QDir(storageDirectory).filePath(QStringLiteral("preferences.json"));

  if (!storageDirectory.isEmpty()) {
    if (!QDir().mkpath(storageDirectory)) {
      qWarning() << "Unable to create HubSight preference directory:"
                 << storageDirectory;
    }
    m_preferences.setStoragePath(storagePath);
    m_preferences.setAutoPersist(true);
  }

  const bool hasPreferenceFile =
      !storagePath.isEmpty() && QFileInfo::exists(storagePath);
  if (hasPreferenceFile) {
    QString errorMessage;
    if (!m_preferences.loadFile(storagePath, &errorMessage)) {
      qWarning() << "Unable to load HubSight preferences:" << errorMessage;
    }
  } else {
    const QString defaultLanguage =
        QLocale::system().language() == QLocale::Vietnamese
            ? QStringLiteral("vi")
            : QStringLiteral("en");

    // Migrate the two settings written by older builds once. PreferenceStore
    // remains the only store used after this initialization step.
    QSettings legacySettings;
    const bool hasLegacySettings =
        legacySettings.contains(QStringLiteral("ui/themeMode")) ||
        legacySettings.contains(QStringLiteral("ui/theme")) ||
        legacySettings.contains(QStringLiteral("ui/darkMode")) ||
        legacySettings.contains(QStringLiteral("ui/language"));
    QString themeMode;
    if (legacySettings.contains(QStringLiteral("ui/themeMode"))) {
      themeMode =
          legacySettings.value(QStringLiteral("ui/themeMode")).toString();
    } else if (legacySettings.contains(QStringLiteral("ui/theme"))) {
      themeMode = legacySettings.value(QStringLiteral("ui/theme")).toString();
    } else if (legacySettings.contains(QStringLiteral("ui/darkMode"))) {
      themeMode = legacySettings.value(QStringLiteral("ui/darkMode")).toBool()
                      ? QStringLiteral("dark")
                      : QStringLiteral("light");
    } else {
      themeMode = QStringLiteral("system");
    }
    if (normalizedThemeMode(themeMode).isEmpty()) {
      themeMode = QStringLiteral("system");
    }
    QString language =
        hasLegacySettings
            ? legacySettings
                  .value(QStringLiteral("ui/language"), defaultLanguage)
                  .toString()
            : defaultLanguage;
    if (language != QStringLiteral("vi") && language != QStringLiteral("en")) {
      language = defaultLanguage;
    }

    auto transaction = m_preferences.beginTransaction();
    transaction->set(QStringLiteral("ui.theme"), QJsonValue(themeMode));
    transaction->set(QStringLiteral("ui.language"), QJsonValue(language));
    transaction->set(QStringLiteral("meta.schemaVersion"), QJsonValue(2));
    const auto result = transaction->commit();
    if (result != HubSight::Preferences::PreferenceStore::TransactionResult::
                      Committed &&
        result != HubSight::Preferences::PreferenceStore::TransactionResult::
                      NoChanges) {
      qWarning() << "Unable to initialize HubSight preferences:"
                 << transaction->lastError();
    }
  }

  const QString systemLanguage =
      QLocale::system().language() == QLocale::Vietnamese
          ? QStringLiteral("vi")
          : QStringLiteral("en");
  const int preferenceSchemaVersion =
      m_preferences.get(QStringLiteral("meta.schemaVersion"), QJsonValue(1))
          .toInt(1);
  m_themeMode = normalizedThemeMode(
      m_preferences.get(QStringLiteral("ui.theme"), QJsonValue()).toString());
  if (m_themeMode.isEmpty()) {
    const QJsonValue legacyDarkMode =
        m_preferences.get(QStringLiteral("ui.darkMode"), QJsonValue());
    m_themeMode = legacyDarkMode.isBool()
                      ? (legacyDarkMode.toBool() ? QStringLiteral("dark")
                                                 : QStringLiteral("light"))
                      : QStringLiteral("system");
    if (!m_preferences.set(QStringLiteral("ui.theme"),
                           QJsonValue(m_themeMode))) {
      qWarning() << "Unable to migrate HubSight theme preference:"
                 << m_preferences.lastError();
    }
  }
  if (preferenceSchemaVersion < 2 &&
      !m_preferences.set(QStringLiteral("meta.schemaVersion"), QJsonValue(2))) {
    qWarning() << "Unable to update HubSight preference schema:"
               << m_preferences.lastError();
  }
  if (m_preferences.contains(QStringLiteral("ui.darkMode")) &&
      !m_preferences.remove(QStringLiteral("ui.darkMode"))) {
    qWarning() << "Unable to remove legacy HubSight theme preference:"
               << m_preferences.lastError();
  }
  m_darkMode = effectiveDarkMode(m_themeMode);
  m_language =
      m_preferences
          .get(QStringLiteral("ui.language"), QJsonValue(systemLanguage))
          .toString();
  if (m_language != QStringLiteral("vi") &&
      m_language != QStringLiteral("en")) {
    m_language = QStringLiteral("en");
  }
}

void AppController::setPin(const QString &pin) {
  if (m_pin == pin) {
    return;
  }
  m_pin = pin;
  clearError();
  emit pinChanged();
}

void AppController::setScreen(Screen screen) {
  if (m_screen == screen) {
    return;
  }
  m_screen = screen;
  emit screenChanged();
}

void AppController::setImportStep(ImportStep step) {
  if (m_importStep == step) {
    return;
  }
  m_importStep = step;
  clearError();
  emit importStepChanged();
}

void AppController::setThemeMode(const QString &themeMode) {
  const QString normalizedMode = normalizedThemeMode(themeMode);
  if (normalizedMode.isEmpty() || m_themeMode == normalizedMode) {
    return;
  }

  const bool previousDarkMode = m_darkMode;
  m_themeMode = normalizedMode;
  m_darkMode = effectiveDarkMode(m_themeMode);
  if (!m_preferences.set(QStringLiteral("ui.theme"), QJsonValue(m_themeMode))) {
    qWarning() << "Unable to persist HubSight theme preference:"
               << m_preferences.lastError();
  }
  emit themeModeChanged();
  if (previousDarkMode != m_darkMode) {
    emit darkModeChanged();
  }
}

void AppController::toggleTheme() {
  if (m_themeMode == QStringLiteral("system")) {
    setThemeMode(QStringLiteral("light"));
  } else if (m_themeMode == QStringLiteral("light")) {
    setThemeMode(QStringLiteral("dark"));
  } else {
    setThemeMode(QStringLiteral("system"));
  }
}

void AppController::installLanguage(const QString &language) {
  if (auto *application = QCoreApplication::instance()) {
    application->removeTranslator(&m_translator);
    if (language == QStringLiteral("vi") &&
        m_translator.load(QStringLiteral(":/i18n/hubsight_vi.qm"))) {
      application->installTranslator(&m_translator);
    }
  }
}

void AppController::toggleLanguage() {
  m_language = m_language == QStringLiteral("en") ? QStringLiteral("vi")
                                                  : QStringLiteral("en");
  installLanguage(m_language);
  if (!m_preferences.set(QStringLiteral("ui.language"),
                         QJsonValue(m_language))) {
    qWarning() << "Unable to persist HubSight language preference:"
               << m_preferences.lastError();
  }
  emit languageChanged();
  emit fileChanged();
  if (m_importStep == SummaryStep && !m_summaryRows.isEmpty()) {
    validateAndBuildSummary();
  }
}

void AppController::setError(const QString &message) {
  if (m_errorMessage == message) {
    return;
  }
  m_errorMessage = message;
  emit errorMessageChanged();
}

void AppController::clearError() { setError({}); }

void AppController::setBusy(bool busy, const QString &message) {
  if (m_busy == busy && m_busyMessage == message) {
    return;
  }
  m_busy = busy;
  m_busyMessage = busy ? message : QString();
  emit busyChanged();
}

void AppController::setAuthStatus(const QString &message, bool error) {
  if (m_authStatus == message && m_authError == error) {
    return;
  }
  m_authStatus = message;
  m_authError = error;
  emit authStateChanged();
}

void AppController::startSetup() {
  setScreen(ImportScreen);
  setImportStep(FileStep);
}

void AppController::goBack() {
  if (m_screen != ImportScreen) {
    return;
  }

  switch (m_importStep) {
  case WelcomeStep:
    break;
  case FileStep:
    setImportStep(WelcomeStep);
    break;
  case PinStep:
    setImportStep(FileStep);
    break;
  case SummaryStep:
    setImportStep(PinStep);
    break;
  }
}

void AppController::continueToPin() {
  if (m_screen == ImportScreen && !m_configBytes.isEmpty()) {
    setImportStep(PinStep);
  }
}

bool AppController::loadConfigFile(const QUrl &url) {
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
  m_config = {};
  m_integrity = HubSight::Admin::HscfgIntegrityState::NotChecked;
  clearError();
  emit fileChanged();
  return true;
}

void AppController::validatePin() {
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

void AppController::validateAndBuildSummary() {
  m_summaryRows = {
      QVariantMap{{QStringLiteral("label"), tr("Profile")},
                  {QStringLiteral("value"), m_config.metadata.profile.isEmpty()
                                                ? tr("HubSight Admin")
                                                : m_config.metadata.profile}},
      QVariantMap{{QStringLiteral("label"), tr("Config ID")},
                  {QStringLiteral("value"), m_config.identity.clientId}},
      QVariantMap{
          {QStringLiteral("label"), tr("Gateway")},
          {QStringLiteral("value"), m_config.urls.gatewayUrl.toString()}},
      QVariantMap{
          {QStringLiteral("label"), tr("API base URL")},
          {QStringLiteral("value"), m_config.urls.apiBaseUrl.toString()}},
      QVariantMap{{QStringLiteral("label"), tr("Relay WebSocket")},
                  {QStringLiteral("value"),
                   m_config.urls.relayWebSocketUrl.toString()}},
      QVariantMap{{QStringLiteral("label"), tr("Client")},
                  {QStringLiteral("value"), m_config.identity.clientName}},
      QVariantMap{{QStringLiteral("label"), tr("Integrity")},
                  {QStringLiteral("value"), formatIntegrity(m_integrity)}},
  };
  emit summaryChanged();
  setImportStep(SummaryStep);
}

void AppController::confirmImport() {
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

void AppController::signIn(const QString &username, const QString &password) {
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

void AppController::submitTwoFactor(const QString &code) {
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

void AppController::cancelTwoFactor() {
  m_twoFactorVisible = false;
  emit twoFactorChanged();
  m_signingIn = false;
  setAuthStatus(tr("Two-factor verification was cancelled."), true);
}

void AppController::handleTwoFactor() {
  m_signingIn = false;
  m_twoFactorVisible = true;
  emit twoFactorChanged();
  emit authStateChanged();
  setAuthStatus(tr("Two-factor verification is required."));
}

void AppController::handleAuthenticated(HubSight::Admin::AdminUser user) {
  m_signingIn = false;
  m_userDisplayName = user.fullName.isEmpty() ? user.username : user.fullName;
  setAuthStatus(tr("Signed in as %1.").arg(m_userDisplayName));
  emit authStateChanged();
  setScreen(WorkspaceScreen);
}

void AppController::handleError(HubSight::Admin::AdminError error) {
  const QString message = formatAdminError(error);
  if (m_screen == AuthScreen || m_signingIn || m_twoFactorVisible) {
    m_signingIn = false;
    setAuthStatus(message, true);
    emit authStateChanged();
    return;
  }
  setError(message);
}

QString AppController::formatAdminError(
    const HubSight::Admin::AdminError &error) const {
  const QString code = error.serverCode.trimmed();
  const auto withCode = [&code](const QString &message) {
    return code.isEmpty() ? message : message.arg(code);
  };

  switch (error.category) {
  case HubSight::Admin::ErrorCategory::Authentication:
    return withCode(code.isEmpty() ? tr("Sign-in failed.")
                                   : tr("Sign-in failed (%1)."));
  case HubSight::Admin::ErrorCategory::Authorization:
    return withCode(
        code.isEmpty()
            ? tr("You are not authorized to access this HubSight environment.")
            : tr("You are not authorized to access this HubSight environment "
                 "(%1)."));
  case HubSight::Admin::ErrorCategory::Network:
    return withCode(code.isEmpty()
                        ? tr("Unable to reach the HubSight service.")
                        : tr("Unable to reach the HubSight service (%1)."));
  case HubSight::Admin::ErrorCategory::Maintenance:
    return withCode(code.isEmpty()
                        ? tr("HubSight is temporarily unavailable.")
                        : tr("HubSight is temporarily unavailable (%1)."));
  case HubSight::Admin::ErrorCategory::Configuration:
    return withCode(code.isEmpty()
                        ? tr("HubSight configuration is invalid.")
                        : tr("HubSight configuration is invalid (%1)."));
  case HubSight::Admin::ErrorCategory::Validation:
  case HubSight::Admin::ErrorCategory::Conflict:
  case HubSight::Admin::ErrorCategory::Server:
  case HubSight::Admin::ErrorCategory::Parse:
  case HubSight::Admin::ErrorCategory::Canceled:
  case HubSight::Admin::ErrorCategory::Unknown:
    return withCode(code.isEmpty() ? tr("HubSight request failed.")
                                   : tr("HubSight request failed (%1)."));
  }
  return tr("HubSight request failed.");
}

QString AppController::formatImportError(
    const HubSight::Admin::HscfgImportResult &result) const {
  const QString code = HubSight::Admin::toString(result.error).toUpper();
  QString reason;
  switch (result.error) {
  case HubSight::Admin::HscfgImportError::ImporterUnavailable:
    reason = tr("The configuration importer is unavailable in this build.");
    break;
  case HubSight::Admin::HscfgImportError::InvalidMagic:
    reason = tr("This is not a valid HubSight Admin configuration file.");
    break;
  case HubSight::Admin::HscfgImportError::InvalidContainer:
    reason = tr("The configuration container is invalid or too large.");
    break;
  case HubSight::Admin::HscfgImportError::InvalidPin:
    reason = tr("The configuration PIN must contain exactly six digits.");
    break;
  case HubSight::Admin::HscfgImportError::DecryptionFailed:
    reason =
        tr("The configuration could not be decrypted. Check the PIN and file.");
    break;
  case HubSight::Admin::HscfgImportError::ZipInvalid:
    reason = tr("The encrypted configuration payload is invalid.");
    break;
  case HubSight::Admin::HscfgImportError::YamlInvalid:
    reason = tr("The configuration metadata is invalid.");
    break;
  case HubSight::Admin::HscfgImportError::UnexpectedPayload:
    reason = tr("The configuration is missing required data.");
    break;
  case HubSight::Admin::HscfgImportError::ValidationFailed:
    reason = tr("The configuration does not match the HubSight Admin profile "
                "requirements.");
    break;
  case HubSight::Admin::HscfgImportError::ContentHashMissing:
    reason = tr("The configuration content hash is missing.");
    break;
  case HubSight::Admin::HscfgImportError::ContentHashMismatch:
    reason = tr("The configuration content hash does not match.");
    break;
  case HubSight::Admin::HscfgImportError::SignatureInvalid:
    reason = tr("The configuration signature is invalid.");
    break;
  case HubSight::Admin::HscfgImportError::None:
    reason = tr("The configuration could not be imported.");
    break;
  }
  return tr("Configuration import failed (%1): %2").arg(code, reason);
}

QString AppController::formatSize(qsizetype size) const {
  return size < 1024 * 1024
             ? tr("%1 KB").arg(QString::number(size / 1024.0, 'f', 1))
             : tr("%1 MB").arg(
                   QString::number(size / (1024.0 * 1024.0), 'f', 1));
}

QString AppController::formatIntegrity(
    HubSight::Admin::HscfgIntegrityState state) const {
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
