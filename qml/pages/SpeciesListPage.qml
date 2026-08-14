import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property int _deleteId: -1

    function _subtitle(family, sowing, days) {
        var parts = [];
        if (family)
            parts.push(family);
        if (sowing)
            parts.push(sowing);
        if (days)
            parts.push(days + "gg");
        return parts.join(" \u00b7 ");
    }

    Component.onCompleted: speciesModel.loadAll()

    RemorsePopup {
        id: remorsePopup
        onTriggered: {
            if (_deleteId > 0)
                speciesModel.deleteSpecies(_deleteId);
            _deleteId = -1;
        }
    }

    SilicaListView {
        id: listView
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("New species")
                onClicked: pageStack.push(Qt.resolvedUrl("SpeciesDetailPage.qml"))
            }
        }

        header: Column {
            width: listView.width

            PageHeader {
                title: qsTr("Species")
            }

            SearchField {
                id: searchField
                width: parent.width
                placeholderText: qsTr("Search species…")
                onTextChanged: {
                    if (text.length > 0) {
                        speciesModel.search(text);
                    } else {
                        speciesModel.loadAll();
                    }
                }
            }
        }

        model: speciesModel

        delegate: ListItem {
            width: parent.width
            contentHeight: Theme.itemSizeMedium

            IconButton {
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                icon.source: "image://theme/icon-m-delete"
                onClicked: {
                    _deleteId = model.speciesId;
                    remorsePopup.execute(qsTr("Deleting species"));
                }
            }

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                width: parent.width - 2 * Theme.horizontalPageMargin - Theme.itemSizeSmall - Theme.paddingMedium

                Label {
                    text: model.variety ? model.name + " (" + model.variety + ")" : (model.name || "")
                    color: parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    text: _subtitle(model.family, model.sowingSeason, model.harvestDays)
                    font.pixelSize: Theme.fontSizeSmall
                    color: parent.parent.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
            }

            onClicked: pageStack.push(Qt.resolvedUrl("SpeciesDetailPage.qml"), {
                speciesId: model.speciesId || 0
            })
        }

        ViewPlaceholder {
            enabled: speciesModel.count === 0
            text: qsTr("No species")
        }

        VerticalScrollDecorator {}
    }
}
