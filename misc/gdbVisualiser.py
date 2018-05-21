#!/usr/bin/python

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
