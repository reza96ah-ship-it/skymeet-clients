import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../"
import "../components"

Rectangle {
    id: root
    property var api
    color: Style.background

    Flickable {
        anchors.fill: parent
        anchors.bottomMargin: 84
        contentWidth: width
        contentHeight: content.implicitHeight + 30
        clip: true

        ColumnLayout {
            id: content
            width: parent.width
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 22
            anchors.top: parent.top
            anchors.topMargin: 24
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: "Streams"
                    color: Style.textPrimary
                    font.family: Style.fontFamily
                    font.pixelSize: 27
                    font.weight: Font.Bold
                }
                AppButton {
                    text: "Sync ZLM"
                    implicitHeight: 40
                    variant: "secondary"
                    onClicked: api.syncZlmStreams()
                }
                AppButton {
                    text: "Refresh"
                    implicitHeight: 40
                    variant: "secondary"
                    onClicked: api.fetchStreams()
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 330
                radiusValue: 24

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Text {
                        text: "Create stream"
                        color: Style.textPrimary
                        font.family: Style.fontFamily
                        font.pixelSize: 18
                        font.weight: Font.Bold
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "For rtsp://127.0.0.1/meeting/live, use app = meeting and stream = live."
                        color: Style.textSecondary
                        font.family: Style.fontFamily
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }

                    InputField {
                        id: streamName
                        Layout.fillWidth: true
                        placeholder: "Name"
                        iconText: "▶"
                        text: "Meeting Live"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        InputField {
                            id: appName
                            Layout.fillWidth: true
                            placeholder: "App"
                            iconText: "▦"
                            text: "meeting"
                        }
                        InputField {
                            id: streamKey
                            Layout.fillWidth: true
                            placeholder: "Stream"
                            iconText: "#"
                            text: "live"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        InputField {
                            id: protocol
                            Layout.fillWidth: true
                            placeholder: "Protocol"
                            iconText: "◎"
                            text: "rtsp"
                        }
                        InputField {
                            id: sourceType
                            Layout.fillWidth: true
                            placeholder: "Source type"
                            iconText: "⇧"
                            text: "rtsp_push"
                        }
                    }

                    InputField {
                        id: sourceUrl
                        Layout.fillWidth: true
                        placeholder: "Source URL, optional for push streams"
                        iconText: "⌁"
                        text: "rtsp://127.0.0.1/meeting/live"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        AppButton {
                            Layout.fillWidth: true
                            text: "Create Stream"
                            onClicked: api.createStream(streamName.text, appName.text, streamKey.text, sourceType.text, sourceUrl.text, protocol.text)
                        }
                        AppButton {
                            Layout.fillWidth: true
                            text: "Sync From ZLM"
                            variant: "secondary"
                            onClicked: api.syncZlmStreams()
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredHeight: 128
                radiusValue: 24
                color: Style.dark
                border.color: "#1E293B"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 8
                    Text {
                        text: "ZLMediaKit streams"
                        color: "white"
                        font.family: Style.fontFamily
                        font.pixelSize: 18
                        font.weight: Font.Bold
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Create a stream record here, or press Sync ZLM to import currently running streams from ZLMediaKit into MongoDB."
                        color: "#CBD5E1"
                        font.family: Style.fontFamily
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Repeater {
                model: api.streams
                delegate: StreamCard {
                    Layout.fillWidth: true
                    stream: modelData
                }
            }

            Text {
                visible: api.streams.length === 0
                Layout.fillWidth: true
                text: "No streams yet. Create meeting/live above, or start/push a stream in ZLMediaKit and press Sync ZLM."
                color: Style.textSecondary
                font.family: Style.fontFamily
                font.pixelSize: 14
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    component StreamCard: Card {
        id: card
        property var stream
        Layout.preferredHeight: 124
        radiusValue: 20
        RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 12
            Rectangle {
                Layout.preferredWidth: 52
                Layout.preferredHeight: 52
                radius: 18
                color: statusText(stream) === "online" ? "#D1FAE5" : Style.surfaceMuted
                Text {
                    anchors.centerIn: parent
                    text: "▶"
                    color: statusText(stream) === "online" ? Style.success : Style.textSecondary
                    font.pixelSize: 18
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    text: stream.name || stream.stream_key || "Stream"
                    color: Style.textPrimary
                    font.family: Style.fontFamily
                    font.pixelSize: 15
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                Text {
                    text: "Path: " + pathText(stream)
                    color: Style.textSecondary
                    font.family: Style.fontFamily
                    font.pixelSize: 12
                    elide: Text.ElideMiddle
                }
                Text {
                    text: "Status: " + statusText(stream) + " · Protocol: " + protocolText(stream)
                    color: Style.textSecondary
                    font.family: Style.fontFamily
                    font.pixelSize: 12
                }
                Text {
                    text: "URL: rtsp://127.0.0.1/" + pathText(stream)
                    color: Style.primary
                    font.family: Style.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideMiddle
                }
            }
        }
    }

    function pathText(s) {
        if (s.stream_key) {
            return s.stream_key
        }
        if (s.zlm) {
            return (s.zlm.app || "") + "/" + (s.zlm.stream || "")
        }
        return (s.app || "") + "/" + (s.stream || "")
    }

    function statusText(s) {
        if (s.runtime && s.runtime.status) {
            return s.runtime.status
        }
        return s.status || "offline"
    }

    function protocolText(s) {
        if (s.protocol) {
            return s.protocol
        }
        if (s.zlm && s.zlm.schema) {
            return s.zlm.schema
        }
        return "rtsp"
    }
}
