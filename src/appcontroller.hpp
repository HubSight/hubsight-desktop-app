#pragma once

#include <QByteArray>
#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <hubsight/admin/config/hscfg_types.h>

namespace HubSight::Admin {
class AdminApplicationClient;
struct AdminError;
struct AdminUser;
}

namespace hubsight {

class AppController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)
    Q_PROPERTY(int importStep READ importStep NOTIFY importStepChanged)
    Q_PROPERTY(QString stepTitle READ stepTitle NOTIFY uiTextChanged)
    Q_PROPERTY(QString stepSubtitle READ stepSubtitle NOTIFY uiTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileChanged)
    Q_PROPERTY(QString fileMeta READ fileMeta NOTIFY fileChanged)
    Q_PROPERTY(bool fileReady READ fileReady NOTIFY fileChanged)
    Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY pinChanged)
    Q_PROPERTY(QVariantList summaryRows READ summaryRows NOTIFY summaryChanged)
    Q_PROPERTY(QString authStatus READ authStatus NOTIFY authStateChanged)
    Q_PROPERTY(bool authError READ authError NOTIFY authStateChanged)
    Q_PROPERTY(bool signingIn READ signingIn NOTIFY authStateChanged)
    Q_PROPERTY(bool twoFactorVisible READ twoFactorVisible NOTIFY twoFactorChanged)
    Q_PROPERTY(QString userDisplayName READ userDisplayName NOTIFY authStateChanged)
    Q_PROPERTY(bool darkMode READ darkMode NOTIFY darkModeChanged)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    enum Screen {
        ImportScreen = 0,
        AuthScreen = 1,
        WorkspaceScreen = 2,
    };
    Q_ENUM(Screen)

    enum ImportStep {
        WelcomeStep = 0,
        FileStep = 1,
        PinStep = 2,
        SummaryStep = 3,
    };
    Q_ENUM(ImportStep)

    explicit AppController(HubSight::Admin::AdminApplicationClient *client,
                           QObject *parent = nullptr);

    int screen() const { return m_screen; }
    int importStep() const { return m_importStep; }
    QString stepTitle() const;
    QString stepSubtitle() const;
    bool busy() const { return m_busy; }
    QString busyMessage() const { return m_busyMessage; }
    QString errorMessage() const { return m_errorMessage; }
    QString fileName() const { return m_fileName; }
    QString fileMeta() const;
    bool fileReady() const { return !m_configBytes.isEmpty(); }
    QString pin() const { return m_pin; }
    QVariantList summaryRows() const { return m_summaryRows; }
    QString authStatus() const { return m_authStatus; }
    bool authError() const { return m_authError; }
    bool signingIn() const { return m_signingIn; }
    bool twoFactorVisible() const { return m_twoFactorVisible; }
    QString userDisplayName() const { return m_userDisplayName; }
    bool darkMode() const { return m_darkMode; }
    QString language() const { return m_language; }
    QString appVersion() const;

    void setPin(const QString &pin);

    Q_INVOKABLE void startSetup();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void continueToPin();
    Q_INVOKABLE bool loadConfigFile(const QUrl &url);
    Q_INVOKABLE void validatePin();
    Q_INVOKABLE void confirmImport();
    Q_INVOKABLE void signIn(const QString &username, const QString &password);
    Q_INVOKABLE void submitTwoFactor(const QString &code);
    Q_INVOKABLE void cancelTwoFactor();
    Q_INVOKABLE void toggleTheme();
    Q_INVOKABLE void toggleLanguage();

signals:
    void screenChanged();
    void importStepChanged();
    void busyChanged();
    void errorMessageChanged();
    void fileChanged();
    void pinChanged();
    void summaryChanged();
    void authStateChanged();
    void twoFactorChanged();
    void darkModeChanged();
    void languageChanged();
    void uiTextChanged();

private slots:
    void handleTwoFactor();
    void handleAuthenticated(HubSight::Admin::AdminUser user);
    void handleError(HubSight::Admin::AdminError error);

private:
    void setScreen(Screen screen);
    void setImportStep(ImportStep step);
    void setError(const QString &message);
    void clearError();
    void setBusy(bool busy, const QString &message = {});
    void setAuthStatus(const QString &message, bool error = false);
    void validateAndBuildSummary();
    QString formatImportError(const HubSight::Admin::HscfgImportResult &result) const;
    QString formatSize(qsizetype size) const;
    QString formatIntegrity(HubSight::Admin::HscfgIntegrityState state) const;
    QString localized(const char *english, const char *vietnamese) const;

    HubSight::Admin::AdminApplicationClient *m_client = nullptr;
    Screen m_screen = ImportScreen;
    ImportStep m_importStep = WelcomeStep;
    bool m_busy = false;
    QString m_busyMessage;
    QString m_errorMessage;
    QByteArray m_configBytes;
    QString m_fileName;
    QString m_pin;
    HubSight::Admin::HscfgConfig m_config;
    HubSight::Admin::HscfgIntegrityState m_integrity =
        HubSight::Admin::HscfgIntegrityState::NotChecked;
    QVariantList m_summaryRows;
    QString m_authStatus;
    bool m_authError = false;
    bool m_signingIn = false;
    bool m_twoFactorVisible = false;
    QString m_userDisplayName;
    bool m_darkMode = false;
    QString m_language = QStringLiteral("en");
};

} // namespace hubsight
