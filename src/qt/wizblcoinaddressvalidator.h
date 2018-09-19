// Copyright (c) 2011-2014 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WIZBLCOIN_QT_WIZBLCOINADDRESSVALIDATOR_H
#define WIZBLCOIN_QT_WIZBLCOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class WizblcoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit WizblcoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

/** Wizblcoin address widget validator, checks for a valid wizblcoin address.
 */
class WizblcoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit WizblcoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // WIZBLCOIN_QT_WIZBLCOINADDRESSVALIDATOR_H
