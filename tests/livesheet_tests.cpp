#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "backend.h"
#include "theme.h"

class LivesheetTests : public QObject {
    Q_OBJECT

private slots:
    // Backend::parseDocument() should map every JSON "type" string to the
    // matching Block::Type, in the same order the lines were given, and
    // carry each block's "content" through unchanged.
    void parsesEachLineTypeFromJson()
    {
        const auto blocks = Backend::parseDocument(
            R"([{"type":"title","content":"My Title"},)"
            R"({"type":"paragraph","content":"A Paragraph"},)"
            R"({"type":"subparagraph","content":"A Subparagraph"},)"
            R"({"type":"text","content":"Some body text."},)"
            R"({"type":"math","content":"a:=2"}])");
        QCOMPARE(blocks.size(), 5);
        QCOMPARE(blocks[0].type, Backend::Block::Title);
        QCOMPARE(blocks[0].content, QString("My Title"));
        QCOMPARE(blocks[1].type, Backend::Block::Paragraph);
        QCOMPARE(blocks[2].type, Backend::Block::Subparagraph);
        QCOMPARE(blocks[3].type, Backend::Block::Text);
        QCOMPARE(blocks[4].type, Backend::Block::Math);
        QCOMPARE(blocks[4].content, QString("a:=2"));
    }

    // Headings must become LaTeX's \paragraph{} and \subparagraph{}
    // sectioning commands, not plain text.
    void latexExportUsesParagraphAndSubparagraphCommands()
    {
        QVector<Backend::Block> blocks{
            {Backend::Block::Paragraph, "P"}, {Backend::Block::Subparagraph, "S"}};
        const QString tex = Backend::blocksToLatex(blocks);
        QVERIFY(tex.contains("\\paragraph{P}"));
        QVERIFY(tex.contains("\\subparagraph{S}"));
    }

    // A Math block's content (already LaTeX from MathQuill) must be
    // wrapped in \[ \] so it renders as a centered display equation.
    void latexExportWrapsMathLinesAsDisplayEquations()
    {
        QVector<Backend::Block> blocks{{Backend::Block::Math, "a:=2"}};
        const QString tex = Backend::blocksToLatex(blocks);
        QVERIFY(tex.contains("\\[a:=2\\]"));
    }

    // An empty Text block should be dropped entirely rather than leaving
    // a blank paragraph (an extra "\n\n\n" run) in the exported .tex.
    void latexExportSkipsEmptyNonMathLines()
    {
        QVector<Backend::Block> blocks{{Backend::Block::Text, ""}, {Backend::Block::Text, "Kept"}};
        const QString tex = Backend::blocksToLatex(blocks);
        QVERIFY(!tex.contains("\n\n\n"));
        QVERIFY(tex.contains("Kept"));
    }

    // Without a colors.toml to read (a fresh temp directory), Theme
    // should settle on the documented fallback accent and a foreground
    // that's actually readable against it.
    void themeFallsBack()
    {
        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QString("black"));
    }
};

QTEST_MAIN(LivesheetTests)
#include "livesheet_tests.moc"
