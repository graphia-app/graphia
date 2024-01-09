/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MODELCOMPLETER_H
#define MODELCOMPLETER_H

#include "shared/utils/static_block.h"

#include <QQmlEngine>
#include <QObject>
#include <QAbstractItemModel>
#include <QString>
#include <QList>

class ModelCompleter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel* model MEMBER _model NOTIFY modelChanged)
    Q_PROPERTY(QString startsWith MEMBER _term NOTIFY termChanged)
    Q_PROPERTY(QList<QModelIndex> candidates MEMBER _candidates READ candidates NOTIFY candidatesChanged)
    Q_PROPERTY(QString commonPrefix MEMBER _commonPrefix READ commonPrefix NOTIFY commonPrefixChanged)
    Q_PROPERTY(QModelIndex closestMatch MEMBER _closestMatch READ closestMatch NOTIFY closestMatchChanged)

private:
    QAbstractItemModel* _model = nullptr;
    QString _term;
    QList<QModelIndex> _candidates;
    QString _commonPrefix;
    QModelIndex _closestMatch;

    auto candidates() const { return _candidates; }
    auto commonPrefix() const { return _commonPrefix; }
    auto closestMatch() const { return _closestMatch; }

    void update();

public:
    ModelCompleter();

signals:
    void modelChanged();
    void termChanged();
    void candidatesChanged();
    void commonPrefixChanged();
    void closestMatchChanged();
};

static_block
{
    qmlRegisterType<ModelCompleter>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "ModelCompleter");
}

#endif // MODELCOMPLETER_H
