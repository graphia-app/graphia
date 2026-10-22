# Pending decisions from the unit test work

These came up while writing the tests on `claude/unit-test-infrastructure`. Each
bug fix is a separate commit on `claude/bug-fixes`, so any of the changes below
can be dropped on its own, along with the commit on the test branch that tests
it. This file should be deleted once everything here is resolved.

## Is this a bug?

- **Attribute Synthesis gives non-matching elements a value.** Elements whose
  source value doesn't match the regular expression get `""`, or `0` if the
  new attribute is detected as an int, rather than having no value. That
  makes them indistinguishable from real empty or zero values. This was left
  untested and unchanged.
- **Enrichment's Bonferroni correction.** Each p-value is multiplied by the
  number of values of the second attribute, though a test is done for every
  pair of values of the two attributes. That's right if each value of the
  first attribute is regarded as a separate family of tests, but otherwise
  it understates the correction.
- **Clone Attribute only clones the nodes in the transformed graph.** Nodes
  hidden by a filter, or merged into others, get no value in the clone, so
  removing the filter reveals nodes without values. Using every node in the
  graph would fix that for user attributes, but it's not clear what the clone
  of an attribute computed by a transform should hold for nodes it doesn't
  cover.
- **GraphML exports name nodes by their GraphML ids.** Node names are written
  as a "Description" attribute, and the ids are `n0`, `n1` and so on, so
  re-importing an export gives every node a different name. The names could
  be used as the ids when they're unique.
- **Exports include the attributes Graphia computes.** GML and GraphML exports
  include Node Degree, Component Identifier and the like, which come back as
  user attributes when the file is imported.
- **Pairwise exports drop nodes that have no edges.** Since 4e36b1e, a row
  with a single node name creates that node, so these could be exported too.
- **Attributes from files named like those parsers declare.** A file
  attribute called, for example, `NodeName` shares the user data that holds
  the node names, so its values are lost. Before `i18n` it was called
  `Node NodeName`, so didn't clash. Clashes with attributes Graphia declares
  are already given a "User " prefix; doing the same for those of the parser
  would avoid this.
- **Colours for string values only use part of the hue range.**
  `u::colorForString`, which colours string values that a palette gives no
  colour, takes the hue modulo 255, whereas `QColor` hues run from 0 to 359,
  so blue-violet to pink never appear. It also adds 128 to a `char`, which is
  unsigned on some platforms, such as Linux on ARM, where the colours would
  differ. Changing either would change existing colours.
- **Strings starting with `$` in conditions.** A quoted string in a condition
  is stored in the same way as an attribute name, which is distinguished only
  by a leading `$`. So in `$"Node Name" == "$HOME"`, `"$HOME"` is taken to be
  the attribute `HOME`. Fixing this needs a separate type for attribute
  references in `GraphTransformConfig::TerminalValue`, and a change to how
  conditions are passed to QML, which relies on the `$`.
- **Removing a component while the layout runs.** If a component is
  removed while the layout thread isn't paused,
  `LayoutThread::removeComponent` pauses it, removes the component's layout,
  then resumes it, all while the graph is still part way through changing,
  so the layout runs on a graph that's being rebuilt. In a test that does
  this, the result is a fatal "componentById called with component
  management disabled", nodes left unpositioned, or a hang. `Document`
  pauses the layout whenever a command starts, and graphs only change in
  commands, so this doesn't happen in the app, and the tests don't do it.
  Either the resume could wait for `graphChanged`, or the branch could go,
  with an assertion that the layout is paused.
- **Picking the node nearest a click.** `nodeIdInsideFrustumNearestPoint` in
  `graphcommoninteractor.cpp` scores each node in the picking frustum by its
  distance from the clicked point plus `Ray(frustum.centreLine()).distanceTo(point)`.
  The second term uses the clicked point rather than the node's position, so
  it's the same for every node and has no effect; it was presumably meant to
  favour nodes near the centre of the frustum. Using the node's position would
  change which node is picked.

