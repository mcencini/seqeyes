#pragma once

#include <QDialog>

class QTableWidget;
class PulseqLoader;

/**
 * @brief Legend window for extension markers/colors.
 *
 * This is a separate window to avoid cluttering the main UI and to match Matlab's legend concept.
 */
class ExtensionLegendDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExtensionLegendDialog(QWidget* parent = nullptr);
    void refresh(PulseqLoader* loader);

private:
    QTableWidget* m_table {nullptr};
};

