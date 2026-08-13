import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property var selectedIds: []

    signal areasPicked(var areaIds)

    function _toggleAll() {
        var allSelected = selectedIds.length === areaModel.count && areaModel.count > 0;
        if (allSelected) {
            selectedIds = [];
        } else {
            var ids = [];
            for (var i = 0; i < areaModel.count; i++)
                ids.push(areaModel.get(i).areaId);
            selectedIds = ids;
        }
    }

    function _toggle(areaId) {
        var idx = selectedIds.indexOf(areaId);
        if (idx >= 0) {
            selectedIds.splice(idx, 1);
        } else {
            selectedIds.push(areaId);
        }
        selectedIds = selectedIds;
    }

    canAccept: selectedIds.length > 0

    onAccepted: {
        areasPicked(selectedIds);
    }

    Component.onCompleted: areaModel.loadAll()

    onStatusChanged: {
        if (status === PageStatus.Active)
            areaModel.loadAll();
    }

    SilicaListView {
        id: listView
        anchors.fill: parent

        header: DialogHeader {
            title: qsTr("Select beds")
            acceptText: qsTr("Confirm")
        }

        model: areaModel

        delegate: ListItem {
            width: parent.width
            contentHeight: Theme.itemSizeMedium

            property bool _checked: dialog.selectedIds.indexOf(model.areaId) >= 0

            Row {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                spacing: Theme.paddingMedium

                Rectangle {
                    width: 28
                    height: 28
                    radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: _checked ? Theme.highlightColor : "transparent"
                    border {
                        width: 2
                        color: _checked ? Theme.highlightColor : Theme.secondaryColor
                    }

                    Label {
                        anchors.centerIn: parent
                        text: _checked ? "\u2713" : ""
                        color: Theme.darkPrimaryColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                }

                Label {
                    text: model.name || ""
                    color: parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            onClicked: _toggle(model.areaId)
        }

        PullDownMenu {
            MenuItem {
                text: selectedIds.length === areaModel.count && areaModel.count > 0 ? qsTr("Deselect all") : qsTr("Select all")
                onClicked: _toggleAll()
            }
        }

        VerticalScrollDecorator {}
    }
}
