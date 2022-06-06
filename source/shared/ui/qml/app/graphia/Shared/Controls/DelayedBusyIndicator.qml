/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick
import QtQuick.Controls

BusyIndicator
{
    id: root

    property alias delay: timer.interval
    property bool delayedRunning: false

    running: false

    Timer
    {
        id: timer
        repeat: false
        onTriggered:
        {
            if(root.delayedRunning)
                root.running = true;
        }
    }

    onDelayedRunningChanged:
    {
        if(!root.delayedRunning)
        {
            timer.stop();
            root.running = false;
        }
        else
            timer.start();
    }
}