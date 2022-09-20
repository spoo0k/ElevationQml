import QtQuick 2.15
import QtQuick.Window 2.15
import CastomPlot 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    RouteElevation{
        anchors.fill: parent
        Component.onCompleted: initQwtPlot()
    }
}
