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
            settings.defaultFontSize: (platform.osName === "symbian")? 22: 28
            settings.minimumFontSize: (platform.osName === "symbian")? 20: 26
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
        }
    }

    onBack: {
        closeUrl()
    }

    function closeUrl() {
        console.log("* LoginBrowser.closeUrl")
        gft.closeBrowser.disconnect(closeUrl)
        sipFixer.enabled = false
        pageStack.pop()
    }

    function openUrl(url) {
        console.log("* LoginBrowser.openUrl")
        sipFixer.enabled = true
        gft.closeBrowser.connect(closeUrl)
        webView.url = url
    }
}
