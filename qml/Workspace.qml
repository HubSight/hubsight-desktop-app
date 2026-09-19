import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"

Item {
    id: root

    property var controller
    property QtObject theme
    property int section: 0

    Rectangle {
        anchors.fill: parent
        color: root.theme.background
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 232
            color: root.theme.surface
            border.color: root.theme.borderSubtle
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Image {
                        source: "qrc:/icons/hubsight-512.png"
                        sourceSize: Qt.size(34, 34)
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                    }
                    Label {
                        text: qsTr("HubSight")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }
                }
                Item { Layout.preferredHeight: 12 }
                Label {
                    text: qsTr("WORKSPACE")
                    color: root.theme.textMuted
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 4
                    model: [qsTr("Overview"), qsTr("Preferences"), qsTr("Help & About")]
                    delegate: NavButton {
                        width: ListView.view.width
                        text: modelData
                        theme: root.theme
                        selected: root.section === index
                        onClicked: root.section = index
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: root.theme.borderSubtle
                }
                Label {
                    text: root.controller.userDisplayName.length > 0
                          ? root.controller.userDisplayName : qsTr("Authenticated admin")
                    color: root.theme.textSecondary
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    font.pixelSize: 12
                }
                Label {
                    text: qsTr("Connected")
                    color: root.theme.successText
                    font.pixelSize: 11
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.section

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 34
                    spacing: 16
                    Label {
                        text: qsTr("Overview")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Your HubSight desktop workspace is ready.")
                        color: root.theme.textMuted
                        font.pixelSize: 14
                    }
                    AppCard {
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 150
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 8
                            Label {
                                text: qsTr("Connection active")
                                color: root.theme.successText
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: qsTr("Authentication completed successfully. Camera, member and system tools will appear here.")
                                color: root.theme.textSecondary
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                font.pixelSize: 14
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 34
                    spacing: 16
                    Label {
                        text: qsTr("Preferences")
                        color: root.theme.text
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: qsTr("Manage desktop behavior and local presentation settings.")
                        color: root.theme.textMuted
                        font.pixelSize: 14
                    }
                    AppCard {
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 226
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 12
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("Theme")
                                    color: root.theme.text
                                    Layout.fillWidth: true
                                }
                                ComboBox {
                                    Layout.preferredWidth: 156
                                    model: [qsTr("System"), qsTr("Light"), qsTr("Dark")]
                                    currentIndex: root.controller.themeMode === "light"
                                                  ? 1 : root.controller.themeMode === "dark" ? 2 : 0
                                    onActivated: root.controller.setThemeMode(
                                                     index === 1 ? "light"
                                                                 : index === 2 ? "dark" : "system")
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("Launch at login")
                                    color: root.theme.text
                                    Layout.fillWidth: true
                                }
                                Switch { checked: false }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: qsTr("Show connection status")
                                    color: root.theme.text
                                    Layout.fillWidth: true
                                }
                                Switch { checked: true }
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 34
                    spacing: 16
                    Label {
                        text: qsTr("Help & About")
                        color: root.theme.text
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    AppCard {
                        theme: root.theme
                        Layout.fillWidth: true
                        Layout.preferredHeight: 190
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 8
                            Label {
                                text: qsTr("HubSight Desktop")
                                color: root.theme.text
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }
                            Label {
                                text: qsTr("Qt 6 · C++20 · HubSight Admin SDK")
                                color: root.theme.textMuted
                                font.pixelSize: 13
                            }
                            Label {
                                text: qsTr("Import a verified .hscfg configuration, authenticate securely, and manage your HubSight environment from one desktop workspace.")
                                color: root.theme.textSecondary
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                font.pixelSize: 14
                            }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
