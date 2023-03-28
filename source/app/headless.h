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

#ifndef HEADLESS_H
#define HEADLESS_H

#include <QObject>


#include <memory>

struct HeadlessState;

class Headless : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<HeadlessState> _;

    void processNext();

public:
    Headless(const QStringList& sourceFilenames, const QString& parametersFilename);
    ~Headless() override;

public slots:
    void run();
    void cancel();

signals:
    void done();
};

#endif // HEADLESS_H
