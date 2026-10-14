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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QReadWriteLock>
#include <QString>
#include <QTranslator>

#include <memory>
#include <vector>

class QObject;

class Translator : public QTranslator
{
    Q_OBJECT

public:
    using QTranslator::QTranslator;

    void addPluginCatalogue(const QString& pluginName, const QObject* plugin);

    QString translate(const char* context, const char* sourceText,
        const char* disambiguation, int n) const override;

private:
    struct PluginCatalogue
    {
        const QObject* _plugin;
        std::unique_ptr<QTranslator> _translator;
    };

    // Plugin catalogues may come and go while other threads translate
    mutable QReadWriteLock _lock;
    std::vector<PluginCatalogue> _pluginCatalogues;

    void removePluginCatalogue(const QObject* plugin);
};

namespace u
{
    void installTranslator();
    void installPluginTranslator(const QString& pluginName, const QObject* plugin);
} // namespace u

#endif // TRANSLATOR_H
