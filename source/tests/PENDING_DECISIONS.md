# Pending decisions from the unit test work

These came up while writing the tests on `claude/unit-test-infrastructure`. Each
bug fix is a separate commit, so any of the changes below can be dropped on its
own. This file should be deleted once everything here is resolved.

## Is this a bug?

- **Attribute Synthesis gives non-matching elements a value.** Elements whose
  source value doesn't match the regular expression get `""`, or `0` if the
  new attribute is detected as an int, rather than having no value. That
  makes them indistinguishable from real empty or zero values. This was left
  untested and unchanged.

## Behaviour changes to confirm or drop

- **4e36b1e: pairwise rows with only one node name.** A blank row used to
  create a node named `""` with a self loop, and a row with one name created
  an edge to that `""` node. Blank rows are now ignored, and a row with one
  name creates just that node, with no edge. This allows nodes with no edges
  to be specified.
- **938d615: BioPAX `right` and `controlled`.** These created each edge four
  times, by pushing the same source and target twice. They now create one
  edge, like the other edge properties. Please check that the duplication
  wasn't meant to do something else, such as reverse the direction.
- **6c1d490: integers too large for an `int`.** These were typed as ints but
  read with `toInt()`, so they became 0. They're now treated as floats, which
  are stored at single precision.
- **9fd3601: DOT `node [...]` and `edge [...]` defaults.** These used to be
  applied to every element after parsing, overriding explicit values and
  ignoring subgraph scope. They now follow the DOT spec: they apply to
  elements created after the statement, within its graph or subgraph, and
  explicit values take precedence.

## Known parser gaps, not fixed

- **DOT:**
  - `strict` is ignored, so duplicate edges are kept.
  - These forms fail to parse:
    - `"a" + "b"` string concatenation
    - several attribute lists, as in `a [x=1][y=2]`
    - `[` inside HTML strings
    - a quoted string ending in a backslash
    - `#` preprocessor lines
    - a UTF-8 BOM
  - In `{a -> b} -> c`, the edges to `c` are lost.
- **Adjacency matrices:**
  - A file with exactly 3 integer columns, such as a 3x3 0/1 matrix, is read
    as an edge list.
  - Edge list mode names nodes "Node 1", "Node 2" and so on in order of
    appearance, rather than using the ids in the file.
  - CSV and MATLAB matrices give opposite edge directions. CSV cell (row i,
    column j) gives an edge j→i, but MATLAB `M(i,j)` gives i→j.
- **MATLAB:**
  - The first numeric variable is used rather than the largest, because the
    function returns inside its loop.
  - The file is never closed (no `Mat_VarFree` or `Mat_Close`).
  - Complex data isn't checked for, and is reinterpreted as real.
  - When nothing usable is found, it fails without giving a reason.
- **JSON Graph:**
  - JGF v2 nodes keyed by id without an `"id"` field are rejected.
  - A graph with no `edges` key is rejected, though the key is optional.
  - Numeric ids are rejected.
  - A label is used as the node name but isn't stored as a "Node Name"
    attribute.
- **GraphML:** nodes in nested graphs are dropped. This is a known FIXME.
- **GML:** a duplicate `id` silently creates two nodes, and a non-string label
  is dropped.
- **Correlation:** `CorrelationFileParser::epsilonFor` returns `DBL_MAX / 2`
  when the data has no positive values.
- **Observed, not investigated:** a numeric user attribute that an element has
  no value for reads as 0 rather than as missing.

## The PR

The branch has around 50 commits, and a PR was deferred until more was
implemented. It may be worth deciding where to draw the line.
