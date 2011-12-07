import QtQuick 1.1
import QtWebKit 1.0
import "symbian"

StepsPage {
    id: loginBrowser
    lockOrientation: false

    Flickable {
        id: flickable
        width: loginBrowser.width
        contentWidth: Math.max(parent.width, webView.width)
        contentHeight: Math.max(parent.height, webView.height)
        anchors.fill: parent
        pressDelay: 0
        flickableDirection: Flickable.HorizontalAndVerticalFlick

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
            settings.localStorageDatabaseEnabled: true
            settings.offlineStorageDatabaseEnabled: true
            settings.offlineWebApplicationCacheEnabled: true
            settings.pluginsEnabled: false
            smooth: false
            preferredWidth: loginBrowser.width
            preferredHeight: loginBrowser.height
            contentsScale: 1

            onLoadFinished: {
                // Disable links
                // webView.evaluateJavaScript("for (var i = 0; i < document.links.length; i++) {l = document.links[i]; l.disabled = true; l.onclick = new Function('return false'); l.style.textDecoration = 'none'}")
            }
        }
    }

    onBack: {
        closeUrl()
    }

    function closeUrl() {
        sipFixer.enabled = false
        pageStack.pop()
    }

    function openUrl(url) {
        sipFixer.enabled = true
        gft.closeBrowser.connect(closeUrl)
        webView.url = url
    }
}
