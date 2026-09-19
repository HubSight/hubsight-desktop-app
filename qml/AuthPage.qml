import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"

Item {
    id: root

    property var controller
    property QtObject theme

    Rectangle {
        anchors.fill: parent
        color: root.theme.background
    }

    ColumnLayout {
        width: Math.min(parent.width - 80, 520)
        anchors.centerIn: parent
        spacing: 14

        Label {
            text: qsTr("HUBSIGHT  /  ADMIN ACCESS")
            color: root.theme.accentText
            font.family: "Roboto"
            font.pixelSize: 11
            font.weight: Font.DemiBold
        }
        Label {
            text: qsTr("Sign in to HubSight")
            color: root.theme.text
            font.family: "Roboto"
            font.pixelSize: 24
            font.weight: Font.DemiBold
        }
        Label {
            text: qsTr("Your server configuration is ready. Sign in to continue to the desktop workspace.")
            color: root.theme.textMuted
            font.family: "Roboto"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Item { Layout.preferredHeight: 8 }

        AppCard {
            theme: root.theme
            Layout.fillWidth: true
            Layout.preferredHeight: 250
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 24
                spacing: 12

                Label {
                    text: qsTr("Username")
                    color: root.theme.textSecondary
                    font.pixelSize: 13
                }
                AppTextField {
                    id: usernameField
                    theme: root.theme
                    placeholderText: qsTr("Username or email")
                    Layout.fillWidth: true
                    onAccepted: passwordField.forceActiveFocus()
                }
                Label {
                    text: qsTr("Password")
                    color: root.theme.textSecondary
                    font.pixelSize: 13
                }
                AppTextField {
                    id: passwordField
                    theme: root.theme
                    placeholderText: qsTr("Password")
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    onAccepted: root.controller.signIn(usernameField.text, passwordField.text)
                }
                Label {
                    visible: root.controller.authStatus.length > 0
                    text: root.controller.authStatus
                    color: root.controller.authError ? root.theme.errorText : root.theme.accentText
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    font.pixelSize: 13
                }
                AppButton {
                    id: signInButton
                    text: root.controller.signingIn ? qsTr("Signing in…") : qsTr("Sign in")
                    theme: root.theme
                    enabled: !root.controller.signingIn
                    Layout.fillWidth: true
                    onClicked: root.controller.signIn(usernameField.text, passwordField.text)
                }
            }
        }
    }

    Popup {
        id: twoFactorPopup
        visible: root.controller.twoFactorVisible
        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        width: 380
        height: 250
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: root.theme.surface
            radius: 8
            border.width: 1
            border.color: root.theme.border
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12
            Label {
                text: qsTr("Two-factor authentication")
                color: root.theme.text
                font.pixelSize: 18
                font.weight: Font.DemiBold
            }
            Label {
                text: qsTr("Enter the verification code from your authenticator.")
                color: root.theme.textMuted
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            AppTextField {
                id: twoFactorField
                theme: root.theme
                placeholderText: qsTr("6-digit code")
                maximumLength: 6
                inputMethodHints: Qt.ImhDigitsOnly
                Layout.fillWidth: true
                onAccepted: root.controller.submitTwoFactor(text)
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                AppButton {
                    text: qsTr("Cancel")
                    theme: root.theme
                    primary: false
                    onClicked: root.controller.cancelTwoFactor()
                }
                AppButton {
                    text: qsTr("Verify")
                    theme: root.theme
                    enabled: twoFactorField.text.length > 0 && !root.controller.signingIn
                    onClicked: root.controller.submitTwoFactor(twoFactorField.text)
                }
            }
        }
    }

    Connections {
        target: root.controller
        function onTwoFactorChanged() {
            if (!root.controller.twoFactorVisible)
                twoFactorField.clear()
        }
    }
}
