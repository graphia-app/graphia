#!/usr/bin/python
#
# Copyright © 2013-2024 Graphia Technologies Ltd.
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.

from stdtypes import qdump__std__vector

def qdump__ElementId(d, value):
    v = value["_value"].integer()

    if v >= 0:
        d.putValue(v)
    elif v == -1:
        d.putValue("null")
    else:
        d.putValue("INVALID")

    d.putNumChild(0)

def qdump__NodeId(d, value):
    qdump__ElementId(d, value)

def qdump__EdgeId(d, value):
    qdump__ElementId(d, value)

def qdump__ComponentId(d, value):
    qdump__ElementId(d, value)

def qdump__GraphArray(d, value):
    v = value["_array"]

    qdump__std__vector(d, v)

def qdump__NodeArray(d, value):
    qdump__GraphArray(d, value)

def qdump__EdgeArray(d, value):
    qdump__GraphArray(d, value)

def qdump__ComponentArray(d, value):
    qdump__GraphArray(d, value)
