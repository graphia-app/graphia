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

#include "translator.h"

#include <QCoreApplication>
#include <QLocale>
#include <QObject>
#include <QReadLocker>
#include <QString>
#include <QWriteLocker>

using namespace Qt::Literals::StringLiterals;

QString u::translationLanguage(const QStringList& uiLanguages,
    const std::function<bool(const QString&)>& loadCatalogue)
{
    for(const auto& language : uiLanguages)
    {
        const QLocale locale(language);

        if(loadCatalogue(locale.name()))
            return locale.name();

        // Since en_GB is the source language and thus no translation exists for it,
        // we won't have been able to successfully load a catalogue for it. The next
        // step is to fall back to en_US, which we obviously don't want, so if we want
        // any English that hasn't loaded, we default to not doing any translation.
        // Note that this will also apply to non-British, non-American Englishes, where
        // en_GB is typically a better fit, hence the lack of an explicit en_GB check.
        if(locale.language() == QLocale::English)
            return {};
    }

    return {};
}

// The plugin catalogue is only installed in the language of the app
void Translator::addPluginCatalogue(const QString& pluginName, const QObject* plugin)
{
    auto translator = std::make_unique<QTranslator>();

    if(!translator->load(u":/i18n/%1_%2"_s.arg(pluginName.toLower(), language())))
        return;

    // The catalogue is in the plugin's resources, so it mustn't outlive the plugin
    connect(plugin, &QObject::destroyed, this, [this, plugin] { removePluginCatalogue(plugin); });

    const QWriteLocker locker(&_lock);
    _pluginCatalogues.push_back({plugin, std::move(translator)});
}

void Translator::removePluginCatalogue(const QObject* plugin)
{
    const QWriteLocker locker(&_lock);
    std::erase_if(_pluginCatalogues, [plugin](const auto& catalogue) { return catalogue._plugin == plugin; });
}

QString Translator::translate(const char* context, const char* sourceText,
    const char* disambiguation, int n) const
{
    auto translation = QTranslator::translate(context, sourceText, disambiguation, n);
    if(!translation.isNull())
        return translation;

    const QReadLocker locker(&_lock);

    for(const auto& catalogue : _pluginCatalogues)
    {
        translation = catalogue._translator->translate(context, sourceText, disambiguation, n);
        if(!translation.isNull())
            return translation;
    }

    return {};
}

void u::installTranslator()
{
    auto translator = std::make_unique<Translator>(QCoreApplication::instance());

    const auto language = translationLanguage(QLocale().uiLanguages(), [&translator](const QString& name)
        { return translator->load(u":/i18n/graphia_%1"_s.arg(name)); });

    if(!language.isEmpty())
        QCoreApplication::installTranslator(translator.release());
}

void u::installPluginTranslator(const QString& pluginName, const QObject* plugin)
{
    auto* appTranslator = QCoreApplication::instance()->findChild<Translator*>(
        Qt::FindDirectChildrenOnly);

    if(appTranslator != nullptr)
        appTranslator->addPluginCatalogue(pluginName, plugin);
}
