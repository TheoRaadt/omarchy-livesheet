#include "backend.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTextStream>

// Used by Main.qml to pre-fill the export dialog's path field.
QString Backend::homeDirectory() { return QDir::homePath(); }

// Reads the JSON array notebook.html's collectDocument() produced and
// turns each {"type", "content"} object into a Block. Walks the lines in
// document order, which is also the order they'll appear in the exported
// .tex file.
QVector<Backend::Block> Backend::parseDocument(const QString &json) {
    QVector<Block> blocks;
    const auto array = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const auto &value : array) {
        const auto object = value.toObject();
        const QString type = object.value("type").toString();
        const QString content = object.value("content").toString();
        // Unrecognized or missing "type" values fall back to plain Text,
        // so a malformed entry degrades gracefully instead of being lost.
        Block::Type blockType = Block::Text;
        if (type == "title") blockType = Block::Title;
        else if (type == "paragraph") blockType = Block::Paragraph;
        else if (type == "subparagraph") blockType = Block::Subparagraph;
        else if (type == "math") blockType = Block::Math;
        blocks.append({blockType, content});
    }
    return blocks;
}

// Converts each block to the LaTeX command for its line type: headings
// become \paragraph{}/\subparagraph{}/a centered bold line, plain text
// passes through unchanged, and math (including a matrix's bmatrix
// environment) becomes a centered display equation. Empty lines are
// dropped so they don't leave stray blank paragraphs in the output,
// except an empty Math line, which can't happen in practice (an empty
// MathQuill field never makes it into the exported document) but is kept
// explicit here rather than relying on that.
QString Backend::blocksToLatex(const QVector<Block> &blocks) {
    QString tex;
    for (const auto &block : blocks) {
        if (block.content.trimmed().isEmpty() && block.type != Block::Math)
            continue;
        switch (block.type) {
        case Block::Title:
            tex += "{\\centering\\Large\\bfseries " + block.content + "\\par}\n\n";
            break;
        case Block::Paragraph:
            tex += "\\paragraph{" + block.content + "}\n\n";
            break;
        case Block::Subparagraph:
            tex += "\\subparagraph{" + block.content + "}\n\n";
            break;
        case Block::Text:
            tex += block.content + "\n\n";
            break;
        case Block::Math:
            if (!block.content.trimmed().isEmpty())
                tex += "\\[" + block.content + "\\]\n\n";
            break;
        }
    }
    return tex;
}

// Wraps the document's blocks in a minimal article preamble and writes it
// straight to `path`. This is the plain "save as .tex" action; exportPdf()
// below does the same write and then compiles it.
bool Backend::exportLatex(const QString &documentJson, const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, "Could not write " + path);
        return false;
    }
    QTextStream out(&file);
    out << "\\documentclass{article}\n"
        << "\\usepackage{amsmath}\n"
        << "\\begin{document}\n\n"
        << blocksToLatex(parseDocument(documentJson))
        << "\\end{document}\n";
    emit exportFinished(true, path);
    return true;
}

// Writes the same .tex source as exportLatex() (to a file with the same
// base name as the requested PDF, in the same directory) and then shells
// out to pdflatex to compile it. Blocks the calling thread for up to 15
// seconds waiting for pdflatex; that's acceptable here since the UI only
// calls this from a one-off "Export" button click, not during typing.
bool Backend::exportPdf(const QString &documentJson, const QString &path) {
    const QFileInfo info(path);
    const QString texPath = info.absolutePath() + "/" + info.completeBaseName() + ".tex";

    QFile file(texPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, "Could not write " + texPath);
        return false;
    }
    QTextStream out(&file);
    out << "\\documentclass{article}\n"
        << "\\usepackage{amsmath}\n"
        << "\\begin{document}\n\n"
        << blocksToLatex(parseDocument(documentJson))
        << "\\end{document}\n";
    file.close();

    QProcess process;
    process.start("pdflatex", {"-interaction=nonstopmode", "-output-directory", info.absolutePath(), texPath});
    if (!process.waitForFinished(15000) || process.exitCode() != 0) {
        emit exportFinished(false, "pdflatex failed; is it installed?");
        return false;
    }

    // pdflatex can exit 0 even when it skipped writing the PDF (e.g. a
    // LaTeX error it chose to warn about rather than fail on under
    // -interaction=nonstopmode), so the real success check is simply
    // whether the file is there afterward.
    const bool ok = QFileInfo::exists(path);
    emit exportFinished(ok, ok ? path : "pdflatex did not produce " + path);
    return ok;
}
