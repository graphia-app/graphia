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

#ifndef QCP_LAYOUTELEMENT_TEXTELEMENT_H
#define QCP_LAYOUTELEMENT_TEXTELEMENT_H

#include "../global.h"
#include "../layer.h"
#include "../layout.h"

class QCPPainter;
class QCustomPlot;

class QCP_LIB_DECL QCPTextElement : public QCPLayoutElement
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(QFont font READ font WRITE setFont)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
  Q_PROPERTY(QFont selectedFont READ selectedFont WRITE setSelectedFont)
  Q_PROPERTY(QColor selectedTextColor READ selectedTextColor WRITE setSelectedTextColor)
  Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
  Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectionChanged)
  /// \endcond
public:
  explicit QCPTextElement(QCustomPlot *parentPlot);
  QCPTextElement(QCustomPlot *parentPlot, const QString &text);
  QCPTextElement(QCustomPlot *parentPlot, const QString &text, double pointSize);
  QCPTextElement(QCustomPlot *parentPlot, const QString &text, const QString &fontFamily, double pointSize);
  QCPTextElement(QCustomPlot *parentPlot, const QString &text, const QFont &font);
  
  // getters:
  QString text() const { return mText; }
  int textFlags() const { return mTextFlags; }
  QFont font() const { return mFont; }
  QColor textColor() const { return mTextColor; }
  QFont selectedFont() const { return mSelectedFont; }
  QColor selectedTextColor() const { return mSelectedTextColor; }
  bool selectable() const { return mSelectable; }
  bool selected() const { return mSelected; }
  
  // setters:
  void setText(const QString &text);
  void setTextFlags(int flags);
  void setFont(const QFont &font);
  void setTextColor(const QColor &color);
  void setSelectedFont(const QFont &font);
  void setSelectedTextColor(const QColor &color);
  Q_SLOT void setSelectable(bool selectable);
  Q_SLOT void setSelected(bool selected);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  virtual void mousePressEvent(QMouseEvent *event, const QVariant &details) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent *event, const QPointF &startPos) Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QMouseEvent *event, const QVariant &details) Q_DECL_OVERRIDE;
  
signals:
  void selectionChanged(bool selected);
  void selectableChanged(bool selectable);
  void clicked(QMouseEvent *event);
  void doubleClicked(QMouseEvent *event);
  
protected:
  // property members:
  QString mText;
  int mTextFlags;
  QFont mFont;
  QColor mTextColor;
  QFont mSelectedFont;
  QColor mSelectedTextColor;
  QRect mTextBoundingRect;
  bool mSelectable, mSelected;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;
  virtual QSize maximumOuterSizeHint() const Q_DECL_OVERRIDE;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) Q_DECL_OVERRIDE;
  virtual void deselectEvent(bool *selectionStateChanged) Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QFont mainFont() const;
  QColor mainTextColor() const;
  
private:
  Q_DISABLE_COPY(QCPTextElement)
};



#endif // QCP_LAYOUTELEMENT_TEXTELEMENT_H
