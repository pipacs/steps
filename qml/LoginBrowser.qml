import QtQuick 1.1
import QtWebKit 1.0
import "meego"

StepsPage {
    property alias url: webView.url
    id: loginBrowser

    TextEdit {
        id: vkbCloser
        anchors.top: parent.bottom
    }

    Flickable {
        id: flickable
        width: loginBrowser.width
        contentWidth: Math.max(parent.width, webView.width)
        contentHeight: Math.max(parent.height, webView.height)
        anchors.fill: parent
        pressDelay: 0
        flickableDirection: Flickable.VerticalFlick

        WebView {
            id: webView
            transformOrigin: Item.TopLeft
            pressGrabTime: 9999
            focus: true
            settings.defaultFontSize: 26
            settings.minimumFontSize: 22
            settings.javaEnabled: false
            settings.javascriptCanAccessClipboard: false
            settings.javascriptCanOpenWindows: false
            settings.javascriptEnabled: true
            settings.linksIncludedInFocusChain: false
            settings.localContentCanAccessRemoteUrls: false
            settings.localStorageDatabaseEnabled: false
            settings.offlineStorageDatabaseEnabled: false
            settings.offlineWebApplicationCacheEnabled: false
            settings.pluginsEnabled: false
            smooth: false
            preferredWidth: loginBrowser.width
            preferredHeight: loginBrowser.height
            contentsScale: 1

            onLoadFinished: {
                // Disable links
                // webView.evaluateJavaScript("for (var i = 0; i < document.links.length; i++) {l = document.links[i]; l.disabled = true; l.onclick = new Function('return false'); l.style.textDecoration = 'none'}")
            }

            function onLinkingSucceeded() {
                console.log("* LoginBrowser.onLinkingSucceeded")
                vkbCloser.closeSoftwareInputPanel()
                appWindow.pageStack.pop()
            }

            function onLinkingFailed(error) {
                console.log("* LoginBrowser.onLinkingFailed " + error)
                vkbCloser.closeSoftwareInputPanel()
                appWindow.pageStack.pop()
            }

            Component.onCompleted: {
                googleDocs.linkingSucceeded.connect(onLinkingSucceeded)
                googleDocs.linkingFailed.connect(onLinkingFailed)
                loadFinished.connect(flickable.loadFinished)
            }
        }
    }
}
