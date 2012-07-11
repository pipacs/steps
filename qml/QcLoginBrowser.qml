import QtQuick 1.1
import QtWebKit 1.0
import "meego"

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
            Component.onCompleted: {
                if (platform.osName === "symbian") {
                    settings.defaultFontSize = 26
                    settings.minimumFontSize = 24
                } else {
                    settings.defaultFontSize = 28
                    settings.minimumFontSize = 26
                }
            }
            onLoadFinished: {
                // Work around Symbian WebView bug
                if (platform.osName === "symbian") {
                    evaluateJavaScript("if (!document.body.style.backgroundColor) document.body.style.backgroundColor='white';");
                }
            }
        }
    }

    onBack: {
        closeUrl()
    }

    function closeUrl() {
        qc.closeBrowser.disconnect(closeUrl)
        sipFixer.enabled = false
        pageStack.pop()
    }

    function openUrl(url) {
        sipFixer.enabled = true
        qc.closeBrowser.connect(closeUrl)
        webView.url = url
    }
}
