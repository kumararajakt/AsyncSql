import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 700
    height: 600
    visible: true
    title: "Async SQL — List Viewer"

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            Button {
                text: "Select"
                onClicked: listModel.select()
            }

            Button {
                text: "Submit"
                onClicked: listModel.submitAll()
            }

            Button {
                text: "Revert"
                onClicked: listModel.revertAll()
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
                placeholderText: "e.g. amount_paid > 100"
            }
            Button {
                text: "Apply"
                onClicked: {
                    listModel.setFilter(filterField.text.trim())
                    listModel.select()
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left: list of sales records
        ListView {
            id: listView
            Layout.preferredWidth: parent.width * 0.45
            Layout.fillHeight: true
            clip: true
            model: listModel

            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                width: listView.width
                highlighted: listModel.currentRow === index

                contentItem: ColumnLayout {
                    spacing: 2

                    Label {
                        text: "TX: " + (model.transact_id ?? "")
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        Label {
                            text: "Qty: " + (model.quantity ?? "")
                            font.pixelSize: 11
                            color: "#555"
                        }
                        Label {
                            text: "Total: " + (model.total_cost ?? "")
                            font.pixelSize: 11
                            color: "#555"
                        }
                        Label {
                            text: "Paid: " + (model.amount_paid ?? "")
                            font.pixelSize: 11
                            color: "#555"
                        }
                    }
                }

                onClicked: listModel.currentRow = index
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                visible: listView.count === 0 && !listModel.isBusy()
                text: "No records"
                color: "#888"
            }
        }

        // Divider
        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: "#ccc"
        }

        // Right: detail panel for selected row
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Label {
                text: listModel.currentRow >= 0
                    ? "Row " + listModel.currentRow
                    : "Select a row"
                font.bold: true
                padding: 12
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#ddd"
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                visible: listModel.currentRow >= 0

                // Use a Repeater over the role names exposed by the model.
                // roleNames() returns column names; we iterate a fixed list of
                // the sales table columns to display them in order.
                ListView {
                    id: detailView
                    model: [
                        "id", "transact_id", "name", "category", "item",
                        "unit_price", "quantity", "unit", "total_cost",
                        "amount_paid", "balance", "currency", "notes",
                        "created", "last_edited"
                    ]

                    delegate: Rectangle {
                        width: detailView.width
                        height: 36
                        color: index % 2 === 0 ? "#fafafa" : "#f0f0f0"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            Label {
                                text: modelData + ":"
                                font.bold: true
                                Layout.preferredWidth: 110
                                elide: Text.ElideRight
                            }

                            Label {
                                text: String(listModel.field(modelData) ?? "")
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }

            // Placeholder when nothing selected
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: listModel.currentRow < 0

                Label {
                    anchors.centerIn: parent
                    text: "No row selected"
                    color: "#aaa"
                }
            }
        }
    }

    Connections {
        target: listModel
        function onSelected(successful) {
            statusLabel.text = successful
                ? "Loaded " + listModel.rowCount() + " rows"
                : "Error: " + listModel.lastError().text
        }
        function onSubmitted(successful) {
            statusLabel.text = successful ? "Submit done!" : "Error: " + listModel.lastError().text
        }
        function onBusyChanged(busy) {
            if (busy) statusLabel.text = "Loading..."
        }
    }
}
