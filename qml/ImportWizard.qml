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
                    text: "HUBSIGHT"
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
                ScrollView {
                    anchors.fill: parent
                    clip: true
                    ColumnLayout {
                        width: Math.max(1, root.width - 80)
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 18

                        Item { Layout.preferredHeight: 8 }
                        Image {
                            source: "qrc:/icons/hubsight-512.png"
                            sourceSize: Qt.size(92, 92)
                            Layout.preferredWidth: 92
                            Layout.preferredHeight: 92
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Label {
                            text: "Welcome to HubSight"
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            color: root.theme.text
                            font.family: "Roboto"
                            font.pixelSize: 24
                            font.weight: Font.DemiBold
                        }
                        Label {
                            text: "Import a secure server configuration to connect this desktop app to your HubSight environment."
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
                            Layout.preferredHeight: 112
                            FeatureCard {
                                id: encryptedCard
                                theme: root.theme
                                title: "Encrypted by default"
                                description: "The PIN and decrypted profile stay in memory during import."
                                x: 0
                                width: Math.max(0, (parent.width - 24) / 3)
                                height: parent.height
                            }
                            FeatureCard {
                                theme: root.theme
                                title: "Integrity checked"
                                description: "The SDK validates the container and its content hash."
                                x: encryptedCard.width + 12
                                width: encryptedCard.width
                                height: parent.height
                            }
                            FeatureCard {
                                theme: root.theme
                                title: "Zero manual setup"
                                description: "Endpoints and credentials are read from one verified profile."
                                x: (encryptedCard.width + 12) * 2
                                width: encryptedCard.width
                                height: parent.height
                            }
                        }
                        AppButton {
                            text: "Start setup  →"
                            theme: root.theme
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 280
                            onClicked: root.controller.startSetup()
                        }
                        Item { Layout.preferredHeight: 10 }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 40
                    spacing: 12
                    Label {
                        text: "Import configuration file"
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: "Select a HubSight Admin .hscfg profile. The SDK will decrypt and validate it with your 6-digit PIN."
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
                                text: root.controller.fileReady ? root.controller.fileName : "Drop a .hscfg file here or choose one"
                                color: root.theme.text
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            Label {
                                text: root.controller.fileReady ? root.controller.fileMeta : "Encrypted Admin configuration profiles are supported."
                                color: root.theme.textMuted
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            AppButton {
                                text: "Choose .hscfg file"
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
                            text: "Back"
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: "Continue to PIN  →"
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
                        text: "Enter your security PIN"
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: "Use the 6-digit PIN that was provided with this configuration profile. It is never stored."
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: "FILE  •  " + root.controller.fileName
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
                                text: "6-DIGIT PASSCODE"
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
                            text: "Back"
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: "Decrypt and validate  →"
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
                        text: "Review configuration"
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: "The configuration was decrypted and validated. Review the destination before activating it."
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: "✓  CRYPTOGRAPHIC INTEGRITY CHECKED"
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
                            text: "Back to PIN"
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: "Activate and continue to sign in  →"
                            theme: root.theme
                            enabled: !root.controller.busy
                            onClicked: root.controller.confirmImport()
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select HubSight configuration"
        fileMode: FileDialog.OpenFile
        nameFilters: ["HubSight configuration (*.hscfg)"]
        onAccepted: root.controller.loadConfigFile(selectedFile)
    }
}
