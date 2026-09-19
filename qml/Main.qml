import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"

ApplicationWindow {
    id: root

    visible: false
    width: 1120
    height: 760
    minimumWidth: 900
    minimumHeight: 620
    title: appController.screen === 0 ? qsTr("HubSight — Configuration setup")
                                      : appController.screen === 1 ? qsTr("HubSight — Sign in")
                                                                    : qsTr("HubSight")
    color: theme.background

    QtObject {
        id: theme
        property bool dark: appController.darkMode
        property color background: dark ? "#0b0f14" : "#f8fafc"
        property color surface: dark ? "#111820" : "#ffffff"
        property color surfaceSubtle: dark ? "#151d27" : "#f8fafc"
        property color surfaceMuted: dark ? "#1c2733" : "#f1f5f9"
        property color border: dark ? "#334155" : "#cbd5e1"
        property color borderSubtle: dark ? "#80334155" : "#b3cbd5e1"
        property color text: dark ? "#f8fafc" : "#0f172a"
        property color textSecondary: dark ? "#cbd5e1" : "#334155"
        property color textMuted: dark ? "#94a3b8" : "#64748b"
        property color accent: "#ea580c"
        property color accentHover: "#c2410c"
        property color accentSoft: dark ? "#431407" : "#fff7ed"
        property color accentText: dark ? "#fb923c" : "#c2410c"
        property color successSoft: dark ? "#052e16" : "#ecfdf5"
        property color successText: dark ? "#6ee7b7" : "#047857"
        property color errorSoft: dark ? "#450a0a" : "#fef2f2"
        property color errorText: dark ? "#fca5a5" : "#b91c1c"
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: appController.screen

        ImportWizard {
            controller: appController
            theme: theme
        }
        AuthPage {
            controller: appController
            theme: theme
        }
        Workspace {
            controller: appController
            theme: theme
        }
    }
}
