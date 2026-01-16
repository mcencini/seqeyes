#include <QApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "PulseqLoader.h"
#include "WaveformDrawer.h"
#include "InteractionHandler.h"

static int runOne(const QString& seqPath)
{
    MainWindow w;
    PulseqLoader* loader = w.getPulseqLoader();
    WaveformDrawer* drawer = w.getWaveformDrawer();
    InteractionHandler* ih = w.getInteractionHandler();
    if (!loader || !drawer || !ih) {
        std::cerr << "Init failure" << std::endl;
        return 2;
    }

    if (!loader->LoadPulseqFile(seqPath)) {
        std::cerr << "Load failed: " << seqPath.toStdString() << std::endl;
        return 3;
    }

    // Initialize view
    drawer->ResetView();

    // Baseline viewport
    QCPRange r = w.ui->customPlot->xAxis->range();
    double center = 0.5 * (r.lower + r.upper);
    double width = r.size();

    // Prepare a zoom-in by 50%
    double newWidth = width * 0.5;
    QCPRange newRange(center - newWidth/2.0, center + newWidth/2.0);

    // Measure end-to-end time of a programmatic zoom using direct axis range update
    // (avoids potential crashes inside complex interaction sync paths)
    QElapsedTimer t; t.start();
    w.ui->customPlot->xAxis->setRange(newRange);
    w.ui->customPlot->replot(QCustomPlot::rpQueuedReplot);
    qApp->processEvents();
    qint64 ms = t.elapsed();

    QTextStream(stdout) << "ZOOM_MS: " << ms << "\n";

    return 0;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QString seqPath;
    for (int i = 1; i < argc; ++i) {
        QString a = argv[i];
        if (a == "--seq" && i+1 < argc) { seqPath = argv[++i]; }
    }
    if (seqPath.isEmpty()) {
        // 1) Prefer build-time provided test source dir
#ifdef QT_TESTCASE_SOURCEDIR
        QString sourceDir = QString::fromUtf8(QT_TESTCASE_SOURCEDIR);
        QStringList candidates = {
            sourceDir + "/seq_files/writeCineGradientEcho.seq",
            sourceDir + "/seq_files/spi.seq",
            sourceDir + "/seq_files/epi.seq",
            sourceDir + "/seq_files/spi_sub.seq",
            sourceDir + "/seq_files/write_QA_Sag_Localizer.seq"
        };
        for (const auto& c : candidates) {
            if (QFileInfo::exists(c)) { seqPath = c; break; }
        }
#endif
        // 2) Fallback: search upwards from binary dir for test/seq_files
        if (seqPath.isEmpty()) {
            QDir d(QCoreApplication::applicationDirPath());
            for (int up = 0; up < 6 && seqPath.isEmpty(); ++up) {
                QString base = d.absolutePath() + "/test/seq_files";
                QStringList cs = {
                    base + "/writeCineGradientEcho.seq",
                    base + "/spi.seq",
                    base + "/epi.seq",
                    base + "/spi_sub.seq",
                    base + "/write_QA_Sag_Localizer.seq"
                };
                for (const auto& c : cs) { if (QFileInfo::exists(c)) { seqPath = c; break; } }
                d.cdUp();
            }
        }
        // 3) Last resort: previous relative path attempt
        if (seqPath.isEmpty()) {
            QString bin = QCoreApplication::applicationDirPath();
            QString try1 = bin + "/../../test/seq_files/writeCineGradientEcho.seq";
            QString try2 = bin + "/../../test/seq_files/spi.seq";
            if (QFileInfo::exists(try1)) seqPath = try1; else if (QFileInfo::exists(try2)) seqPath = try2;
        }
    }
    if (seqPath.isEmpty() || !QFileInfo::exists(seqPath)) {
        std::cerr << "Could not locate any default .seq file. Use --seq <path>." << std::endl;
        return 4;
    }
    return runOne(seqPath);
}

