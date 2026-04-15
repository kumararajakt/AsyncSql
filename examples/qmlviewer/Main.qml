import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 900
    height: 600
    visible: true
    title: "Async SQL — QML Viewer"

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            Button {
                text: "Select"
                onClicked: sqlModel.select()
            }

            Button {
                text: "Submit"
                onClicked: sqlModel.submitAll()
            }

            Button {
                text: "Revert"
                onClicked: sqlModel.revertAll()
            }

            Item { Layout.fillWidth: true }

            Label {
                id: statusLabel
                text: "Ready"
                font.italic: true
            }
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            Label { text: "Filter:" }
            TextField {
                id: filterField
                Layout.fillWidth: true
                placeholderText: "e.g. amount > 100"
            }
            Button {
                text: "Apply"
                onClicked: {
                    sqlModel.setFilter(filterField.text.trim())
                    sqlModel.select()
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TableView {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: sqlModel

            delegate: TextField {
                implicitWidth: 150
                implicitHeight: 36
                text: tableView.model.data(tableView.model.index(row, column)) ?? ""
                selectByMouse: true
                onEditingFinished: {
                    sqlModel.setData(sqlModel.index(row, column), text, Qt.EditRole)
                }
            }
        }

        // Detail panel showing current row
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            color: "#f5f5f5"
            visible: sqlModel.currentRow >= 0

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 4

                Label {
                    text: "Row " + sqlModel.currentRow
                    font.bold: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: sqlModel.columnCount() > 0 ? sqlModel.columnCount() : 0
                    delegate: RowLayout {
                        spacing: 8
                        Label {
                            text: sqlModel.headerData(modelData, Qt.Horizontal, Qt.DisplayRole) + ":"
                            font.bold: true
                            width: 120
                        }
                        Label {
                            text: String(sqlModel.field(sqlModel.headerData(modelData, Qt.Horizontal, Qt.DisplayRole)))
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    // Status messages
    Connections {
        target: sqlModel
        function onSelected(successful) {
            statusLabel.text = successful ? "Select done!" : "Error: " + sqlModel.lastError().text
        }
        function onSubmitted(successful) {
            statusLabel.text = successful ? "Submit done!" : "Error: " + sqlModel.lastError().text
        }
        function onBusyChanged(busy) {
            if (busy) statusLabel.text = "Processing..."
        }
        function onCurrentRowChanged(row) {
            tableView.positionViewAtRow(row)
        }
    }
}
