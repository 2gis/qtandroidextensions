import QtQuick 2.0
import SceneGraphRendering 1.0

Item {
	width: 400
    height: 400

	OffscreenEditText {
		id: edittext
		x: 20
		y: 20
		width: parent.width - 40
		height: 200
		Component.onCompleted: {
			edittext.setAllowFullscreenKeyboard(false)
			edittext.setSingleLine(true)
			edittext.setHint("Hello QML World")
			edittext.backgroundColor = "yellow"
		}
	}

	OffscreenWebView {
		id: webview
		x: 20
		y: 240
		width: parent.width - 40
		height: parent.height - 260
		Component.onCompleted: {
			webview.loadUrl("http://www.android.com/intl/en/about/")
			webview.setWebContentsDebuggingEnabled(true)
		}
    }

    Rectangle {
        id: labelFrame
        anchors.margins: -10
        radius: 5
        color: "white"
        border.color: "black"
        opacity: 0.8
		anchors.fill: aboutExample
    }

    Text {
		id: aboutExample
		anchors.bottom: webview.bottom
		anchors.left: webview.left
		anchors.right: webview.right
        anchors.margins: 20
        wrapMode: Text.WordWrap
		text: "This is an example of Android Views rendered into texture and then into FBO in Quick 2.0 scene."
    }
}
