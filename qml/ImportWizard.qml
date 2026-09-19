import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import "components"

Item {
    id: root

    property var controller
    property QtObject theme
    function t(english, vietnamese) {
        return controller.language === "vi" ? vietnamese : english
    }

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
            AppButton {
                text: root.controller.language === "en" ? "VI" : "EN"
                theme: root.theme
                primary: false
                compact: true
                Layout.preferredWidth: 46
                onClicked: root.controller.toggleLanguage()
                ToolTip.visible: hovered
                ToolTip.text: root.controller.language === "en"
                              ? "Chuyển sang tiếng Việt" : "Switch to English"
            }
            AppButton {
                text: root.controller.darkMode ? "☀" : "☾"
                theme: root.theme
                primary: false
                compact: true
                Layout.preferredWidth: 42
                onClicked: root.controller.toggleTheme()
                ToolTip.visible: hovered
                ToolTip.text: root.controller.darkMode
                              ? root.t("Switch to light theme", "Chuyển sang giao diện sáng")
                              : root.t("Switch to dark theme", "Chuyển sang giao diện tối")
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
                        text: root.t("Welcome to HubSight", "Chào mừng đến với HubSight")
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: root.t(
                                  "Import a secure server configuration to connect this desktop app to your HubSight environment.",
                                  "Nhập cấu hình máy chủ bảo mật để kết nối ứng dụng desktop với hệ thống HubSight của bạn.")
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
                            title: root.t("Encrypted by default", "Mã hóa mặc định")
                            description: root.t(
                                             "The PIN and decrypted profile stay in memory during import.",
                                             "Mã PIN và cấu hình đã giải mã chỉ được giữ trong bộ nhớ khi nhập.")
                            x: 0
                            width: Math.max(0, (parent.width - 24) / 3)
                            height: parent.height
                        }
                        FeatureCard {
                            theme: root.theme
                            iconSource: "qrc:/icons/features/integrity.svg"
                            title: root.t("Integrity checked", "Đã kiểm tra toàn vẹn")
                            description: root.t(
                                             "The SDK validates the container and its content hash.",
                                             "SDK xác thực container và mã băm nội dung.")
                            x: encryptedCard.width + 12
                            width: encryptedCard.width
                            height: parent.height
                        }
                        FeatureCard {
                            theme: root.theme
                            iconSource: "qrc:/icons/features/setup.svg"
                            title: root.t("Zero manual setup", "Không cần thiết lập thủ công")
                            description: root.t(
                                             "Endpoints and credentials are read from one verified profile.",
                                             "Các endpoint và thông tin xác thực được đọc từ một cấu hình đã xác minh.")
                            x: (encryptedCard.width + 12) * 2
                            width: encryptedCard.width
                            height: parent.height
                        }
                    }
                    AppButton {
                        text: root.t("Start setup  →", "Bắt đầu thiết lập  →")
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
                        text: root.t("Import configuration file", "Nhập tệp cấu hình")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: root.t(
                                  "Select a HubSight Admin .hscfg profile. The SDK will decrypt and validate it with your 6-digit PIN.",
                                  "Chọn cấu hình HubSight Admin .hscfg. SDK sẽ giải mã và xác thực bằng mã PIN 6 chữ số.")
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
                                      : root.t("Drop a .hscfg file here or choose one",
                                               "Thả tệp .hscfg vào đây hoặc chọn tệp")
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
                                      : root.t("Encrypted Admin configuration profiles are supported.",
                                               "Hỗ trợ cấu hình Admin được mã hóa.")
                                color: root.theme.textMuted
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            AppButton {
                                text: root.t("Choose .hscfg file", "Chọn tệp .hscfg")
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
                            text: root.t("Back", "Quay lại")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: root.t("Continue to PIN  →", "Tiếp tục đến PIN  →")
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
                        text: root.t("Enter your security PIN", "Nhập mã PIN bảo mật")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: root.t(
                                  "Use the 6-digit PIN that was provided with this configuration profile. It is never stored.",
                                  "Sử dụng mã PIN 6 chữ số được cung cấp cùng cấu hình này. Mã PIN không được lưu.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: root.t("FILE", "TỆP") + "  •  " + root.controller.fileName
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
                                text: root.t("6-DIGIT PASSCODE", "MÃ PIN 6 CHỮ SỐ")
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
                            text: root.t("Back", "Quay lại")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: root.t("Decrypt and validate  →", "Giải mã và xác thực  →")
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
                        text: root.t("Review configuration", "Kiểm tra cấu hình")
                        color: root.theme.text
                        font.family: "Roboto"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Label {
                        text: root.t(
                                  "The configuration was decrypted and validated. Review the destination before activating it.",
                                  "Cấu hình đã được giải mã và xác thực. Hãy kiểm tra đích trước khi kích hoạt.")
                        color: root.theme.textMuted
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.pixelSize: 14
                    }
                    Label {
                        text: root.t("✓  CRYPTOGRAPHIC INTEGRITY CHECKED",
                                     "✓  ĐÃ KIỂM TRA TOÀN VẸN MẬT MÃ")
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
                            text: root.t("Back to PIN", "Quay lại PIN")
                            theme: root.theme
                            primary: false
                            onClicked: root.controller.goBack()
                        }
                        Item { Layout.fillWidth: true }
                        AppButton {
                            text: root.t("Activate and continue to sign in  →",
                                         "Kích hoạt và tiếp tục đăng nhập  →")
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
                text: root.t("© 2026 HubSight. All rights reserved.",
                             "© 2026 HubSight. Đã đăng ký bản quyền.")
                color: root.theme.textMuted
                font.pixelSize: 11
            }
            Item { Layout.fillWidth: true }
            Label {
                text: root.t("Version", "Phiên bản") + " " + root.controller.appVersion
                color: root.theme.textMuted
                font.pixelSize: 11
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: root.t("Select HubSight configuration", "Chọn cấu hình HubSight")
        fileMode: FileDialog.OpenFile
        nameFilters: [root.t("HubSight configuration (*.hscfg)",
                             "Cấu hình HubSight (*.hscfg)")]
        onAccepted: root.controller.loadConfigFile(selectedFile)
    }
}
