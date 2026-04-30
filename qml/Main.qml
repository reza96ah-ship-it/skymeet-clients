import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SkyMeet.Backend 1.0
import "./pages"
import "./components"

ApplicationWindow {
    id: window
    width: 430
    height: 932
    minimumWidth: 390
    minimumHeight: 760
    visible: true
    title: "SkyMeet Client"
    color: "#F8FAFC"

    property int currentPage: api.authenticated ? 1 : 0
    property string toastMessage: ""
    property bool toastError: false

    ApiClient {
        id: api
        onOperationOk: function(name, message) {
            toastError = false
            toastMessage = message
            toastTimer.restart()
        }
        onOperationFailed: function(name, message) {
            toastError = true
            toastMessage = message
            toastTimer.restart()
        }
        onAuthenticatedChanged: {
            currentPage = authenticated ? 1 : 0
        }
    }

    EventSocket {
        id: events
        url: api.baseUrl.replace("http://", "ws://").replace("https://", "wss://") + "/ws/v1/streams"
    }

    Timer {
        id: toastTimer
        interval: 4000
        onTriggered: toastMessage = ""
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: currentPage

        LoginPage {
            api: api
        }

        HomePage {
            api: api
            events: events
            onOpenSessions: currentPage = 2
            onOpenUsers: currentPage = 3
            onOpenStreams: currentPage = 4
        }

        SessionsPage {
            api: api
        }

        UsersPage {
            api: api
        }

        StreamsPage {
            api: api
        }

        SettingsPage {
            api: api
            events: events
        }
    }

    Rectangle {
        id: bottomBar
        visible: api.authenticated && currentPage !== 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 84
        color: "#FFFFFF"
        border.color: "#E2E8F0"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            anchors.bottomMargin: 8
            spacing: 8

            TabButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconText: "⌂"
                label: "Home"
                active: currentPage === 1
                onClicked: currentPage = 1
            }
            TabButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconText: "▭"
                label: "Sessions"
                active: currentPage === 2
                onClicked: currentPage = 2
            }
            TabButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconText: "♙"
                label: "Users"
                active: currentPage === 3
                onClicked: currentPage = 3
            }
            TabButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconText: "▶"
                label: "Streams"
                active: currentPage === 4
                onClicked: currentPage = 4
            }
            TabButton {
                Layout.fillWidth: true
                Layout.fillHeight: true
                iconText: "⚙"
                label: "Settings"
                active: currentPage === 5
                onClicked: currentPage = 5
            }
        }
    }

    Toast {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomBar.visible ? bottomBar.top : parent.bottom
        anchors.margins: 18
        message: toastMessage
        error: toastError
        z: 10
    }

    Rectangle {
        visible: api.loading
        anchors.fill: parent
        color: "#66000000"
        z: 20
        BusyIndicator {
            anchors.centerIn: parent
            running: api.loading
        }
    }
}
