/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2022 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: https://www.qcustomplot.com/                         **
**             Date: 06.11.22                                             **
**          Version: 2.1.1                                                **
****************************************************************************/

#include "axistickerpi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxisTickerPi
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPAxisTickerPi
  \brief Specialized axis ticker to display ticks in units of an arbitrary constant, for example pi
  
  \image html axisticker-pi.png
  
  This QCPAxisTicker subclass generates ticks that are expressed with respect to a given symbolic
  constant with a numerical value specified with \ref setPiValue and an appearance in the tick
  labels specified with \ref setPiSymbol.
  
  Ticks may be generated at fractions of the symbolic constant. How these fractions appear in the
  tick label can be configured with \ref setFractionStyle.
  
  The ticker can be created and assigned to an axis like this:
  \snippet documentation/doc-image-generator/mainwindow.cpp axistickerpi-creation
*/

/*!
  Constructs the ticker and sets reasonable default values. Axis tickers are commonly created
  managed by a QSharedPointer, which then can be passed to QCPAxis::setTicker.
*/
QCPAxisTickerPi::QCPAxisTickerPi() :
  mPiSymbol(QLatin1String(" ")+QChar(0x03C0)),
  mPiValue(M_PI),
  mPeriodicity(0),
  mFractionStyle(fsUnicodeFractions),
  mPiTickStep(0)
{
  setTickCount(4);
}

/*!
  Sets how the symbol part (which is always a suffix to the number) shall appear in the axis tick
  label.
  
  If a space shall appear between the number and the symbol, make sure the space is contained in \a
  symbol.
*/
void QCPAxisTickerPi::setPiSymbol(QString symbol)
{
  mPiSymbol = symbol;
}

/*!
  Sets the numerical value that the symbolic constant has.

  This will be used to place the appropriate fractions of the symbol at the respective axis
  coordinates.
*/
void QCPAxisTickerPi::setPiValue(double pi)
{
  mPiValue = pi;
}

/*!
  Sets whether the axis labels shall appear periodicly and if so, at which multiplicity of the
  symbolic constant.
  
  To disable periodicity, set \a multiplesOfPi to zero.
  
  For example, an axis that identifies 0 with 2pi would set \a multiplesOfPi to two.
*/
void QCPAxisTickerPi::setPeriodicity(int multiplesOfPi)
{
  mPeriodicity = qAbs(multiplesOfPi);
}

/*!
  Sets how the numerical/fractional part preceding the symbolic constant is displayed in tick
  labels. See \ref FractionStyle for the various options.
*/
void QCPAxisTickerPi::setFractionStyle(QCPAxisTickerPi::FractionStyle style)
{
  mFractionStyle = style;
}

/*! \internal
  
  Returns the tick step, using the constant's value (\ref setPiValue) as base unit. In consequence
  the numerical/fractional part preceding the symbolic constant is made to have a readable
  mantissa.
  
  \seebaseclassmethod
*/
double QCPAxisTickerPi::getTickStep(const QCPRange &range)
{
  mPiTickStep = range.size()/mPiValue/double(mTickCount+1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
  mPiTickStep = cleanMantissa(mPiTickStep);
  return mPiTickStep*mPiValue;
}

/*! \internal
  
  Returns the sub tick count, using the constant's value (\ref setPiValue) as base unit. In
  consequence the sub ticks divide the numerical/fractional part preceding the symbolic constant
  reasonably, and not the total tick coordinate.
  
  \seebaseclassmethod
*/
int QCPAxisTickerPi::getSubTickCount(double tickStep)
{
  return QCPAxisTicker::getSubTickCount(tickStep/mPiValue);
}

/*! \internal
  
  Returns the tick label as a fractional/numerical part and a symbolic string as suffix. The
  formatting of the fraction is done according to the specified \ref setFractionStyle. The appended
  symbol is specified with \ref setPiSymbol.
  
  \seebaseclassmethod
*/
QString QCPAxisTickerPi::getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision)
{
  double tickInPis = tick/mPiValue;
  if (mPeriodicity > 0)
    tickInPis = fmod(tickInPis, mPeriodicity);
  
  if (mFractionStyle != fsFloatingPoint && mPiTickStep > 0.09 && mPiTickStep < 50)
  {
    // simply construct fraction from decimal like 1.234 -> 1234/1000 and then simplify fraction, smaller digits are irrelevant due to mPiTickStep conditional above
    int denominator = 1000;
    int numerator = qRound(tickInPis*denominator);
    simplifyFraction(numerator, denominator);
    if (qAbs(numerator) == 1 && denominator == 1)
      return (numerator < 0 ? QLatin1String("-") : QLatin1String("")) + mPiSymbol.trimmed();
    else if (numerator == 0)
      return QLatin1String("0");
    else
      return fractionToString(numerator, denominator) + mPiSymbol;
  } else
  {
    if (qFuzzyIsNull(tickInPis))
      return QLatin1String("0");
    else if (qFuzzyCompare(qAbs(tickInPis), 1.0))
      return (tickInPis < 0 ? QLatin1String("-") : QLatin1String("")) + mPiSymbol.trimmed();
    else
      return QCPAxisTicker::getTickLabel(tickInPis, locale, formatChar, precision) + mPiSymbol;
  }
}

