import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

CoverBackground {
    id: cover

    property string _d1: ""
    property string _t1: ""
    property string _d2: ""
    property string _t2: ""
    property string _d3: ""
    property string _t3: ""
    property int _count: 0

    Timer {
        id: _refreshTimer
        interval: 500
        repeat: false
        onTriggered: _refresh()
    }

    function _fmtDate(isoStr) {
        return Qt.formatDate(new Date(isoStr), Qt.DefaultLocaleShortDate);
    }

    function _refresh() {
        activityModel.loadPending();
        _count = activityModel.rowCount();
        if (_count > 0) {
            var a = activityModel.get(0);
            _d1 = _fmtDate(a.date);
            _t1 = a.activityTypeName || "";
            if (a.areaNames)
                _t1 += " \u00b7 " + a.areaNames;
        }
        if (_count > 1) {
            var a = activityModel.get(1);
            _d2 = _fmtDate(a.date);
            _t2 = a.activityTypeName || "";
            if (a.areaNames)
                _t2 += " \u00b7 " + a.areaNames;
        }
        if (_count > 2) {
            var a = activityModel.get(2);
            _d3 = _fmtDate(a.date);
            _t3 = a.activityTypeName || "";
            if (a.areaNames)
                _t3 += " \u00b7 " + a.areaNames;
        }
    }

    Component.onCompleted: _refreshTimer.start()

    Connections {
        target: activityModel
        onModelReset: _refreshTimer.start()
        onRowsInserted: _refreshTimer.start()
        onRowsRemoved: _refreshTimer.start()
        onDataChanged: _refreshTimer.start()
    }

    Column {
        anchors {
            left: parent.left
            leftMargin: Theme.paddingLarge
            right: parent.right
            rightMargin: Theme.paddingLarge
            verticalCenter: parent.verticalCenter
        }
        spacing: 2

        Label {
            text: qsTr("Pending activities")
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.highlightColor
            visible: _count > 0
        }

        Label {
            text: qsTr("No scheduled activities")
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
            visible: _count === 0
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Row {
            width: parent.width
            visible: _count > 0
            Label {
                id: dateLabel1
                text: _d1 + " "
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
            }
            Label {
                text: _t1
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
                width: parent.width - dateLabel1.width
                truncationMode: TruncationMode.Fade
            }
        }

        Row {
            width: parent.width
            visible: _count > 1
            Label {
                id: dateLabel2
                text: _d2 + " "
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
            }
            Label {
                text: _t2
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
                width: parent.width - dateLabel2.width
                truncationMode: TruncationMode.Fade
            }
        }

        Row {
            width: parent.width
            visible: _count > 2
            Label {
                id: dateLabel3
                text: _d3 + " "
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.highlightColor
            }
            Label {
                text: _t3
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
                width: parent.width - dateLabel3.width
                truncationMode: TruncationMode.Fade
            }
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: {
                app.activate();
                pageStack.push(Qt.resolvedUrl("../pages/ActivityEditPage.qml"));
            }
        }
    }
}
