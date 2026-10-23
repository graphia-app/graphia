# Pending decisions from the unit test work

These came up while writing the tests on `claude/unit-test-infrastructure`.
Most have now been decided, and the changes decided on have been made; what's
left are the questions deferred for later. Delete this file once they're
resolved.

Fixes are on `claude/bug-fixes`, one commit each, and each is tested by a
commit on the test branch titled "Test the fix for ...".

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
