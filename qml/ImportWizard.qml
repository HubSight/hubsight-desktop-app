import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
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
        anchors.fill: parent
        anchors.margins: 30
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            AppButton {
                visible: root.controller.importStep > 0
                text: "‹"
                theme: root.theme
                primary: false
                compact: true
                Layout.preferredWidth: 40
                onClicked: root.controller.goBack()
            }

            Image {
                source: "qrc:/icons/hubsight-512.png"
                sourceSize: Qt.size(32, 32)
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                fillMode: Image.PreserveAspectFit
            }

            ColumnLayout {
                spacing: 1
                Label {
                    text: qsTr("HUBSIGHT")
                    color: root.theme.text
                    font.family: "Roboto"
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Label {
                    text: root.controller.stepTitle
                    color: root.theme.textSecondary
                    font.family: "Roboto"
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Label {
                    text: root.controller.stepSubtitle
                    color: root.theme.textMuted
                    font.family: "Roboto"
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }
            }
            Item { Layout.fillWidth: true }
            AppButton {
                text: root.controller.language === "en" ? "VI" : "EN"
                theme: root.theme
                primary: false
                compact: true
                Layout.preferredWidth: 46
                onClicked: root.controller.toggleLanguage()
                ToolTip.visible: hovered
                ToolTip.text: root.controller.language === "en"
                              ? qsTr("Switch to Vietnamese") : qsTr("Switch to English")
            }
            AppButton {
                text: root.controller.themeMode === "system"
                      ? "◐" : root.controller.themeMode === "dark" ? "☾" : "☀"
                theme: root.theme
                primary: false
                compact: true
                Layout.preferredWidth: 42
                onClicked: root.controller.toggleTheme()
                ToolTip.visible: hovered
                ToolTip.text: root.controller.themeMode === "system"
                              ? qsTr("Switch to light theme")
                              : root.controller.themeMode === "light"
                                ? qsTr("Switch to dark theme") : qsTr("Use system theme")
            }
        }

        RowLayout {
            visible: root.controller.importStep > 0
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: 3
                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4
                    radius: 2
                    color: index < root.controller.importStep
                           ? root.theme.accent : root.theme.surfaceMuted
                }
            }
        }

        Label {
            visible: root.controller.errorMessage.length > 0
            text: root.controller.errorMessage
            Layout.fillWidth: true
            color: root.theme.errorText
            padding: 10
            wrapMode: Text.WordWrap
            background: Rectangle {
                color: root.theme.errorSoft
                radius: 6
                border.color: root.theme.errorText
            }
        }

        Label {
            visible: root.controller.busy
            text: "⟳  " + root.controller.busyMessage
            Layout.fillWidth: true
            color: root.theme.accentText
            font.family: "Roboto"
            font.pixelSize: 13
            font.weight: Font.DemiBold
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.controller.importStep

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 40
                    anchors.rightMargin: 40
                    anchors.topMargin: 8
                    anchors.bottomMargin: 10
                    spacing: 18

                    Image {
                        source: "qrc:/icons/hubsight-512.png"
                        sourceSize: Qt.size(92, 92)
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 92
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Label {
                        text: qsTr("Welcome to HubSight")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Import a secure server configuration to connect this desktop app to your HubSight environment.")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: root.theme.textMuted
                        font.family: "Roboto"
                        font.pixelSize: 14
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredHeight: 156
                        FeatureCard {
                            id: encryptedCard
                            theme: root.theme
                            iconSource: "qrc:/icons/features/encrypted.svg"
                            title: qsTr("Encrypted by default")
                            description: qsTr("The PIN and decrypted profile stay in memory during import.")
                            x: 0
                            width: Math.max(0, (parent.width - 24) / 3)
                            height: parent.height
                        }
                        FeatureCard {
                            theme: root.theme
                            iconSource: "qrc:/icons/features/integrity.svg"
                            title: qsTr("Integrity checked")
                            description: qsTr("The SDK validates the container and its content hash.")
                            x: encryptedCard.width + 12
                            width: encryptedCard.width
                            height: parent.height
                        }
                        FeatureCard {
                            theme: root.theme
                            iconSource: "qrc:/icons/features/setup.svg"
                            title: qsTr("Zero manual setup")
                            description: qsTr("Endpoints and credentials are read from one verified profile.")
                            x: (encryptedCard.width + 12) * 2
                            width: encryptedCard.width
                            height: parent.height
                        }
                    }
                    AppButton {
                        text: qsTr("Start setup  →")
                        theme: root.theme
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 280
                        onClicked: root.controller.startSetup()
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 40
                    spacing: 12
                    Label {
                        text: qsTr("Import configuration file")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Select a HubSight Admin .hscfg profile. The SDK will decrypt and validate it with your 6-digit PIN.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Item { Layout.preferredHeight: 8 }
                    AppCard {
                        id: dropCard
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 250
                        DropArea {
                            anchors.fill: parent
                            onDropped: function(drop) {
                                if (drop.hasUrls && drop.urls.length > 0)
                                    root.controller.loadConfigFile(drop.urls[0])
                            }
                        }
                        ColumnLayout {
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 56, 520)
                            spacing: 10
                            Label {
                                text: "↑"
                                color: root.theme.accent
                                font.pixelSize: 34
                                font.weight: Font.DemiBold
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Label {
                                text: root.controller.fileReady
                                      ? root.controller.fileName
                                      : qsTr("Drop a .hscfg file here or choose one")
                                color: root.theme.text
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            Label {
                                text: root.controller.fileReady
                                      ? root.controller.fileMeta
                                      : qsTr("Encrypted Admin configuration profiles are supported.")
                                color: root.theme.textMuted
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            AppButton {
                                text: qsTr("Choose .hscfg file")
                                theme: root.theme
                                primary: false
                                Layout.alignment: Qt.AlignHCenter
                                onClicked: fileDialog.open()
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    RowLayout {
                        Layout.fillWidth: true
                        AppButton {
                            text: qsTr("Back")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: qsTr("Continue to PIN  →")
                            theme: root.theme
                            enabled: root.controller.fileReady && !root.controller.busy
                            onClicked: root.controller.continueToPin()
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 40
                    spacing: 12
                    Label {
                        text: qsTr("Enter your security PIN")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Use the 6-digit PIN that was provided with this configuration profile. It is never stored.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: qsTr("FILE") + "  •  " + root.controller.fileName
                        color: root.theme.successText
                        padding: 8
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        background: Rectangle { color: root.theme.successSoft; radius: 6; border.color: root.theme.successText }
                    }
                    AppCard {
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220
                        ColumnLayout {
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 52, 520)
                            spacing: 12
                            Label {
                                text: qsTr("6-DIGIT PASSCODE")
                                color: root.theme.textMuted
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                                Layout.alignment: Qt.AlignHCenter
                            }
                            AppTextField {
                                id: pinField
                                theme: root.theme
                                text: root.controller.pin
                                echoMode: TextInput.Password
                                maximumLength: 6
                                inputMethodHints: Qt.ImhDigitsOnly
                                horizontalAlignment: TextInput.AlignHCenter
                                font.pixelSize: 24
                                Layout.fillWidth: true
                                onTextChanged: {
                                    if (root.controller.pin !== text)
                                        root.controller.pin = text
                                }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    RowLayout {
                        Layout.fillWidth: true
                        AppButton {
                            text: qsTr("Back")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: qsTr("Decrypt and validate  →")
                            theme: root.theme
                            enabled: root.controller.pin.length === 6 && !root.controller.busy
                            onClicked: root.controller.validatePin()
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 40
                    spacing: 12
                    Label {
                        text: qsTr("Review configuration")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("The configuration was decrypted and validated. Review the destination before activating it.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: qsTr("✓  CRYPTOGRAPHIC INTEGRITY CHECKED")
                        color: root.theme.successText
                        padding: 8
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        background: Rectangle { color: root.theme.successSoft; radius: 6; border.color: root.theme.successText }
                    }
                    AppCard {
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 270
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 0
                            Repeater {
                                model: root.controller.summaryRows
                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 34
                                    Label {
                                        text: modelData.label
                                        color: root.theme.textMuted
                                        Layout.preferredWidth: 150
                                        font.pixelSize: 13
                                    }
                                    Label {
                                        text: modelData.value
                                        color: root.theme.text
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                        font.family: "monospace"
                                        font.pixelSize: 13
                                    }
                                }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                    RowLayout {
                        Layout.fillWidth: true
                        AppButton {
                            text: qsTr("Back to PIN")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: qsTr("Activate and continue to sign in  →")
                            theme: root.theme
                            enabled: !root.controller.busy
                            onClicked: root.controller.confirmImport()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: root.theme.borderSubtle
        }
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("© 2026 HubSight. All rights reserved.")
                color: root.theme.textMuted
                font.pixelSize: 11
            }
            Item { Layout.fillWidth: true }
            Label {
                text: qsTr("Version") + " " + root.controller.appVersion
                color: root.theme.textMuted
                font.pixelSize: 11
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Select HubSight configuration")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("HubSight configuration (*.hscfg)")]
        onAccepted: root.controller.loadConfigFile(selectedFile)
    }
}
