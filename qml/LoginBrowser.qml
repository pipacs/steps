import QtQuick 1.1
import QtWebKit 1.0
import "symbian"

StepsPage {
    id: loginBrowser
    lockOrientation: false
    showBack: true

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
            settings.defaultFontSize: 28
            settings.minimumFontSize: 26
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
                // Enable fix for the software input panel
                sipFixer.enabled = true
            }
        }
    }

    onBack: {
        closeUrl()
    }

    function closeUrl() {
        sipFixer.enabled = false
        googleDocs.linkingSucceeded.disconnect(closeUrl)
        googleDocs.linkingFailed.disconnect(closeUrl)
        pageStack.pop()
    }

    function openUrl(url) {
        googleDocs.linkingSucceeded.connect(closeUrl)
        googleDocs.linkingFailed.connect(closeUrl)
        webView.url = url
    }
}
