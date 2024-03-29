/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "qcustomplotcolorprovider.h"

#include "shared/utils/shadedcolors.h"

#include <QGuiApplication>
#include <QPalette>

QColor QCustomPlotColorProvider::backgroundColor()
{
    auto palette = QGuiApplication::palette();
    return ShadedColors::light(palette);
}

QColor QCustomPlotColorProvider::lightPenColor()
{
    auto palette = QGuiApplication::palette();
    return ShadedColors::mid(palette);
}

QColor QCustomPlotColorProvider::penColor()
{
    auto palette = QGuiApplication::palette();
    return ShadedColors::darkest(palette);
}
