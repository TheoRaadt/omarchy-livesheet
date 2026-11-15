import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtWebEngine

// The whole window is really just one WebEngineView (`preview`, below)
// hosting src/notebook.html, which owns all typing and rendering. This
// file's only other job is the small native Export dialog it pops up.
ApplicationWindow {
    id: win
    width: 1000
    height: 760
    minimumWidth: 480
    minimumHeight: 360
    visible: true
    title: "livesheet"
    color: "#0e0e10"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    // Opened by the page itself via the Ctrl+S / "livesheet:export"
    // navigation trick below. Both buttons ask the page for the whole
    // document as JSON (via collectDocument(), defined in notebook.html)
    // and hand it to the matching Backend method.
    Dialog {
        id: exportDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Export")
        standardButtons: Dialog.Close
        width: 420

        // Give keyboard focus to the path field on open, and back to the
        // notebook page on close, so typing never has to find its way
        // back there manually.
        onOpened: exportPath.forceActiveFocus()
        onClosed: preview.forceActiveFocus()

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            TextField {
                id: exportPath
                Layout.fillWidth: true
                placeholderText: qsTr("/path/to/document.pdf")
                text: backend.homeDirectory + "/document.pdf"
            }
            RowLayout {
                spacing: 8
                Button {
                    text: qsTr("Export .tex")
                    onClicked: preview.runJavaScript("JSON.stringify(collectDocument())", function (json) {
                        backend.exportLatex(json, exportPath.text.replace(/\.pdf$/, ".tex"))
                    })
                }
                Button {
                    text: qsTr("Export .pdf")
                    highlighted: true
                    onClicked: preview.runJavaScript("JSON.stringify(collectDocument())", function (json) {
                        backend.exportPdf(json, exportPath.text)
                    })
                }
            }
            Label {
                id: exportStatus
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: "#9aa"
            }
        }
    }

    // Backend.exportLatex()/exportPdf() both emit this when they're done
    // (success or failure); shows the result under the export buttons.
    Connections {
        target: backend
        function onExportFinished(success, message) {
            exportStatus.text = success ? qsTr("Saved to %1").arg(message) : qsTr("Failed: %1").arg(message)
            exportStatus.color = success ? "#8f8" : "#f88"
        }
    }

    // The app's entire surface: the notebook page fills the window.
    WebEngineView {
        id: preview
        anchors.fill: parent
        url: "qrc:/notebook.html"

        // The page routes Ctrl+S and Ctrl+Q through a fake navigation to
        // "livesheet:..." because it owns every keystroke (it's the only
        // input surface), so plain QML Shortcuts would never see them.
        // request.reject() cancels the fake navigation itself — the page
        // never actually goes anywhere.
        onNavigationRequested: function (request) {
            const target = request.url.toString()
            if (target === "livesheet:export") {
                request.reject()
                exportDialog.open()
            } else if (target === "livesheet:quit") {
                request.reject()
                Qt.quit()
            }
        }

        // Without this, the window can open with keyboard focus
        // nowhere in particular, so the first few keystrokes get lost.
        onLoadingChanged: function (info) {
            if (info.status === WebEngineView.LoadSucceededStatus)
                preview.forceActiveFocus()
        }
    }
}
