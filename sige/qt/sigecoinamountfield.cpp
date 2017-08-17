// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sigecoinamountfield.h"

#include "guiconstants.h"
#include "qvaluecombobox.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>

AmountSpinBox::AmountSpinBox(QWidget *parent) :
    QAbstractSpinBox(parent),
    currentUnit(SigecoinUnits::SGC),
    singleStep(100000) // siges
{
    setAlignment(Qt::AlignRight);
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SIGNAL(valueChanged()));
}

QValidator::State AmountSpinBox::validate(QString &text, int &pos) const
{
    if(text.isEmpty())
        return QValidator::Intermediate;
    bool valid = false;
    parse(text, &valid);
    /* Make sure we return Intermediate so that fixup() is called on defocus */
    return valid ? QValidator::Intermediate : QValidator::Invalid;
}

void AmountSpinBox::fixup(QString &input) const
{
    bool valid = false;
    CAmount val = parse(input, &valid);
    if(valid)
    {
        input = SigecoinUnits::format(currentUnit, val, false, SigecoinUnits::separatorAlways);
        lineEdit()->setText(input);
    }
}

void AmountSpinBox::setValue(const CAmount& value)
{
    lineEdit()->setText(SigecoinUnits::format(currentUnit, value, false, SigecoinUnits::separatorAlways));
    Q_EMIT valueChanged();
}

void AmountSpinBox::stepBy(int steps)
{
    bool valid = false;
    CAmount val = value(&valid);
    val = val + steps * singleStep;
    val = qMin(qMax(val, CAmount(0)), SigecoinUnits::maxMoney());
    setValue(val);
}

void AmountSpinBox::setDisplayUnit(int unit)
{
    bool valid = false;
    CAmount val = value(&valid);

    currentUnit = unit;

    if(valid)
        setValue(val);
    else
        clear();
}

QSize AmountSpinBox::minimumSizeHint() const
{
    if(cachedMinimumSizeHint.isEmpty())
    {
        ensurePolished();

        const QFontMetrics fm(fontMetrics());
        int h = lineEdit()->minimumSizeHint().height();
        int w = fm.width(SigecoinUnits::format(SigecoinUnits::SGC, SigecoinUnits::maxMoney(), false, SigecoinUnits::separatorAlways));
        w += 2; // cursor blinking space

        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        QSize hint(w, h);
        QSize extra(35, 6);
        opt.rect.setSize(hint + extra);
        extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                QStyle::SC_SpinBoxEditField, this).size();
        // get closer to final result by repeating the calculation
        opt.rect.setSize(hint + extra);
        extra += hint - style()->subControlRect(QStyle::CC_SpinBox, &opt,
                                                QStyle::SC_SpinBoxEditField, this).size();
        hint += extra;
        hint.setHeight(h);

        opt.rect = rect();

        cachedMinimumSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this)
                                .expandedTo(QApplication::globalStrut());
    }
    return cachedMinimumSizeHint;
}

CAmount AmountSpinBox::parse(const QString &text, bool *valid_out) const
{
    CAmount val = 0;
    bool valid = SigecoinUnits::parse(currentUnit, text, &val);
    if(valid)
    {
        if(val < 0 || val > SigecoinUnits::maxMoney())
            valid = false;
    }
    if(valid_out)
        *valid_out = valid;
    return valid ? val : 0;
}

bool AmountSpinBox::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Comma)
        {
            // Translate a comma into a period
            QKeyEvent periodKeyEvent(event->type(), Qt::Key_Period, keyEvent->modifiers(), ".", keyEvent->isAutoRepeat(), keyEvent->count());
            return QAbstractSpinBox::event(&periodKeyEvent);
        }
    }
    return QAbstractSpinBox::event(event);
}

AmountSpinBox::StepEnabled AmountSpinBox::stepEnabled() const
{
    if (isReadOnly()) // Disable steps when AmountSpinBox is read-only
        return StepNone;
    if (text().isEmpty()) // Allow step-up with empty field
        return StepUpEnabled;

    StepEnabled rv = 0;
    bool valid = false;
    CAmount val = value(&valid);
    if(valid)
    {
        if(val > 0)
            rv |= StepDownEnabled;
        if(val < SigecoinUnits::maxMoney())
            rv |= StepUpEnabled;
    }
    return rv;
}


SigecoinAmountField::SigecoinAmountField(QWidget *parent) :
    QWidget(parent),
    amount(0)
{
    amount = new AmountSpinBox(this);
    amount->setLocale(QLocale::c());
    amount->installEventFilter(this);
    amount->setMaximumWidth(170);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(amount);
    unit = new QValueComboBox(this);
    unit->setModel(new SigecoinUnits(this));
    layout->addWidget(unit);
    layout->addStretch(1);
    layout->setContentsMargins(0,0,0,0);

    setLayout(layout);

    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(amount);

    // If one if the widgets changes, the combined content changes as well
    connect(amount, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    connect(unit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)));

    // Set default based on configuration
    unitChanged(unit->currentIndex());
}

void SigecoinAmountField::clear()
{
    amount->clear();
    unit->setCurrentIndex(0);
}

void SigecoinAmountField::setEnabled(bool fEnabled)
{
    amount->setEnabled(fEnabled);
    unit->setEnabled(fEnabled);
}

bool SigecoinAmountField::validate()
{
    bool valid = false;
    value(&valid);
    setValid(valid);
    return valid;
}

void SigecoinAmountField::setValid(bool valid)
{
    if (valid)
        amount->setStyleSheet("");
    else
        amount->setStyleSheet(STYLE_INVALID);
}

bool SigecoinAmountField::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        // Clear invalid flag on focus
        setValid(true);
    }
    return QWidget::eventFilter(object, event);
}

QWidget *SigecoinAmountField::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, amount);
    QWidget::setTabOrder(amount, unit);
    return unit;
}

CAmount SigecoinAmountField::value(bool *valid_out) const
{
    return amount->value(valid_out);
}

void SigecoinAmountField::setValue(const CAmount& value)
{
    amount->setValue(value);
}

void SigecoinAmountField::setReadOnly(bool fReadOnly)
{
    amount->setReadOnly(fReadOnly);
}

void SigecoinAmountField::unitChanged(int idx)
{
    // Use description tooltip for current unit for the combobox
    unit->setToolTip(unit->itemData(idx, Qt::ToolTipRole).toString());

    // Determine new unit ID
    int newUnit = unit->itemData(idx, SigecoinUnits::UnitRole).toInt();

    amount->setDisplayUnit(newUnit);
}

void SigecoinAmountField::setDisplayUnit(int newUnit)
{
    unit->setValue(newUnit);
}

void SigecoinAmountField::setSingleStep(const CAmount& step)
{
    amount->setSingleStep(step);
}
