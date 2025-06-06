/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef IMPORTATTRIBUTESKEYDETECTION_H
#define IMPORTATTRIBUTESKEYDETECTION_H

#include "app/ui/document.h"
#include "shared/loading/tabulardata.h"
#include "shared/utils/cancellable.h"

#include <QObject>
#include <QQmlEngine>
#include <QFutureWatcher>
#include <QVariantMap>

class ImportAttributesKeyDetection : public QObject, public Cancellable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Document* document MEMBER _document)
    Q_PROPERTY(std::shared_ptr<TabularData> tabularData MEMBER _tabularData)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QVariantMap result MEMBER _result NOTIFY resultChanged)

private:
    Document* _document = nullptr;
    std::shared_ptr<TabularData> _tabularData = nullptr;
    QFutureWatcher<void> _watcher;
    QVariantMap _result;

public:
    ImportAttributesKeyDetection();
    ~ImportAttributesKeyDetection() override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void cancel() override { Cancellable::cancel(); }

    bool busy() const { return _watcher.isRunning(); }

signals:
    void busyChanged();
    void resultChanged();
};

#endif // IMPORTATTRIBUTESKEYDETECTION_H
