# Pending decisions from the unit test work

These came up while writing the tests on `claude/unit-test-infrastructure`.
Each entry says what happens now, then asks the question that needs an
answer. Delete this file once everything in it is resolved.

Fixes are on `claude/bug-fixes`, one commit each, named below by their
subjects. Any of them can be dropped on its own, along with the commit on the
test branch titled "Test the fix for ..." that tests it.

## Behaviour changes made by fixes: keep or drop?

These fixes change something that may have been intentional.

- **Pairwise rows with only one node name** ("Fix pairwise files creating a
  node with no name").
  - Before: a blank row created a node named `""` with an edge to itself, and
    a row with one name created an edge from that node to the `""` node.
  - Now: blank rows are ignored, and a row with one name creates just that
    node, with no edge, so a pairwise file can list nodes that have no
    edges.
  - Keep?
- **BioPAX `right` and `controlled` properties** ("Fix BioPAX right and
  controlled properties creating four edges").
  - Before: each of these created the same edge four times.
  - Now: one edge, as for the other properties that create edges.
  - Keep, unless the duplication was meant to do something else, such as
    add an edge in the other direction?
- **Integers too large for an `int`** ("Fix integers too large for an int
  attribute reading as zero").
  - Before: a value such as 3000000000 was typed as an integer but read as
    0.
  - Now: the attribute is typed as a float instead, so values are kept,
    though at single precision, i.e. to about 7 significant figures.
  - Keep?
- **Enrichment results in saved files** ("Fix enrichment results being lost
  when saving").
  - Before: only the rows the table showed were saved, which by default is
    only the enriched ones, in the order the table was sorted by.
  - Now: every result is saved, in its original order.
  - Keep?
- **Editing a numeric value to nothing** ("Fix undoing attribute edits not
  restoring the original values").
  - Before: the edit was ignored.
  - Now: it removes the value. Undoing an edit of a value that was missing
    needs this.
  - Keep?
- **Newlines in locked transforms** ("Fix transform labels with quotes or
  backslashes in them").
  - Before: a newline in a string value in a locked transform was shown as a
    line break.
  - Now: it's shown as a space, like other whitespace.
  - Keep?
- **Deleting a merged node from its context menu** ("Fix deleting a merged
  node from its menu only deleting part of it").
  - Before: "Delete 'a'" on a node that a transform such as Contract Edges
    had merged with others deleted only the node it was shown as, and the
    rest stayed, still shown as one node.
  - Now: it deletes all of them, as deleting the node when it's selected
    does.
  - Keep?
- **DOT `node [...]` and `edge [...]` statements** ("Fix DOT default
  attributes overriding all others").
  - Before: their attributes were applied to every node or edge once the
    file was parsed, overriding values given explicitly, and ignoring
    subgraphs.
  - Now: as the DOT language specifies, they apply only to the elements that
    follow them, within the same graph or subgraph, and explicit values take
    precedence.
  - Keep?
- **Line breaks in exported TSV files** ("Fix exported tables with line
  breaks in their values").
  - Before: a value with a line break split its row in two.
  - Now: line breaks become spaces in TSV files, which can't hold them in a
    field, as tabs were already removed. CSV files quote such values, as RFC
    4180 requires, but Graphia's own CSV parser reads a line at a time, so
    can't load them back.
  - Keep?

## Possible bugs, not changed

Each of these is left as it is, because fixing it would change results or
needs a design decision.

- **Attribute Synthesis values for elements that don't match.**
  - Now: an element whose source value doesn't match the regular expression
    gets `""`, or `0` if the new attribute is detected as an integer, so it
    can't be told apart from one with a real empty or zero value.
  - Should non-matching elements have no value instead?
- **Enrichment's Bonferroni correction.**
  - Now: each p-value is multiplied by the number of values of the second
    attribute, though a test is done for every pair of values of the two
    attributes.
  - That's right if each value of the first attribute is its own family of
    tests; otherwise the correction is too small. Which is intended?
- **Clone Attribute and nodes that aren't shown.**
  - Now: only the nodes in the transformed graph get a value in the clone,
    so nodes hidden by a filter, or merged into others, have none, which
    shows when the filter is removed.
  - Copying the values of every node would fix this for attributes from the
    user's data, but for an attribute a transform computes, it isn't clear
    what the clone should hold for nodes the transform doesn't cover. Change
    it for user attributes only?
- **Node ids in GraphML exports.**
  - Now: nodes are given the ids `n0`, `n1` and so on, and their names are
    written as a "Description" attribute, so re-importing an export gives
    every node a different name.
  - Use the node names as the ids when they're unique?
- **Computed attributes in exports.**
  - Now: GML and GraphML exports include the attributes Graphia computes,
    such as `NodeDegree` and `NodeComponentIdentifier`, which come back as
    user attributes when the file is imported.
  - Leave them out?
- **Pairwise exports and nodes with no edges.**
  - Now: such nodes aren't written. Since "Fix pairwise files creating a node
    with no name", a row with one node name creates that node, so they could
    be.
  - Write a one-name row for each?
- **File attributes named like the attributes a parser declares.**
  - Now: an attribute from a file named, for example, `NodeName` shares the
    user data that holds the node names, so its own values are lost. Before
    `i18n` it was called `Node NodeName`, so didn't clash.
  - Attributes that clash with ones Graphia declares are already given a
    "User " prefix. Do the same for those the parsers declare?
- **Colours of string values.**
  - Now: `u::colorForString`, which colours the string values a palette
    doesn't give a colour, takes the hue modulo 255, but `QColor` hues run
    from 0 to 359, so blue-violet to pink never appear. It also adds 128 to a
    `char`, which is unsigned on some platforms, such as Linux on ARM, so the
    colours differ between platforms.
  - Fixing either changes the colours of existing documents. Fix them anyway?
- **Strings starting with `$` in conditions.**
  - Now: a string in a condition is stored the same way as an attribute name,
    which is only told apart by a leading `$`. So in
    `$NodeName == "$HOME"`, `"$HOME"` is taken to be the attribute `HOME`.
  - Fixing it needs a separate type for attribute names in
    `GraphTransformConfig::TerminalValue`, and a change to how conditions are
    given to QML, which also relies on the `$`. Worth doing?
- **Removing a component while the layout runs.**
  - Now: if a component is removed while the layout isn't paused,
    `LayoutThread::removeComponent` pauses it, removes the component's
    layout, then resumes it, while the graph is still part way through
    changing. A test that does this gets a fatal "componentById called with
    component management disabled", nodes left without positions, or a hang.
    It can't happen in the app, as `Document` pauses the layout whenever a
    command starts, and the graph only changes in commands.
  - Either make the resume wait for `graphChanged`, or remove the branch and
    assert that the layout is paused. Which?
- **Picking the node nearest a click.**
  - Now: `nodeIdInsideFrustumNearestPoint`, in `graphcommoninteractor.cpp`,
    scores each node in the picking frustum by its distance from the clicked
    point, plus the distance of the clicked point from the frustum's centre
    line. The second term is the same for every node, so has no effect;
    presumably it was meant to be the node's distance from the centre line,
    to favour nodes near the middle of the frustum.
  - Use the node's position? This changes which node a click picks.
- **Empty values in numeric column annotations.**
  - Now: in the correlation plot, an empty value in a column annotation whose
    other values are all numbers is drawn in the gradient's lowest colour,
    like the smallest value. In other annotations, empty values are left
    transparent (`QCPColumnAnnotations::resolveRects`).
  - Leave them transparent here too?
- **Missing values when sorting the node attribute table.**
  - Now: nodes with no value in the sorted column go last when sorting
    ascending, but first when descending, as the order is just reversed.
    Spreadsheets usually keep missing values last either way.
    `tst_nodeattributetablemodel` pins the current order.
  - Keep them last either way?

## Adjacency matrices

These are inconsistencies in how matrices are loaded. Changing any of them
changes the graphs existing files load as.

- **Which way edges go.**
  - Now: in a CSV adjacency matrix, the value at row r and column c creates
    an edge from c's node to r's. In a MATLAB matrix it's from r's node to
    c's, which is the more usual convention.
  - Make CSV go from r to c?
- **Node names in edge lists.**
  - Now: a table of two integer columns and a number column is loaded as an
    edge list, whose nodes are named "Node 1", "Node 2" and so on in the
    order they're found, not after the integers. So the ids in a CSV edge
    list are lost, and a sparse MATLAB matrix, which is loaded as an edge
    list, gets different node names from the same matrix stored densely.
  - Name the nodes after the integers?
- **MATLAB data is transposed.**
  - Now: `MatLabFileParser` puts each MATLAB column in a row of its table.
    That's why the matrix edges go from row to column, but where a MATLAB
    file is loaded as a table, such as for correlation data, a matrix with a
    row per gene gives a column per gene.
  - Stop transposing, and reverse the edges instead?
- **3 by 3 matrices.**
  - Now: a 3 by 3 adjacency matrix of integers can't be told apart from an
    edge list, and is loaded as one.
  - Leave it, or prefer the matrix when the values allow both?

## Known parser gaps, not fixed

These were found while testing, and left as they are. Should any of them be
fixed?

- **DOT:**
  - `strict` is ignored, so duplicate edges are kept.
  - In `{a -> b} -> c`, the edges to `c` are lost.
  - These fail to parse:
    - `"a" + "b"` string concatenation;
    - more than one attribute list, as in `a [x=1][y=2]`;
    - `[` inside HTML strings;
    - a quoted string ending in a backslash;
    - `#` preprocessor lines;
    - a UTF-8 byte order mark.
- **MATLAB:** complex data isn't detected, and is read as if it were real.
- **JSON Graph:**
  - A graph with no `edges` key is rejected, though the key is optional.
  - Numeric node ids, e.g. `"id": 1`, are rejected.
  - A node's label is used as its name, but not stored as a `NodeName`
    attribute.
- **GraphML:** nodes in nested graphs are dropped. The code has a FIXME for
  this.
- **GML:** a repeated `id` silently creates two nodes, and a label that isn't
  a string is dropped.
- **Correlation:** `CorrelationFileParser::epsilonFor` returns `DBL_MAX / 2`
  when the data has no positive values.
- **Visualisations:** a string parameter ending in a backslash can't be read
  back, as its closing quote is taken to be escaped.

## The pull requests

`claude/bug-fixes` has 113 fixes, and `claude/unit-test-infrastructure` has
233 commits of tests and infrastructure on top of it. Both are based on
`i18n`, which `master` now matches.

- When should these become pull requests?
- One PR for both branches, or the fixes first, then the tests?
