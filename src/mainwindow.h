#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include "external/qcustomplot/qcustomplot.h"
#include "Settings.h"

// Forward declarations
namespace Ui { class MainWindow; }
class InteractionHandler;
class PulseqLoader;
class TRManager;
class WaveformDrawer;
class SettingsDialog;
class QProgressBar;
class QLabel;
class QSplitter;
class QCustomPlot;
class QCPGraph;
class QCPCurve;
class QHBoxLayout;
class QPushButton;
class QWidget;
class QCheckBox;
class QComboBox;
class QDragEnterEvent;
class QDropEvent;
class QWheelEvent;
class QEvent;
class QMouseEvent;
class QCommandLineParser;

#define SAFE_DELETE(p) { if(p) { delete p; p = nullptr; } }

// This dialog is primarily used by InteractionHandler.
// Ideally, it would be in its own file (e.g., eventblockinfodialog.h).
// It is kept here to minimize structural changes to the project file list.
class EventBlockInfoDialog : public QDialog
{
public:
    explicit EventBlockInfoDialog(QWidget* parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Event Block");
        setWindowModality(Qt::NonModal);
        resize(600, 200);

        textEdit = new QPlainTextEdit(this);
        textEdit->setReadOnly(true);
        setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
        textEdit->setWordWrapMode(QTextOption::NoWrap);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->addWidget(textEdit);
    }
    void setInfoContent(QString content)
    {
        textEdit->setPlainText(content);
    }
private:
    QPlainTextEdit* textEdit;
};


class LogDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Grant handlers access to private members like 'ui' and other handlers
    friend class InteractionHandler;
    friend class PulseqLoader;
    friend class TRManager;
    friend class WaveformDrawer;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Getters for handlers
    InteractionHandler* getInteractionHandler() const { return m_interactionHandler; }
    PulseqLoader* getPulseqLoader() const { return m_pulseqLoader; }
    TRManager* getTRManager() const { return m_trManager; }
    WaveformDrawer* getWaveformDrawer() const { return m_waveformDrawer; }
    bool isTrajectoryVisible() const { return m_showTrajectory; }

    // Getters for UI elements needed by handlers
    Ui::MainWindow* getUI() const { return ui; }
    // Expose UI for tests that access w.ui directly
    Ui::MainWindow* ui;
    QLabel* getCoordLabel() const { return m_pCoordLabel; }
    QLabel* getVersionLabel() const { return m_pVersionLabel; }
    QProgressBar* getProgressBar() const { return m_pProgressBar; }

protected:
    // Overridden event handlers to delegate to InteractionHandler
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void Init();
    void setupSettingsMenu();
    void setupPlotArea(QVBoxLayout* mainLayout);
    void refreshTrajectoryPlotData();
    void enforceTrajectoryAspect(bool queueReplot);
    void onPlotSplitterMoved(int pos, int index);
    void scheduleTrajectoryAspectUpdate();
    void updateTrajectoryExportState();
    void refreshTrajectoryCursor();
    void updateTrajectoryAxisLabels();
    bool sampleTrajectoryPosition(double timeSec,
                                  double& kxOut,
                                  double& kyOut,
                                  double& kzOut) const;
    bool writeTrajectoryFile(const QString& path,
                             const QVector<double>& kx,
                             const QVector<double>& ky,
                             const QVector<double>& kz);

private slots:
    void openSettings();
    void openLogWindow();
    void showAbout();
    void showUsage();
    void InitSlots();
    void InitStatusBar();
    void onShowFullDetailToggled(bool checked);
    void exportTrajectory();
    void onTrajectoryWheel(QWheelEvent* event);
    void onShowTrajectoryCursorToggled(bool checked);
    void onTrajectoryRangeModeChanged(int index);
    void onTrajectorySeriesToggled();
    void onResetTrajectoryRange();
    void onTrajectoryMouseMove(QMouseEvent* event);
    void onTrajectoryCrosshairToggled(bool checked);
    void onTimeUnitChanged();

public:
    void openFileFromCommandLine(const QString& filePath);
    void applyCommandLineOptions(const QCommandLineParser& parser);
    void setTrajectoryVisible(bool show);
    bool sampleTrajectoryAtInternalTime(double internalTime,
                                        double& kxOut,
                                        double& kyOut,
                                        double& kzOut) const;
    void updateTrajectoryCursorTime(double internalTime);
    void onSettingsChanged();

    // Window title helpers
    void setLoadedFileTitle(const QString& filePath);
    void clearLoadedFileTitle();

private:
    

    // Handlers for different functionalities
    InteractionHandler* m_interactionHandler;
    PulseqLoader*       m_pulseqLoader;
    TRManager*          m_trManager;
    WaveformDrawer*     m_waveformDrawer;

    // UI elements managed directly by MainWindow (e.g., status bar)
    QLabel* m_pVersionLabel;
    QProgressBar* m_pProgressBar;
    QLabel* m_pCoordLabel; // Used by InteractionHandler and TRManager
    
    // Settings dialog
    SettingsDialog* m_settingsDialog;

    // Track last applied trajectory unit so we can recompute default ranges when it changes
    Settings::TrajectoryUnit m_lastTrajectoryUnit { Settings::TrajectoryUnit::PerM };

    // Plot area layout helpers
    QSplitter* m_plotSplitter {nullptr};
    QWidget* m_pTrajectoryPanel {nullptr};
    QCustomPlot* m_pTrajectoryPlot {nullptr};
    QCPCurve* m_pTrajectoryCurve {nullptr};
    QCPGraph* m_pTrajectorySamplesGraph {nullptr};
    // Colorized ADC scatter (current window)
    QVector<QCPGraph*> m_trajColorGraphs;
    QPushButton* m_pExportTrajectoryButton {nullptr};
    QPushButton* m_pResetTrajectoryButton {nullptr};
    QCheckBox* m_pShowTrajectoryCursorCheckBox {nullptr};
    QComboBox* m_pTrajectoryRangeCombo {nullptr};
    QCheckBox* m_pShowKtrajCheckBox {nullptr};
    QCheckBox* m_pShowKtrajAdcCheckBox {nullptr};
    QCheckBox* m_pTrajectoryCrosshairCheckBox {nullptr};
    QLabel* m_pTrajectoryCrosshairLabel {nullptr};
    QWidget* m_pTrajectoryCursorMarker {nullptr};
    QWidget* m_pTrajectoryCrosshairOverlay {nullptr};
    bool m_showTrajectory {false};
    bool m_pendingTrajectoryAspectUpdate {false};
    QCPRange m_trajectoryBaseXRange {0.0, 1.0};
    QCPRange m_trajectoryBaseYRange {0.0, 1.0};
    bool m_inTrajectoryRangeAdjust {false};
    bool m_showTrajectoryCursor {true};
    bool m_showWholeTrajectory {false};
    bool m_colorCurrentWindow {false};
    bool m_showKtraj {false};
    bool m_showKtrajAdc {true};
    bool m_showTrajectoryCrosshair {false};
    double m_currentTrajectoryTimeInternal {0.0};
    bool m_hasTrajectoryCursorTime {false};
    bool m_trajectoryRangeInitialized {false};

    QString m_loadedSeqFilePath;
};

#endif // MAINWINDOW_H
