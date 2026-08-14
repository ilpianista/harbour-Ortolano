import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property int _deleteActivityId: 0

    function _fmtDate(isoStr) {
        return Qt.formatDate(new Date(isoStr), Qt.DefaultLocaleShortDate);
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("GardenCanvas.qml"));
            mainActivityModel.loadAll();
        }
    }

    RemorsePopup {
        id: remorsePopup
        onTriggered: {
            if (_deleteActivityId > 0)
                mainActivityModel.deleteActivity(_deleteActivityId);
            _deleteActivityId = 0;
            mainActivityModel.loadAll();
        }
    }

    SilicaListView {
        id: listView
        anchors.fill: parent
        model: mainActivityModel

        PullDownMenu {
            MenuItem {
                text: qsTr("Species")
                onClicked: pageStack.push(Qt.resolvedUrl("SpeciesListPage.qml"))
            }
            MenuItem {
                text: qsTr("New activity")
                onClicked: pageStack.push(Qt.resolvedUrl("ActivityEditPage.qml"))
            }
        }

        header: Column {
            width: listView.width

            PageHeader {
                title: "Ortolano"
            }

            InfoLabel {
                topPadding: Theme.paddingLarge
                text: qsTr("No activities")
                visible: mainActivityModel.count === 0
            }
        }

        section {
            property: "section"
            criteria: ViewSection.FullString
            delegate: SectionHeader {
                text: section === "pending" ? qsTr("Pending activities") : qsTr("Completed activities")
            }
        }

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
                    text: model.activityTypeName || ""
                    color: parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    text: {
                        var parts = [_fmtDate(model.date)];
                        if (model.areaNames)
                            parts.push(model.areaNames);
                        if (model.speciesName)
                            parts.push(model.speciesName);
                        if (model.quantity > 0)
                            parts.push(model.quantity.toLocaleString(Qt.locale(), 'f', 1) + " kg");
                        return parts.join(" \u00b7 ");
                    }
                    font.pixelSize: Theme.fontSizeSmall
                    color: parent.parent.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                    truncationMode: TruncationMode.Fade
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    visible: model.activityId > 0
                    onClicked: {
                        _deleteActivityId = model.activityId;
                        remorsePopup.execute(qsTr("Deleting activity"));
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
