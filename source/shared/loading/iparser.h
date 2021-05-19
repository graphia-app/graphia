/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef IPARSER_H
#define IPARSER_H

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/failurereason.h"

class QUrl;
class IGraphModel;
class QString;

class IParser : virtual public Progressable, virtual public Cancellable, virtual public FailureReason
{
public:
    ~IParser() override = default;

    virtual bool parse(const QUrl& url, IGraphModel* graphModel = nullptr) = 0;
    virtual QString log() const { return {}; }
};

#endif // IPARSER_H
