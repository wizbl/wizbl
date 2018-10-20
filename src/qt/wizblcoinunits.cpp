// Copyright (c) 2011-2016 The Wizblcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wizblcoinunits.h"

#include "primitives/transaction.h"
#include "../policy/policy.h"

#include <QStringList>

WizblcoinUnits::WizblcoinUnits(QObject *parent):
    QAbstractListModel(parent),
    unitlist(availableUnits())
{
}

QList<WizblcoinUnits::Unit> WizblcoinUnits::availableUnits()
{
    QList<WizblcoinUnits::Unit> unitlist;
    unitlist.append(WBL);
    unitlist.append(mWBL);
    unitlist.append(uWBL);
    return unitlist;
}

bool WizblcoinUnits::valid(int unit)
{
    switch(unit)
    {
    case WBL:
    case mWBL:
    case uWBL:
        return true;
    default:
        return false;
    }
}

QString WizblcoinUnits::name(int unit)
{
    switch(unit)
    {
    case WBL: return QString("WBL");
    case mWBL: return QString("mWBL");
    case uWBL: return QString::fromUtf8("μWBL");
    default: return QString("???");
    }
}

QString WizblcoinUnits::description(int unit)
{
    switch(unit)
    {
    case WBL: return QString(TEXT_Wizbl "coins");
    case mWBL: return QString("Milli-" TEXT_Wizbl "coins (1 / 1" THIN_SP_UTF8 "000)");
    case uWBL: return QString("Micro-" TEXT_Wizbl "coins (1 / 1" THIN_SP_UTF8 "000" THIN_SP_UTF8 "000)");
    default: return QString("???");
    }
}

qint64 WizblcoinUnits::factor(int unit)
{
    switch(unit)
    {
    case WBL:  return 100000000;
    case mWBL: return 100000;
    case uWBL: return 100;
    default:   return 100000000;
    }
}

int WizblcoinUnits::decimalsLength(int unit)
{
    switch(unit)
    {
    case WBL: return 8;
    case mWBL: return 5;
    case uWBL: return 2;
    default: return 0;
    }
}

//#define DECIMAL_DISPLAY_DECREASE 4//DEFAULT_TRANSACTION_FEE_DENOMINATOR 180809 기준 '0' 4개
int WizblcoinUnits::decimal_display_decrease()
{
    static unsigned long static_Check_for_fee_change = 0;
    static unsigned int uiRet = 0;
    if (static_Check_for_fee_change != TRANSACTION_FEE_DENOMINATOR)
    {
        uiRet = 0;
        static_Check_for_fee_change = TRANSACTION_FEE_DENOMINATOR;
        unsigned int ui = 0;
        unsigned long uidenominator = TRANSACTION_FEE_DENOMINATOR;
        for (; 10 <= uidenominator; ui++)
        {
            uidenominator = uidenominator / 10;
        }
        uiRet = ui;
    }
    return uiRet;
}

QString WizblcoinUnits::format(int unit, const CAmount& nIn, bool fPlus, SeparatorStyle separators, bool decimalDisplayDecrease/* = false*/)
{
    // Note: not using straight sprintf here because we do NOT want
    // localized number formatting.
    if(!valid(unit))
        return QString(); // Refuse to format invalid unit
    qint64 n = (qint64)nIn;
    qint64 coin = factor(unit);
    const int num_decimals = decimalsLength(unit);
    qint64 n_abs = (n > 0 ? n : -n);
    qint64 quotient = n_abs / coin;
    QString quotient_str = QString::number(quotient);

    // Use SI-style thin space separators as these are locale independent and can't be
    // confused with the decimal marker.
    QChar thin_sp(THIN_SP_CP);
    int q_size = quotient_str.size();
    if (separators == separatorAlways || (separators == separatorStandard && q_size > 4))
        for (int i = 3; i < q_size; i += 3)
            quotient_str.insert(q_size - i, thin_sp);

    if (n < 0)
        quotient_str.insert(0, '-');
    else if (fPlus && n > 0)
        quotient_str.insert(0, '+');

    if (num_decimals > 0) {
        qint64 remainder = n_abs % coin;
        QString remainder_str = QString::number(remainder).rightJustified(num_decimals, '0');
        return quotient_str + QString(".") + (decimalDisplayDecrease && !IsAllowedDustAmount(nIn) && num_decimals >= decimal_display_decrease() ?
            remainder_str.leftJustified(num_decimals - decimal_display_decrease(), '0', decimalDisplayDecrease) :
            remainder_str);
    } else {
        return quotient_str;
    }
}


// NOTE: Using formatWithUnit in an HTML context risks wrapping
// quantities at the thousands separator. More subtly, it also results
// in a standard space rather than a thin space, due to a bug in Qt's
// XML whitespace canonicalisation
//
// Please take care to use formatHtmlWithUnit instead, when
// appropriate.

QString WizblcoinUnits::formatWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    return format(unit, amount, plussign, separators) + QString(" ") + name(unit);
}

QString WizblcoinUnits::formatHtmlWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    QString str(formatWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}


bool WizblcoinUnits::parse(int unit, const QString &value, CAmount *val_out, bool decimalDisplayDecrease/* = false*/)
{
    if(!valid(unit) || value.isEmpty())
        return false; // Refuse to parse invalid unit or empty string
    const int num_decimals = decimalsLength(unit);
    const int iNumberLengthLimit = 18;  // Longer numbers will exceed 63 bits

    // Ignore spaces and thin spaces when parsing
    QStringList parts = removeSpaces(value).split(".");

    if(parts.size() > 2)
    {
        return false; // More than one dot
    }
    QString whole = parts[0];
    QString decimals;

    if(parts.size() > 1)
    {
        decimals = parts[1];
    }

    bool ok = false;
    QString strValue;
    {
        if ((whole.size() > (iNumberLengthLimit - num_decimals)) || decimals.size() > num_decimals) {
            return false; // Exceeds max precision
        }
        strValue = whole + decimals.leftJustified(num_decimals, '0');

        if (strValue.size() > iNumberLengthLimit) {
            return false; // Longer numbers will exceed 63 bits
        }
    }

    const CAmount retValue = strValue.toLongLong(&ok);

    if (decimalDisplayDecrease && !IsAllowedDustAmount(retValue))
    {
        const int ddd = decimal_display_decrease();
        const CAmount retValueDDD = retValue - (retValue % ddd);
        if(retValue != retValueDDD)
            return false;
        if (2 == parts.size()) {
            QStringList partsDDD = format(unit, retValueDDD, false, separatorNever, decimalDisplayDecrease).split(".");
            if (2 == partsDDD.size() && parts[1].length() > partsDDD[1].length())
                return false;
        }
    }

    if(val_out)
    {
        *val_out = retValue;
    }
    return ok;
}

QString WizblcoinUnits::getAmountColumnTitle(int unit)
{
    QString amountTitle = QObject::tr("Amount");
    if (WizblcoinUnits::valid(unit))
    {
        amountTitle += " ("+WizblcoinUnits::name(unit) + ")";
    }
    return amountTitle;
}

int WizblcoinUnits::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return unitlist.size();
}

QVariant WizblcoinUnits::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < unitlist.size())
    {
        Unit unit = unitlist.at(row);
        switch(role)
        {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return QVariant(name(unit));
        case Qt::ToolTipRole:
            return QVariant(description(unit));
        case UnitRole:
            return QVariant(static_cast<int>(unit));
        }
    }
    return QVariant();
}

CAmount WizblcoinUnits::maxMoney()
{
    return MAX_MONEY;
}