/*! \internal
  
  Takes the fraction given by \a numerator and \a denominator and modifies the values to make sure
  the fraction is in irreducible form, i.e. numerator and denominator don't share any common
  factors which could be cancelled.
*/
void QCPAxisTickerPi::simplifyFraction(int &numerator, int &denominator) const
{
  if (numerator == 0 || denominator == 0)
    return;
  
  int num = numerator;
  int denom = denominator;
  while (denom != 0) // euclidean gcd algorithm
  {
    int oldDenom = denom;
    denom = num % denom;
    num = oldDenom;
  }
  // num is now gcd of numerator and denominator
  numerator /= num;
  denominator /= num;
}

/*! \internal
  
  Takes the fraction given by \a numerator and \a denominator and returns a string representation.
  The result depends on the configured fraction style (\ref setFractionStyle).
  
  This method is used to format the numerical/fractional part when generating tick labels. It
  simplifies the passed fraction to an irreducible form using \ref simplifyFraction and factors out
  any integer parts of the fraction (e.g. "10/4" becomes "2 1/2").
*/
QString QCPAxisTickerPi::fractionToString(int numerator, int denominator) const
{
  if (denominator == 0)
  {
    qDebug() << Q_FUNC_INFO << "called with zero denominator";
    return QString();
  }
  if (mFractionStyle == fsFloatingPoint) // should never be the case when calling this function
  {
    qDebug() << Q_FUNC_INFO << "shouldn't be called with fraction style fsDecimal";
    return QString::number(numerator/double(denominator)); // failsafe
  }
  int sign = numerator*denominator < 0 ? -1 : 1;
  numerator = qAbs(numerator);
  denominator = qAbs(denominator);
  
  if (denominator == 1)
  {
    return QString::number(sign*numerator);
  } else
  {
    int integerPart = numerator/denominator;
    int remainder = numerator%denominator;
    if (remainder == 0)
    {
      return QString::number(sign*integerPart);
    } else
    {
      if (mFractionStyle == fsAsciiFractions)
      {
        return QString(QLatin1String("%1%2%3/%4"))
            .arg(sign == -1 ? QLatin1String("-") : QLatin1String(""))
            .arg(integerPart > 0 ? QString::number(integerPart)+QLatin1String(" ") : QString(QLatin1String("")))
            .arg(remainder)
            .arg(denominator);
      } else if (mFractionStyle == fsUnicodeFractions)
      {
        return QString(QLatin1String("%1%2%3"))
            .arg(sign == -1 ? QLatin1String("-") : QLatin1String(""))
            .arg(integerPart > 0 ? QString::number(integerPart) : QLatin1String(""))
            .arg(unicodeFraction(remainder, denominator));
      }
    }
  }
  return QString();
}

/*! \internal
  
  Returns the unicode string representation of the fraction given by \a numerator and \a
  denominator. This is the representation used in \ref fractionToString when the fraction style
  (\ref setFractionStyle) is \ref fsUnicodeFractions.
  
  This method doesn't use the single-character common fractions but builds each fraction from a
  superscript unicode number, the unicode fraction character, and a subscript unicode number.
*/
QString QCPAxisTickerPi::unicodeFraction(int numerator, int denominator) const
{
  return unicodeSuperscript(numerator)+QChar(0x2044)+unicodeSubscript(denominator);
}

/*! \internal
  
  Returns the unicode string representing \a number as superscript. This is used to build
  unicode fractions in \ref unicodeFraction.
*/
QString QCPAxisTickerPi::unicodeSuperscript(int number) const
{
  if (number == 0)
    return QString(QChar(0x2070));
  
  QString result;
  while (number > 0)
  {
    const int digit = number%10;
    switch (digit)
    {
      case 1: { result.prepend(QChar(0x00B9)); break; }
      case 2: { result.prepend(QChar(0x00B2)); break; }
      case 3: { result.prepend(QChar(0x00B3)); break; }
      default: { result.prepend(QChar(0x2070+digit)); break; }
    }
    number /= 10;
  }
  return result;
}

/*! \internal
  
  Returns the unicode string representing \a number as subscript. This is used to build unicode
  fractions in \ref unicodeFraction.
*/
QString QCPAxisTickerPi::unicodeSubscript(int number) const
{
  if (number == 0)
    return QString(QChar(0x2080));
  
  QString result;
  while (number > 0)
  {
    result.prepend(QChar(0x2080+number%10));
    number /= 10;
  }
  return result;
}
