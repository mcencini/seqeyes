#ifndef NUMERICLINEEDIT_H
#define NUMERICLINEEDIT_H

#include <QLineEdit>
#include <QObject>

// A small QLineEdit subclass optimized for numeric entry:
// - Does not trigger heavy updates until editingFinished/returnPressed is emitted (handled by caller).
// - Selects all text on focus-in and on double-click to make editing faster.
class NumericLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit NumericLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {}

protected:
    void focusInEvent(QFocusEvent* e) override
    {
        QLineEdit::focusInEvent(e);
        // Select all to make replacing value quicker
        selectAll();
    }

    void mouseDoubleClickEvent(QMouseEvent* e) override
    {
        QLineEdit::mouseDoubleClickEvent(e);
        // Always select full numeric content on double-click
        selectAll();
    }
};

#endif // NUMERICLINEEDIT_H
