#pragma once

#include <QObject>
#include <QString>
#include <QVector>

// The notebook page (notebook.html) owns the document: an ordered list of
// lines, each a Title, Paragraph or Subparagraph heading, plain Text, or a
// Math line (already given to us as LaTeX via MathQuill's .latex()). Export
// receives that list as JSON — {"type": "...", "content": "..."} per line —
// gathered in JS and handed here only when the user asks to export.
class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString homeDirectory READ homeDirectory CONSTANT)

public:
    struct Block {
        enum Type { Title, Paragraph, Subparagraph, Text, Math };
        Type type;
        QString content;
    };

    explicit Backend(QObject *parent = nullptr) : QObject(parent) {}

    // The directory QML should default export dialogs to.
    static QString homeDirectory();

    // Writes the document as a standalone .tex file. Returns false (and
    // emits exportFinished(false, ...)) only if the file couldn't be opened
    // for writing.
    Q_INVOKABLE bool exportLatex(const QString &documentJson, const QString &path);

    // Writes the .tex file next to `path` (same base name) and compiles it
    // with pdflatex. Returns false if the .tex couldn't be written or
    // pdflatex failed to produce the PDF.
    Q_INVOKABLE bool exportPdf(const QString &documentJson, const QString &path);

    // Turns the JSON array QML sent (one {"type", "content"} object per
    // notebook line) into Blocks. Any line whose type isn't recognized
    // becomes plain Text.
    static QVector<Block> parseDocument(const QString &json);

    // Renders the parsed blocks as the body of a LaTeX document: headings
    // become sectioning commands, Math blocks become centered display
    // equations, and empty non-Math lines are skipped entirely.
    static QString blocksToLatex(const QVector<Block> &blocks);

signals:
    void exportFinished(bool success, QString message);
};
