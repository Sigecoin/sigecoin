// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIGE_QT_SIGECOINADDRESSVALIDATOR_H
#define SIGE_QT_SIGECOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class SigecoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit SigecoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Sigecoin address widget validator, checks for a valid Sigecoin address.
 */
class SigecoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit SigecoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // SIGE_QT_SIGECOINADDRESSVALIDATOR_H
