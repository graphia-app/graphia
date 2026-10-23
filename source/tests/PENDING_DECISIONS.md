# Pending decisions from the unit test work

These came up while writing the tests on `claude/unit-test-infrastructure`.
Most have now been decided. What's left is split into changes to make, and
questions deferred for later. Delete this file once everything in it is
resolved.

Fixes are on `claude/bug-fixes`, one commit each, and each is tested by a
commit on the test branch titled "Test the fix for ...".

## Decided: changes to make

- **Attribute Synthesis values for elements that don't match.** An element
  whose source value doesn't match the regular expression gets `""`, or `0`
  if the new attribute is detected as an integer, so it can't be told apart
  from one with a real empty or zero value. Give such elements no value.
- **Clone Attribute and nodes that aren't shown.** Only the nodes in the
  transformed graph get a value in the clone, so nodes hidden by a filter, or
  merged into others, have none, which shows when the filter is removed. For
  attributes from the user's data, copy the values of every node. Attributes
  a transform computes stay as they are.
- **Computed attributes in exports.** GML and GraphML exports include the
  attributes Graphia computes, such as `NodeDegree` and
  `NodeComponentIdentifier`, which come back as user attributes when the
  file is imported. Leave them out.
- **Pairwise exports and nodes with no edges.** Such nodes aren't written.
  Write a row with just the node's name for each, which the pairwise parser
  now loads as a node with no edges.
- **File attributes named like the attributes a parser declares.** An
  attribute from a file named, for example, `NodeName` shares the user data
  that holds the node names, so its own values are lost. Give these the
  "User " prefix, as attributes that clash with the ones Graphia declares
  already get.
- **Colours of string values.** Fix both problems in `u::colorForString`,
  though it changes the colours of existing documents:
  - it takes the hue modulo 255, but `QColor` hues run from 0 to 359, so
    blue-violet to pink never appear;
  - it adds 128 to a `char`, which is unsigned on some platforms, such as
    Linux on ARM, so the colours differ between platforms.
- **Strings starting with `$` in conditions.** A string in a condition is
  stored the same way as an attribute name, which is only told apart by a
  leading `$`, so in `$NodeName == "$HOME"`, `"$HOME"` is taken to be the
  attribute `HOME`. Give attribute names their own type in
  `GraphTransformConfig::TerminalValue`, and change how conditions are given
  to QML, which also relies on the `$`.
- **Removing a component while the layout runs.** If a component is removed
  while the layout isn't paused, `LayoutThread::removeComponent` pauses it,
  removes the component's layout, then resumes it while the graph is still
  part way through changing. `Document` always pauses the layout before the
  graph changes, so remove that branch and assert that the layout is paused.
- **Empty values in numeric column annotations.** In the correlation plot,
  an empty value in a column annotation whose other values are all numbers
  is drawn in the gradient's lowest colour. Leave it transparent, as other
  annotations do (`QCPColumnAnnotations::resolveRects`).
- **Missing values when sorting the node attribute table.** Nodes with no
  value in the sorted column go last when sorting ascending, but first when
  descending. Keep them last either way, and update
  `tst_nodeattributetablemodel`, which pins the current order.
- **Adjacency matrix edge direction and MATLAB transposing.** These two
  changes go together, so that both formats make an edge from row r's node
  to column c's:
  - In a CSV adjacency matrix, the value at row r and column c creates an
    edge from c's node to r's. Make it go from r's to c's.
  - `MatLabFileParser` puts each MATLAB column in a row of its table, so a
    MATLAB file loaded as a table, such as for correlation data, has a
    column per row of the matrix. Stop transposing, and reverse the matrix
    edges so they still go from r's node to c's.
- **Parser gaps.** Fix all of these:
  - DOT:
    - `strict` is ignored, so duplicate edges are kept;
    - in `{a -> b} -> c`, the edges to `c` are lost;
    - these fail to parse: `"a" + "b"` string concatenation, more than one
      attribute list as in `a [x=1][y=2]`, `[` inside HTML strings, a quoted
      string ending in a backslash, `#` preprocessor lines, and a UTF-8 byte
      order mark.
  - MATLAB: complex data isn't detected, and is read as if it were real.
  - JSON Graph:
    - a graph with no `edges` key is rejected, though the key is optional;
    - numeric node ids, e.g. `"id": 1`, are rejected;
    - a node's label is used as its name, but not stored as a `NodeName`
      attribute.
  - GML: a repeated `id` silently creates two nodes, and a label that isn't
    a string is dropped.
  - Correlation: `CorrelationFileParser::epsilonFor` returns `DBL_MAX / 2`
    when the data has no positive values.
  - Visualisations: a string parameter ending in a backslash can't be read
    back, as its closing quote is taken to be escaped.

## Deferred

- **Enrichment's Bonferroni correction.** Each p-value is multiplied by the
  number of values of the second attribute, though a test is done for every
  pair of values of the two attributes. That's right if each value of the
  first attribute is its own family of tests; otherwise the correction is
  too small. Which is intended?
- **Node ids in GraphML exports.** Nodes are given the ids `n0`, `n1` and so
  on, and their names are written as a "Description" attribute, so
  re-importing an export gives every node a different name. Use the node
  names as the ids when they're unique?
- **Picking the node nearest a click.** `nodeIdInsideFrustumNearestPoint`,
  in `graphcommoninteractor.cpp`, scores each node in the picking frustum by
  its distance from the clicked point, plus the distance of the clicked
  point from the frustum's centre line. The second term is the same for
  every node, so has no effect; presumably it was meant to be the node's
  distance from the centre line, to favour nodes near the middle of the
  frustum. Use the node's position? This changes which node a click picks.
- **Node names in edge lists.** A table of two integer columns and a number
  column is loaded as an edge list, whose nodes are named "Node 1", "Node 2"
  and so on in the order they're found, not after the integers. So the ids
  in a CSV edge list are lost, and a sparse MATLAB matrix, which is loaded
  as an edge list, gets different node names from the same matrix stored
  densely. Name the nodes after the integers?
- **GraphML nested graphs.** Nodes in nested graphs are dropped. The code
  has a FIXME for this. Fix it?

## Decided: no change

- The fixes that changed behaviour that may have been intentional all stay:
  pairwise rows with one node name, BioPAX `right` and `controlled`
  properties, integers too large for an `int`, enrichment results in saved
  files, editing a numeric value to nothing, newlines in locked transforms,
  deleting a merged node from its context menu, DOT `node [...]` and
  `edge [...]` statements, and line breaks in exported TSV files.
- A 3 by 3 adjacency matrix of integers, which can't be told apart from an
  edge list, stays loaded as an edge list.
- No pull requests are to be opened for these branches.
