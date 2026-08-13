import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    signal speciesPicked(int speciesId, string name)

    function _subtitle(family, sowing, days) {
        if (!family)
            return "";
        var s = family;
        if (sowing)
            s += " \u00b7 " + sowing;
        s += " \u00b7 " + days + "gg";
        return s;
    }

    Component.onCompleted: speciesModel.loadAll()

    onStatusChanged: {
        if (status === PageStatus.Active)
            speciesModel.loadAll();
    }

    SilicaListView {
        id: listView
        anchors.fill: parent

        header: Column {
            width: listView.width

            PageHeader {
                title: qsTr("Select species")
            }

            SearchField {
                id: searchField
                width: parent.width
                placeholderText: qsTr("Search species...")
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

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                width: parent.width - 2 * Theme.horizontalPageMargin

                Label {
                    text: model.variety ? (model.name + " (" + model.variety + ")") : (model.name || "")
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

            onClicked: {
                page.speciesPicked(model.speciesId, model.name);
                pageStack.pop();
            }
        }

        ViewPlaceholder {
            enabled: speciesModel.count === 0
            text: qsTr("No species")
        }

        VerticalScrollDecorator {}
    }
}
