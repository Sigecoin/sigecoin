// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIGE_QT_SIGECOINAMOUNTFIELD_H
#define SIGE_QT_SIGECOINAMOUNTFIELD_H

#include "amount.h"
#include "sigecoinunits.h"

#include <QWidget>
#include <QAbstractSpinBox>

class AmountSpinBox;

QT_BEGIN_NAMESPACE
class QValueComboBox;
QT_END_NAMESPACE

/** Widget for entering sigecoin amounts.
  */
class SigecoinAmountField: public QWidget
{
    Q_OBJECT

    // ugly hack: for some unknown reason CAmount (instead of qint64) does not work here as expected
    Q_PROPERTY(qint64 value READ value WRITE setValue NOTIFY valueChanged USER true)

public:
    explicit SigecoinAmountField(QWidget *parent = 0);

    CAmount value(bool *value=0) const;
    void setValue(const CAmount& value);

    /** Set single step in siges **/
    void setSingleStep(const CAmount& step);

    /** Make read-only **/
    void setReadOnly(bool fReadOnly);

    /** Mark current value as invalid in UI. */
    void setValid(bool valid);
    /** Perform input validation, mark field as invalid if entered value is not valid. */
    bool validate();

    /** Change unit used to display amount. */
    void setDisplayUnit(int unit);

    /** Make field empty and ready for new input. */
    void clear();

    /** Enable/Disable. */
    void setEnabled(bool fEnabled);

    /** Qt messes up the tab chain by default in some cases (issue https://bugreports.qt-project.org/browse/QTBUG-10907),
        in these cases we have to set it up manually.
    */
    QWidget *setupTabChain(QWidget *prev);

Q_SIGNALS:
    void valueChanged();

protected:
    /** Intercept focus-in event and ',' key presses */
    bool eventFilter(QObject *object, QEvent *event);

private:
    AmountSpinBox *amount;
    QValueComboBox *unit;

private Q_SLOTS:
    void unitChanged(int idx);

};

/** QSpinBox that uses fixed-point numbers internally and uses our own
* formatting/parsing functions.
*/
class AmountSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

public:
    explicit AmountSpinBox(QWidget *parent);

    QValidator::State validate(QString &text, int &pos) const;

    void fixup(QString &input) const;

    CAmount value(bool *valid_out = 0) const 
    { return parse(text(), valid_out); }

    void setValue(const CAmount& value);

    void stepBy(int steps);

    void setDisplayUnit(int unit);

    void setSingleStep(const CAmount& step)
    { singleStep = step; }

    QSize minimumSizeHint() const;

private:
    int currentUnit;
    CAmount singleStep;
    mutable QSize cachedMinimumSizeHint;

    /**
    * Parse a string into a number of base monetary units and
    * return validity.
    * @note Must return 0 if !valid.
    */
    CAmount parse(const QString &text, bool *valid_out = 0) const;

protected:
    bool event(QEvent *event);

    StepEnabled stepEnabled() const;

Q_SIGNALS:
    void valueChanged();
};

#endif // SIGE_QT_SIGECOINAMOUNTFIELD_H
