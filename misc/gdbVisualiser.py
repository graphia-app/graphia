#!/usr/bin/python

def qdump__ElementId(d, value):
    v = value["_value"]

    if v >= 0:
        d.putValue(v)
    else:
        d.putValue("null")

    d.putNumChild(0)

def qdump__NodeId(d, value):
    qdump__ElementId(d, value)

def qdump__EdgeId(d, value):
    qdump__ElementId(d, value)

def qdump__ComponentId(d, value):
    qdump__ElementId(d, value)
