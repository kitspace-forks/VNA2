#include "siunitedit.h"

#include <QDoubleValidator>
#include <unit.h>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

SIUnitEdit::SIUnitEdit(QString unit, QString prefixes, int precision, QWidget *parent)
    : QLineEdit(parent)
{
    _value = 0;
    this->unit = unit;
    this->prefixes = prefixes;
    this->precision = precision;
    setAlignment(Qt::AlignCenter);
    installEventFilter(this);
    setValidator(new QDoubleValidator(this));
    connect(this, &QLineEdit::editingFinished, [this]() {
       parseNewValue(1.0);
    });
}

SIUnitEdit::SIUnitEdit(QWidget *parent)
    : SIUnitEdit("", " ", 4, parent)
{

}

void SIUnitEdit::setValue(double value)
{
    if(value != _value) {
        setValueQuiet(value);
        emit valueChanged(value);
        emit valueUpdated(this);
    }
}

static char swapUpperLower(char c) {
    if(isupper(c)) {
        return tolower(c);
    } else if(islower(c)) {
        return toupper(c);
    } else {
        return c;
    }
}

bool SIUnitEdit::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent *>(event)->key();
        if(key == Qt::Key_Escape) {
            // abort editing process and set old value
            setValueQuiet(_value);
            emit editingAborted();
            clearFocus();
            return true;
        }
        if(key == Qt::Key_Return) {
            // use new value without prefix
           parseNewValue(1.0);
           continueEditing();
           return true;
        }
        auto mod = static_cast<QKeyEvent *>(event)->modifiers();
        if (!(mod & Qt::ShiftModifier)) {
            key = tolower(key);
        }
        if(key <= 255) {
            if (prefixes.indexOf(key) >= 0) {
                // a valid prefix key was pressed
                parseNewValue(Unit::SIPrefixToFactor(key));
                continueEditing();
                return true;
            } else if (prefixes.indexOf(swapUpperLower(key)) >= 0) {
                // no match on the pressed case but on the upper/lower case instead -> also accept this
                parseNewValue(Unit::SIPrefixToFactor(swapUpperLower(key)));
                continueEditing();
                return true;
            }
        }
    } else if(event->type() == QEvent::FocusOut) {
        if(!text().isEmpty()) {
            parseNewValue(1.0);
        } else {
            setValueQuiet(_value);
            emit editingAborted();
        }
    }
    return false;
}

void SIUnitEdit::setValueQuiet(double value)
{
    _value = value;
    clear();
    setPlaceholderText(Unit::ToString(value, unit, prefixes, precision));
}

void SIUnitEdit::parseNewValue(double factor)
{
    QString input = text();
    // remove optional unit
    if(input.endsWith(unit)) {
        input.chop(unit.size());
    }
    auto lastChar = input.at(input.size()-1).toLatin1();
    if(prefixes.indexOf(lastChar) >= 0) {
        factor = Unit::SIPrefixToFactor(lastChar);
        input.chop(1);
    }
    // remaining input should only contain numbers
    bool conversion_ok;
    auto v = input.toDouble(&conversion_ok);
    if(conversion_ok) {
        qDebug() << v;
        setValue(v * factor);
    } else {
        qWarning() << "SIUnit conversion failure:" << input;
    }
    clear();
}

void SIUnitEdit::continueEditing()
{
    setText(placeholderText());
    selectAll();
}