- **Empty values in numeric column annotations.** In the correlation plot,
  an empty value in a column annotation whose other values are all numbers
  is drawn in the gradient's lowest colour, the same as the smallest value,
  whereas in other annotations empty values are left transparent
  (`QCPColumnAnnotations::resolveRects`). Leaving them transparent would
  distinguish them.

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
- **5570d01: enrichment results in saved files.** Only the results shown
  in the table were saved, which, by default, is only those that are
  enriched, and in the order the table was sorted by. All of the results are
  now saved, in their original order.
- **Clearing a numeric attribute value.** Editing a numeric value to nothing
  was ignored. It now removes the value, which undoing an edit of a value that
  was missing requires.
- **Newlines in locked transforms.** A newline in a string value in a locked
  transform was shown as a line break; since the fix for labels with quotes
  or backslashes, it's shown as a space, like other whitespace.
- **Deleting a merged node from its context menu.** "Delete 'a'" on a node
  that a transform such as Contract Edges merged with others now deletes all
  of them, as deleting it when it's selected does. Before, only the node it
  was shown as was deleted, and the rest stayed, shown as one node.
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
- **Adjacency matrices:** see the section below.
- **MATLAB:**
  - The first numeric variable is used rather than the largest, because the
    function returns inside its loop.
  - The file is never closed (no `Mat_VarFree` or `Mat_Close`).
  - Complex data isn't checked for, and is reinterpreted as real.
  - When nothing usable is found, it fails without giving a reason.
- **JSON Graph:**
  - A graph with no `edges` key is rejected, though the key is optional.
  - Numeric ids are rejected.
  - A label is used as the node name but isn't stored as a "Node Name"
    attribute.
- **GraphML:** nodes in nested graphs are dropped. This is a known FIXME.
- **GML:** a duplicate `id` silently creates two nodes, and a non-string label
  is dropped.
- **Correlation:** `CorrelationFileParser::epsilonFor` returns `DBL_MAX / 2`
  when the data has no positive values.
- **Visualisations:**
  - A string parameter ending in a backslash can't be read back, as the
    closing quote is taken as escaped.

## Adjacency matrices

- **Which way edges go.** In a CSV adjacency matrix, a value at row r and
  column c creates an edge from c's node to r's. In a MATLAB matrix it's the
  other way round, from r's node to c's, which is the more usual convention.
- **Node names in edge lists.** A three-column table of two integer columns
  and a number is loaded as an edge list, whose nodes are named "Node 1",
  "Node 2" and so on in the order they're found, rather than after the
  integers. So the integer ids in a CSV edge list are lost, and a sparse
  MATLAB matrix, which is loaded as an edge list, gets different node names
  from the same matrix stored densely.
- **MATLAB data is transposed.** `MatLabFileParser` puts each MATLAB column
  in a row of its table. That's what makes the adjacency matrix edges go
  from row to column, but where a MATLAB file is loaded as a table, such as
  for correlation data, a matrix with a row per gene gives a column per gene.
- **3 by 3 matrices.** A 3 by 3 adjacency matrix of integers is
  indistinguishable from an edge list, and is loaded as one.

## Exported tables

- **Line breaks in TSV files.** A value with line breaks is now written to
  a TSV file with each replaced by a space, as TSV doesn't allow them in
  fields; tabs were already removed, rather than replaced. CSV files quote
  such values, as RFC 4180 requires, though Graphia's own CSV parser reads
  a line at a time, so can't load them back.

## The node attribute table

- **Missing values when sorting descending.** Sorting a column of the node
  attribute table puts the nodes with no value for it last when ascending,
  but first when descending, as the order is simply reversed. Spreadsheets
  usually keep missing values last either way. `tst_nodeattributetablemodel`
  pins the current order.

## The PR

The branch has over 220 commits, about half of them fixes, and a PR was deferred until more was
implemented. It may be worth deciding where to draw the line.
