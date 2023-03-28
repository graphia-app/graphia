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

#ifndef PAIRWISEEXPORTER_H
#define PAIRWISEEXPORTER_H

#include "loading/saverfactory.h"

#include <QString>

using namespace Qt::Literals::StringLiterals;

class PairwiseSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    static QString name() { return u"Pairwise Text"_s; }
    static QString extension() { return u"txt"_s; }
    PairwiseSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using PairwiseSaverFactory = SaverFactory<PairwiseSaver>;

#endif // PAIRWISEEXPORTER_H
