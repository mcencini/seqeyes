#include "LogTableDialog.h"

#include <QAbstractTableModel>
#include <QFontDatabase>
#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>
#include <QVBoxLayout>

class LogTableModel final : public QAbstractTableModel
{
public:
    enum Col
    {
        Time = 0,
        Level,
        Category,
        Message,
        Origin,
        ColCount
    };

    explicit LogTableModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return m_entries.size();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) return 0;
        return ColCount;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role != Qt::DisplayRole) return {};
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
                case Time:     return QStringLiteral("Time");
                case Level:    return QStringLiteral("Level");
                case Category: return QStringLiteral("Category");
                case Message:  return QStringLiteral("Message");
                case Origin:   return QStringLiteral("Origin");
                default:       return {};
            }
        }
        return section + 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid()) return {};
        const int r = index.row();
        const int c = index.column();
        if (r < 0 || r >= m_entries.size()) return {};
        if (c < 0 || c >= ColCount) return {};

        const auto& e = m_entries[r];
        if (role == Qt::DisplayRole)
        {
            switch (c)
            {
                case Time:     return e.timestamp;
                case Level:    return e.level;
                case Category: return e.category;
                case Message:  return e.message;
                case Origin:   return e.origin;
                default:       return {};
            }
        }
        return {};
    }

    void setEntries(const QVector<LogManager::LogEntry>& entries)
    {
        beginResetModel();
        m_entries = entries;
        endResetModel();
    }

    void appendEntry(const LogManager::LogEntry& entry)
    {
        const int r = m_entries.size();
        beginInsertRows(QModelIndex(), r, r);
        m_entries.append(entry);
        endInsertRows();
    }

private:
    QVector<LogManager::LogEntry> m_entries;
};

LogTableDialog::LogTableDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Log");
    setWindowModality(Qt::NonModal);
    resize(1100, 520);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    m_model = new LogTableModel(this);
    m_view = new QTableView(this);
    m_view->setModel(m_model);

    // Scrollbar styling (white track + gray handle) for better visibility.
    m_view->setStyleSheet(QStringLiteral(R"(
QTableView {
  background: white;
}
QScrollBar:vertical, QScrollBar:horizontal {
  background: #ffffff;
  border: 1px solid #d0d0d0;
}
QScrollBar:vertical {
  width: 14px;
  margin: 0px;
}
QScrollBar:horizontal {
  height: 14px;
  margin: 0px;
}
QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
  background: #a9a9a9;
  border: 1px solid #8f8f8f;
  border-radius: 6px;
  min-height: 24px;
  min-width: 24px;
}
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
  background: #8f8f8f;
}
QScrollBar::add-line, QScrollBar::sub-line {
  background: transparent;
  border: none;
  width: 0px;
  height: 0px;
}
QScrollBar::add-page, QScrollBar::sub-page {
  background: #ffffff;
}
)"));

    // Monospace font for logs.
    {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setStyleHint(QFont::Monospace);
        m_view->setFont(f);
    }

    m_view->setWordWrap(false);
    m_view->setTextElideMode(Qt::ElideNone);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_view->setAlternatingRowColors(true);
    m_view->setSortingEnabled(false);
    m_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // User-resizable columns (Excel-like).
    QHeaderView* hdr = m_view->horizontalHeader();
    hdr->setSectionsMovable(true);
    hdr->setStretchLastSection(true);
    hdr->setSectionResizeMode(QHeaderView::Interactive);

    // Reasonable defaults.
    m_view->setColumnWidth(LogTableModel::Time, 180);
    m_view->setColumnWidth(LogTableModel::Level, 70);
    m_view->setColumnWidth(LogTableModel::Category, 140);
    m_view->setColumnWidth(LogTableModel::Origin, 140);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_view);
}

bool LogTableDialog::isNearBottom() const
{
    if (!m_view) return true;
    auto* v = m_view->verticalScrollBar();
    if (!v) return true;
    return v->value() >= v->maximum() - 2;
}

void LogTableDialog::scrollToBottomIfNeeded(bool followBottom)
{
    if (!followBottom || !m_view) return;
    if (auto* v = m_view->verticalScrollBar())
        v->setValue(v->maximum());
}

void LogTableDialog::setInitialContent(const QVector<LogManager::LogEntry>& entries)
{
    const bool followBottom = true;
    m_model->setEntries(entries);
    scrollToBottomIfNeeded(followBottom);
}

void LogTableDialog::appendEntry(const LogManager::LogEntry& entry)
{
    const bool followBottom = isNearBottom();
    m_model->appendEntry(entry);
    scrollToBottomIfNeeded(followBottom);
}

