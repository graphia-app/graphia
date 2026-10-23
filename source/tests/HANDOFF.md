# Unit test work: notes for continuing elsewhere

This describes the state of the unit test work, so that it can be picked up
in a new session, for example Claude Code on the desktop. Read this first, then
`PENDING_DECISIONS.md`, which lists questions waiting on Tim. Delete both files
before the branch is merged.

The work is on two branches:
- **`claude/bug-fixes`** has the fixes for the bugs the tests found, and
  nothing else. It's based on Tim's `i18n` branch, and each fix is one commit
  with no test changes in it.
- **`claude/unit-test-infrastructure`** has the tests, the infrastructure for
  them and these notes. It's always rebased onto the tip of `claude/bug-fixes`.

## The i18n branch

`i18n` makes the names Graphia stores and parses separate from those it
shows, so they can be translated:
- Transforms, their parameters and the attributes Graphia declares have
  compact names, e.g. `RemoveNodes`, `RankOrder` and `NodeDegree`, with
  display names such as "Remove Nodes". Transform configs write names
  without quotes unless they need them, e.g.
  `RemoveNodes where $NodeDegree < 2 and $"Gene Type" == "x"`.
- Attributes loaded from files no longer have a "Node " or "Edge " prefix;
  one gets "Node " or "Edge " only when a node and an edge attribute would
  share a name, and "User " when it would clash with a declared attribute.
- `GraphModel` takes its transform and visualisation channel factories, and
  attribute declarations, from `Application`, so tests need one; see
  `TestGraphModel::application()`.
- Native files before version 9, and preferences, have their names upgraded
  when they're loaded (`nativeloader_legacy.cpp`, `legacynames.cpp`).

When `i18n` moves on, rebase `claude/bug-fixes` onto it, then the test branch
onto that. When the branches were first rebased onto it:
- the fixes for the CX2 parser and for quotes in condition strings were
  dropped, as `i18n` makes them too; their tests remain;
- the fix for `source`/`target` in `AttributeUtils.js` was dropped with its
  test, as `i18n` removes the file;
- the fix for quotes in visualised attribute names was replaced by one for
  `Attribute::parseAttributeName`, as writing names on `i18n` removed the
  quotes within them;
- `i18n`'s own fixes for removing attributes' undo (descriptions, and the
  freed name) left only the column order to fix.

## The goal

The aim is a comprehensive unit test suite that stops releases going
backwards, especially:
- wrong analysis results;
- crashes;
- files that no longer load or save correctly.

Unit tests are the centrepiece; Tim chose them over corpus or differential
testing, which may come later as an extra safety net.

## How Tim wants the work done

- **Bugs:** fix any bug that a test finds.
  - Each fix is its own commit on `claude/bug-fixes`, changing only the source,
    not the tests. Then rebase `claude/unit-test-infrastructure` onto it, and
    add the test that exercises the fix as its own commit on the test branch,
    titled "Test the fix for ...".
  - Check every fix by building and running the test without it and seeing it
    fail, then with it and seeing it pass. Do this by temporarily reverting
    only the source change, e.g. with `git stash push <source paths>`.
  - A fix must work without the test infrastructure. Where the test branch has
    refactored the code a fix changes, such as moving the correlation
    preprocessing into `correlationpreprocessing.cpp` or splitting
    `ApplyVisualisationsCommand::patchedVisualisations`, write the fix
    against the code as it is on `claude/bug-fixes`, then resolve the
    refactoring commit's conflict in the rebase so the moved code has the fix.
  - Push both branches with `--force-with-lease` after the rebase. The test
    branch's tree must be the same before and after a rebase that only brings
    in fixes already on it; check with `git diff`.
- **Test commits:** a commit adding tests for an area comes before the commits
  testing fixes for that area. On the test branch every fix is already
  underneath, so the tests pass on every commit.
- **Behaviour changes:** if a fix changes behaviour that could have been
  intentional, add it to `PENDING_DECISIONS.md` rather than asking.
- **CI:** watch it on both branches and fix any failures. Fold CI fixes into
  the earlier commits they belong to, rather than adding "fix CI" commits.
  Rewriting history on these branches is fine; push with `--force-with-lease`.
  A fold into `claude/bug-fixes` means rebasing the test branch again.
- **No pull requests:** Tim has decided not to open any for these branches.
- **Commit messages:** a short imperative subject, then a body explaining the
  problem and the fix in prose. End with the attribution trailers used
  throughout the branch, and don't put model names or identifiers in commits or
  code.
- **Code style:** test code should look like the surrounding code: Qt idioms,
  `u""_s` literals, Allman braces, comments only where something isn't
  obvious.

## Building and running the tests

- **Option:** tests are built when `GRAPHIA_BUILD_TESTS` is on, which is the
  default except for WebAssembly.
- **Sanitizers:** `GRAPHIA_SANITIZERS`, e.g. `address;undefined`, builds
  everything with them (not MSVC). The Clang Debug CI job uses it, through
  `linux-build.sh`'s `SANITIZERS` variable. The whole suite was clean under
  GCC's ASan, UBSan and LeakSanitizer when this was added.
- **Standard library checks:** `GRAPHIA_STDLIB_ASSERTIONS` builds with
  `_GLIBCXX_ASSERTIONS`, which the Clang Debug CI job uses (`STDLIB_ASSERTIONS`
  in `linux-build.sh`). The fuller `_GLIBCXX_DEBUG`, which also checks
  iterators, crashes with a Qt not built with it, as it changes the layout of
  `std::map`, which `QMap` uses. Building locally with it, by adding
  `-D_GLIBCXX_DEBUG` to the flags, still runs the tests that don't destroy a
  `QVariantMap` made by our code, and found the transform cache and
  Winsorization bugs.
- **Locales:** numbers are formatted in the system locale, which on Linux Qt
  takes from `LC_ALL` and the like, so the suite can be run in another
  locale with, e.g., `LC_ALL=sv_SE.UTF-8 ctest`. `tst_string` is also run in
  that locale on Linux. macOS and Windows CI run in en_US.
- **Running:** after a normal CMake configure and build, run `ctest` in the
  build directory, or run a single program, e.g. `source/tests/tst_commands`,
  optionally with a test function name.
- **Headless:** set `QT_QPA_PLATFORM=offscreen`.
- **Qt:** CI uses Qt 6.11.2 and every build job runs `ctest`. Clang-Tidy,
  cppcheck, clazy, IWYU and qmllint also run on every push, so keep new code
  clean for them.

## Infrastructure

- **`appcore`:** an OBJECT library in `source/app/CMakeLists.txt` with all the
  application sources except `main.cpp`. Tests link it.
- **`correlationcore`:** a static, PIC library in
  `source/plugins/correlation/CMakeLists.txt` with the correlation plugin's
  maths and its CSV preprocessing (`loading/correlationpreprocessing.cpp`).
  `tst_correlation` links it.
- **`add_graphia_test`:** registers each test program in
  `source/tests/CMakeLists.txt`.
- **`tests/common/testenvironment.h`:** `TestEnvironment::initialise()`, called
  from `initTestCase`, sets up the thread pool, `ScopeTimerManager` and a
  temporary `QSettings` location.
- **`tests/common/testplugin.h`:**
  - `TestPlugin`, a minimal plugin;
  - `TestPluginInstance`, which records the plugin data it's given to load and
    saves the same data back.
- **`tests/common/testgraphmodel.h`:** `TestGraphModel`, which wraps a
  `GraphModel`.
  - `TestGraphModel::application()` is the one `Application` of the test,
    which every `GraphModel` the tests make needs. Creating it queries OpenGL,
    so tests that use it have a `QGuiApplication`, through `QTEST_MAIN`,
    rather than `QTEST_GUILESS_MAIN`. It loads no plugins, so attributes that
    plugins declare, such as `NodeName`, have no display names.
  - It's built either from a compact description, e.g. `{"a-b", "b-c", "d"}`,
    or by `TestGraphModel::parse(content, createParser, &failureReason)`, which
    parses a file the way `ParserThread` and `Document` do. `parse` keeps the
    parser, so it can be queried with `parser<T>()`.
  - Helpers:
    - names: `nodeNames`, `edgeNames`, `nodeId`, `edgeId`;
    - values: `nodeValue`, `edgeValue`, `nodeNumericValue`,
      `edgeNumericValue`, `nodeHasValue`, `nodePartition`;
    - set-up: `addNodeAttribute`, `addEdgeAttribute`, `applyTransforms`;
    - `state()`, which describes the whole graph and every attribute value
      exactly, for checking that a change and its undo leave everything as it
      was.
- **`NativeSaver::Content`:** holds the document state that `NativeSaver`
  saves, which `NativeSaverFactory` gathers from the `Document`. Tests can
  therefore save native files without a `Document`.
- **`AttributeEdits`:** has public setters, so tests can create edits.
- **`tests/common/testdocument.h`:** `TestDocument`, an `IDocument` for a
  `TestGraphModel`, as plugins see the `Document`; it records what would
  move the camera or be shown to the user.
- **Real plugins:** `tst_headless` sets `Application::setAppDir` to the
  build directory, from `GRAPHIA_BUILD_DIR`, so that `Application` loads the
  plugins built in its `plugins` directory, and depends on them in CMake.
  Native files written by the real plugins are loaded with a `TestPlugin`
  whose data version is the maximum.
- **A whole `Document`:** `tst_document` gives a `Document` a `GraphDisplay`
  that isn't in a window, so nothing is rendered and OpenGL isn't needed,
  then invokes the display's private `onRendererInitialised` slot, as the
  renderer would, after which the `Document` behaves as in the UI, layout
  included. Wait for `busyChanged` after that, as the `Document` only
  pauses the layout for commands once it has seen the change. Commands are
  asynchronous, so the test's `execute` waits for `commandsFinished` and
  then for the `Document` not to be busy. Destroying a `Document` while a
  command waits on the main thread deadlocks, as the UI never does that.
- **`tests/common/documenttest.h`:** `DocumentTest`, the base class of
  `tst_document` and `tst_correlationplugin`, which opens files in a
  `Document` with the real plugins and waits for its commands. A plugin's
  own functions, such as the correlation plugin's, can't be linked, so are
  called through Qt's meta-object system, as QML calls them.
- **A bare `Document`:** `tst_commands` creates a `Document` with nothing
  loaded, which is enough for `ApplyTransformsCommand`, and connects its
  `taskAddedToExecutor` signal to its `executeDeferred` slot so that what
  commands ask of the main thread is done at once. A loaded `Document` isn't
  usable without a `GraphDisplay` that has a renderer, which needs OpenGL;
  without one, `onLoadComplete` stops early, and `busy()` dereferences the
  display.
- **`tst_qml`:** runs the QML tests in `source/tests/qml` with Qt Quick Test.
  `tst_qml.cpp` gives them a `testHelper` context property, which parses
  transforms and visualisations with the real parsers, and writes and
  displays names and conditions with the real code, so that the QML tests can
  stand in for a `Document` with a small `QtObject`.
- **`defineDefaultPreferences`:** in `app/defaultpreferences.cpp`, defines
  every preference with its default; `main()` and
  `TestEnvironment::initialise()` both call it.
- **`lexyutils::describeFirstError`:** in `source/thirdparty/lexy_utils.h`,
  gives the line and column of the first lexy parse error.

## What's covered

| Program | Covers |
|---|---|
| `tst_mutablegraph` | graph editing and consistency |
| `tst_graphtransformconfigparser` | transform config grammar round trips |
| `tst_attribute` | attributes of nodes, edges and components: value, missing value and set value functions, ranges, shared values, parameters, names, an edge's nodes' attributes |
| `tst_conditionfncreator` | every condition operator and type, with attributes or values on either side |
| `tst_structuraltransforms` | filters, contraction, edge reduction and so on |
| `tst_analysistransforms` | clustering, centrality, k-NN and so on |
| `tst_attributetransforms` | attribute synthesis, combination, averaging and so on |
| `tst_enrichmentcalculator` | Fisher's exact test against an exact calculation, over-representation on a small graph |
| `tst_enrichmenttablemodel` | sorting, filtering and resets |
| `tst_correlation` | correlation types, threshold, k-NN for every type, matrices, cancellation, the k-NN proto graph, normalisation, scaling, clipping, imputation (expected values from numpy) |
| `tst_columnannotations` | the correlation plot's column annotations: empty values are left transparent |
| `tst_tabulardata` | CSV, TSV, SSV, TXT and XLSX tokenising and detection, the data rectangle the correlation plugin finds, row and column types and duplicates, key match percentages |
| `tst_graphparsers` | pairwise, GML, GraphML, DOT, JSON Graph, CX, BioPAX, adjacency matrix and MATLAB files (written with matio by the test): valid, precise, malformed, syntax error locations, missing values |
| `tst_savers` | GML, GraphML, JSON Graph and pairwise export, then re-import |
| `tst_nativeloader` | native files: every format version, Tutorial.graphia, malformed files and values of the wrong type, save then load |
| `tst_commands` | execute, undo and redo of delete nodes, the edit, clone, remove and import attribute commands, and applying transforms, including the deferred form and cancelling it |
| `tst_visualisationconfigparser` | visualisation config round trips, malformed configs, the display form |
| `tst_qml` | `Utils.js`, `MenuUtils.js`, `VisualisationUtils.js`, and `TransformConfig.js`: building a transform's controls (including attribute parameters), then its expression, gives back the same transform; locked transforms' text, with display names |
| `tst_colorvisualisations` | colour gradients and palettes, including malformed ones |
| `tst_visualisationmapping` | the range values are mapped from, and the exponent |
| `tst_searchmanager` | Find: case, whole words, regular expressions, chosen attributes, merged nodes, refreshing |
| `tst_commandmanager` | the undo stack: undo, redo, replace, once, failures, maximum levels, rollbacks, queued commands |
| `tst_graphsizeestimate` | the threshold and k-NN graph size estimates plotted when importing |
| `tst_visualisations` | building visualisations on a graph and the resulting visuals: gradients, palettes, inversion, mappings, sizes, text |
| `tst_selectionmanager` | selecting, deselecting, toggling, inverting, masks, merged nodes, removed nodes, signals |
| `tst_geometry` | bounding boxes and spheres, including ray intersection, rays, lines, planes, frustums, circles |
| `tst_layout` | node positions, layout settings, the simple, fast initial and force directed layouts, Barnes-Hut trees against exact forces, component layouts |
| `tst_genericplugin` | which file types files are identified as, the default transforms and visualisations of the generic plugin |
| `tst_layoutthread` | the layout thread, driven as `Document` drives it: running to completion, 2D, pausing, components added and removed, settings |
| `tst_nodeattributetablemodel` | the table of node attributes in plugins' panels: columns, values, removed nodes, selection, attributes added, removed and edited, related rows; the display model the table view shows: rows, hidden and ordered columns, sorting, row order, rows selected in the table |
| `tst_modelutils` | `SortFilterProxyModel` (sorting by role, both orders, ties, filtering) and `ModelCompleter` (candidates, common prefix, closest match) |
| `tst_transformcache` | invalidating cached transform results when attributes change |
| `tst_tabulardataparser` | the parsers the table import and matrix dialogs use: headers for the key column by type, duplicates, failures, and the matrix size estimate |
| `tst_headless` | headless mode, end to end with the real plugins and `Document`: GML, pairwise and correlation files to native files, which are loaded back, and failures |
| `tst_document` | `Document`, as the UI uses it: loading, transforms (applied, edited, moved, removed, undone), selection, Find, deleting nodes, bookmarks, attribute commands, an edited attribute reapplying transforms, visualisations, saving and loading again, the edit attribute table, finding the key for and importing attributes, enrichment results, what the transform and visualisation panels ask of it, exporting tables, saving and loading node positions, the transforms offered when adding one, selecting by value, stepping through found nodes, moving visualisations, `Application`'s file type identification, failure reasons, links and resource files, the tree of attributes in attribute lists, an edge's nodes' attributes in transforms, matrices with the dialog's options, GraphML, DOT, JSON Graph and pairwise TSV through the generic plugin |
| `tst_correlationplugin` | the correlation plugin through `Document`: the graph, correlation values, default transforms, statistics, column annotations, the panel's table, threshold, polarity, type and k-NN parameters, saving and loading, hierarchical clustering, rows of interest, importing annotations, discrete and transposed data, the log, files saved by every earlier version of its data, malformed data, saving after deleting a node; the import dialog's parser (data rectangle, transposing, graph size estimates) and annotation key detection, created through QML |
| `tst_random` | the random seed, which `TestEnvironment` sets so that anything random is reproducible |
| `tst_crypto` | RSA signing and verification with keys generated by the test, RSA and AES encryption |
| `tst_string`, `tst_statistics`, `tst_container`, `tst_datastructures`, `tst_json`, `tst_color` | `shared/utils`: strings and natural order, statistics (against numpy), containers and set operations, `CircularBuffer`, `FixedSizeStack`, `Flags`, JSON conversions, colours |

## Bugs fixed so far

Each one is a commit; see `git log` for the details.

- **Build:** the Windows Clang manifest.
- **Conditions:** 5 fixes, including string operators, and comparing
  an integer with a float as text.
- **Transforms:**
  - betweenness, 2 fixes;
  - edge reduction;
  - the average attribute.
- **Correlation:**
  - SMC;
  - Spearman ties;
  - the k-NN proto graph, 3 fixes;
  - mean and standardisation normalisation;
  - unit scaling;
  - quantile ties;
  - column average imputation.
- **CSV:** escaped quotes.
- **Parsers:**
  - pairwise blank or one-sided rows;
  - CX2 attributes;
  - JSON and CX type errors;
  - BioPAX nesting and duplicate edges;
  - adjacency matrix headers;
  - number precision;
  - integers too large for an `int`;
  - GraphML split text, `for="all"`, `<default>` and edges before nodes;
  - DOT scoped defaults and case-insensitive keywords;
  - GML entities and comments;
  - failure reasons.
- **Export:**
  - GraphML double escaping;
  - JSON Graph node names;
  - pairwise `%1` in names and pairwise quotes;
  - number precision.
- **Native files:**
  - node ids not in the graph writing out of bounds;
  - 2D shading not restored;
  - enrichment results lost on saving;
  - enrichment rows with the wrong number of values failing when shown.
- **Visualisation parameters:** malformed colour gradients, palettes and
  mappings threw uncaught exceptions.
- **Enrichment:**
  - Fisher's exact test excluded tables tied with the observed one, roughly
    halving many p-values;
  - the table model reset itself twice when its data was set.
- **User data:**
  - missing numeric values read as 0, rather than as missing;
  - editing a numeric value to nothing was ignored.
- **Commands:**
  - edit undo restored rounded or invented values;
  - clone rounded floats;
  - remove's undo lost the column order;
  - an import that only replaces columns couldn't be undone.
- **Transform panel (QML):**
  - editing a value in a condition on an attribute with `%` and a digit in
    its name changed the name;
  - labels with quotes or backslashes lost them, or failed with an
    exception;
  - menu items leaked whenever a menu was cleared and rebuilt.
- **Visualisations for created attributes:** when a transform's attribute was
  given a unique name, a name with `(` or `+` stopped its visualisation
  following it. `ApplyVisualisationsCommand::patchedVisualisations` is now a
  static function that tests can call without a `Document`.
- **`shared/utils`:**
  - `u::findUniqueName` mangled names containing `%` and a digit;
  - `u::numericCompare` crashed on a null string, and ordered numbers too
    large for an `int` as text;
  - `u::setsDiffer` ignored values only in its second argument;
  - `u::valuesFor` didn't compile for maps;
  - scientific notation wasn't used in most locales, e.g. on Windows and
    macOS;
  - the `std::string` number checks accepted "1e" and "-".
- **Visualisations:** inverting didn't reflect values about the middle of the
  mapping range.
- **Find:** attributes created after a search began weren't searched when it
  was refreshed.
- **Undo:** a rollback after an undo left the wrong command redoable.
- **MATLAB files:** the first variable with a matrix was loaded, rather than
  the biggest, and the file was left open.
- **Graph size estimates:** k-NN estimates overflowed with the default sizes,
  and a k of 0 crashed.
- **Geometry:** `Ray::distanceTo` measured from the world origin.
- **Signature verification:**
  - a key file that wasn't a public key threw an uncaught exception;
  - a signature that was too short was verified against uninitialised
    memory, and trapped in debug builds.
- **Selection:** `SelectionManager::unselectedNodes` corrupted memory.
- **Adjacency matrices:** the initial edge filter threshold was written with
  6 significant figures.
- **Layouts:**
  - the fast initial layout put the first two children of its starting node
    in the same place;
  - Barnes-Hut trees ignored all but one of the nodes in the same place.
- **Cancelling applying transforms:**
  - a cancel that arrived when no rebuild was running, e.g. while "Apply
    Transforms and Visualisations" applied the visualisations, stopped every
    later rebuild, so the graph stopped following changes;
  - the cancelled transforms stayed in the transformed graph, and were
    applied the next time the graph changed.
- **Pairwise files:**
  - configured columns the file didn't have, e.g. in a one-column file,
    crashed the parser;
  - in headless mode, with no columns given, every row became a self loop.
- **Key detection:** the match percentages for importing attributes or
  annotations undercounted repeated values, and counted missing values as
  matches.
- **Layout thread:**
  - publishing positions deadlocked with a visual update on the main
    thread, as they took two locks in opposite orders; loading a file could
    hang;
  - pausing the layout just after resuming it, before the thread woke up,
    left it running, so it could read the graph as a command changed it.
- **Correlation plugin:**
  - k-NN graphs kept whichever of equally strong correlations came first,
    which varied from run to run;
  - importing a column annotation showed no name in the status, and
    whether one replaced an existing annotation depended on an unrelated
    column's type;
  - negative polarity's default transform filtered on the signed value
    against a positive threshold, so removed nothing;
  - the log named the continuous metric for discrete data.
- **Matrix size estimate:** with headers on only one side, the estimate
  shown when opening a matrix shifted every edge by one.
- **Visualisations panel:** moving a visualisation was checked against the
  number of transforms, so some moves were ignored.
- **Node context menu:** "Select Shared Values of '<node>'" used the
  selection's values rather than the node's (a QML fix, not covered by a
  test, as the suite can't load `TabUI.qml`).
- **Resource files:** any file beside Graphia's own folders, such as
  anywhere in Program Files, counted as one of Graphia's own, so one called
  Tutorial started the tutorial, and none were added to the recent files.
- **QML lists:** descending sorts left rows that compare equal, such as
  attributes of the same element type, in no particular order.
- **Completions:** the common prefix of a list's search matches could be
  longer than a shorter match (not visible, as only its presence is used).
- **JSON Graph:** files in version 2 of the format, whose nodes are keyed
  by id, failed to load.
- **Headless mode:** checked that the current directory, not the target's,
  could be written to; parameters of the wrong types, such as a numeric
  type, aborted Graphia.
- **Transform cache:** invalidating results compared iterators the erase
  had invalidated, and discarded results unaffected by the change.
- **Winsorization:** dereferenced the end of the data, which aborts with
  checked iterators (MSVC debug builds).
- **Exported tables:** values with line breaks split their rows.
- **MCL:** recovering pruned values read beyond a column's values, and in
  graphs of 1400 nodes or fewer beyond the end of its indices.
- **Commands:** destroying a Document while a command was queued behind
  another hung (the UI waits for commands before closing a tab, so this
  mostly affected the tests).
- **Deleting a merged node** from its context menu deleted only the node
  it was shown as, leaving the rest of the merged nodes in its place.
- **Native files:** a value of the wrong type, in the header (even when
  the file was only being identified), the body or the plugin's data,
  aborted Graphia.
- **Correlation plugin data:** values for edges that aren't in the graph
  were written out of bounds; partial or no rows asserted.
- **Node positions files:** a non-numeric id or coordinate crashed Graphia.
- **Edit attribute table:** a value edited back to what it was kept the
  earlier edit, which was shown and applied.
- **Visualisation configs:**
  - whole number parameters couldn't be parsed;
  - numbers were written with 6 significant figures;
  - quotes within attribute names were removed when names were written.

### Decided by Tim

These were in `PENDING_DECISIONS.md`, and Tim chose to change them.

- **Attribute Synthesis:** elements that don't match have no value, rather
  than an empty string or 0.
- **Clone Attribute:** attributes from the user's data are cloned for every
  node, including those a transform removed or merged.
- **Exports:** GML, GraphML and JSON Graph exports leave out the attributes
  Graphia computes from the graph (`GraphModel::attributeDeclarations`);
  pairwise exports write a row with one name for each node with no edges.
- **File attributes** named like an attribute the parser declares, such as
  `NodeName`, get a "User " prefix (`IUserData::undeclaredVectorName`).
- **String colours** use every hue, and are the same on every platform.
- **Conditions:** a string starting with `$` is a string; attribute names
  have their own type, `GraphTransformConfig::AttributeReference`, and QML is
  told which operands are names by `lhsIsName` and `rhsIsName`.
- **The layout thread** asserts that it's paused when a component is
  removed, rather than pausing and resuming itself (not tested, as it's an
  assertion).
- **Column annotations:** empty numeric values are transparent.
- **Node attribute table:** rows with no value are last in either order.
- **Adjacency matrices:** CSV edges go from the row's node to the column's,
  as MATLAB's did, and MATLAB data is no longer transposed.
- **DOT:** `strict`, edges to subgraphs containing edge statements, `+`
  concatenation, several attribute lists, `[` in HTML strings, strings ending
  in a backslash, `#` lines and byte order marks.
- **MATLAB:** matrices of complex numbers are ignored.
- **JSON Graph:** no `edges` key, integer ids, and labels as `NodeName`.
- **GML:** repeated ids fail, and numeric labels are used.
- **Correlation:** data with no positive values gets a tiny epsilon for log
  scaling, rather than a huge one.
- **Config strings:** a backslash escapes a quote or a backslash, so a string
  can end in one; older files' strings are upgraded to keep their meaning.

## Gotchas

- **CryptoPP's assertions** are enabled in debug builds, where `_DEBUG` is
  defined, and raise SIGTRAP; the Clang Debug CI job runs the tests that way,
  so a test that only passes in release builds fails there.
- **Helpers in test classes** go before `private slots:`. moc registers the
  types of slots' parameters and return values as metatypes, and registering
  `json` fails to compile with Windows Clang (Qt's `QDebug` operator check
  reaches the incomplete Windows `MSG` type through json's conversions).
- **moc** lexes raw string literals as ordinary strings, so an unbalanced quote
  or bracket inside `R"(...)"` breaks it. Use escaped normal strings for those.
- **Floats:**
  - `QCOMPARE` on floating point values is fuzzy, at about 1e-5 relative.
  - Attribute values are stored as `float`, so compare with
    `std::equal_to<>()(static_cast<float>(a), static_cast<float>(b))` where
    precision matters.
  - `Attribute::stringValueOf` formats floats with 6 significant figures,
    which is fine for display but loses precision anywhere values are copied
    or saved.
- **nlohmann::json** turns `{{"x", 1}}` into an object, not an array of
  arrays. Use `json::array(...)` explicitly.
- **Don't `QCOMPARE` json values.** On Windows with Clang this fails to
  compile, because Qt Test's check for a `QDebug` operator instantiates
  nlohmann's conversions with an incomplete Windows type. `tst_nativeloader`
  compares `jsonString(...)` values instead.
- **Missing values:** user data is stored as strings. An empty string is a
  missing value, and `UserData::value` converts to the vector's type, so check
  the stored string with `storedValueBy`.
- **Transformed graph:**
  - It's empty until `applyTransforms()` runs. `TestGraphModel::parse` calls
    `applyTransforms({})`.
  - A merged node set must be queried by its head node.
- **Parallel edges:** `TestGraphModel::_edgeIds` keeps only the first of
  several, though `edgeNames()` lists all of them.
- **Name hiding:** a test slot hides a free function with the same name, so
  write `::parse(...)` inside the class.
- **`QFINDTESTDATA`** finds files relative to the test source, which is how
  `tst_nativeloader` reads `app/examples/Tutorial.graphia`.
- **WebAssembly** disables exceptions, so prefer explicit checks to try/catch
  in production code.
- **Warnings:** `QTest::failOnWarning()` in `init()` makes a test fail on any
  Qt warning. `tst_enrichmenttablemodel` uses it.
- **QML tests:**
  - The JavaScript and QML files are compiled into the binary, so rebuild
    `tst_qml` after changing them.
  - Run one file with `tst_qml -input <path to .qml>`.
  - Display names of attribute parameters and an edge's nodes' attributes
    have thin spaces (U+2009) around `›`.
  - `TransformConfig.js` isn't a `.pragma library`, but
    `Qt.createComponent("TransformParameter.qml")` still finds the component
    when called from a test, so the real controls are created.
  - Objects with no parent are destroyed by the garbage collector in any
    order, which can produce spurious warnings; destroy them in `cleanup()`.
  - `ButtonGroup.addButton` doesn't set a button's attached
    `ButtonGroup.group`.
- **moc and raw strings:** besides unbalanced quotes and brackets, a `)"`
  inside the string, as in `R"("Name (1)")"`, ends it early for the
  compiler too, and `//` inside one can be taken by moc for a comment.
- **Plugins and AddressSanitizer:** the plugins and the test programs that
  load them each link their own copy of `shared`, which ASan reports as an
  ODR violation, so `detect_odr_violation` is turned off for those tests in
  sanitizer builds.
- **Races:** `tst_document` found two races in the layout thread that only
  showed up about once in 25 runs. Run new threaded tests many times, in
  Debug too, e.g. with `QTEST_FUNCTION_TIMEOUT` set low so that hangs end
  with a stack trace.
- **valgrind** works well on the Debug build and found a use-after-free that
  only failed on macOS. Set `QT_ENABLE_REGEXP_JIT=0`, otherwise
  `QRegularExpression`'s JIT produces false reports of uninitialised values.
  The whole suite is currently clean under it.

## Next steps, in rough order

1. **Tim's decisions:** the deferred questions in `PENDING_DECISIONS.md`.
   No pull requests are to be opened for these branches.
2. **Coverage gaps:** see "Coverage" below. What's left outside rendering
   is mostly in `Document` (component navigation and other things that need
   the renderer), the correlation plugin's plot helpers, and debug output, so
   returns are diminishing.
3. **The corpus comparison** of release and candidate builds needs the
   private corpus of real files, which the cloud sessions don't have.
4. **The mutation check** (below): add a mutation when a transform or format
   changes, and rerun it now and then.

The original plan, for reference, had these priorities:
1. wrong analysis results (config grammars, conditions, transforms,
   correlation, enrichment);
2. crashes and loading and saving (parsers, native files, savers, graph core,
   commands);
3. UI logic (JavaScript helpers, one `Document` level test);
4. `shared/utils`.

Its mutation check plants deliberate bugs and expects each to make a test
fail. `source/tests/mutation-check.py <build directory>` does this for 12 of
them, one at a time, restoring each file afterwards: an off-by-one in PageRank,
a flipped condition operator, swapped pairwise columns, fields dropped from
native files, k-NN keeping too many edges, betweenness not halved, the sign of
Pearson's numerator, inverting the selection doing nothing, component filters
ignoring inversion, undoing deleting nodes losing edges, and MCL's prune limit.
All but the last are caught. Raising MCL's prune limit from 1e-4 to 0.1 changes
the clusters of a ring of 24 nodes, but how a ring splits depends on floating
point tie-breaking, which could differ between compilers, so no test pins it.
Add mutations to the script when a transform or format changes.

## Coverage

A coverage build shows what the tests don't reach. The Clang Debug CI job
builds with `-DGRAPHIA_COVERAGE=ON` (Clang's source based coverage, for
everything but third party code), and its Coverage step summarises the
result in the job summary and uploads the full report, and an LCOV file, as
the `coverage` artifact. Locally, configure with Clang and
`-DGRAPHIA_COVERAGE=ON`, run `ctest` with `LLVM_PROFILE_FILE` set to e.g.
`$PWD/profiles/%m-%p.profraw`, then use `llvm-profdata merge` and
`llvm-cov report`, as in the workflow. `llvm-cov` warns of thousands of
"functions with mismatched data": inline and template code is instrumented
separately in each test program, and only one copy's counts are used, so
coverage of header code is understated. Alternatively, with GCC, configure a
Debug build with `--coverage` in the C, C++ and linker flags and
`-DCMAKE_UNITY_BUILD=OFF`, build the test targets, run `ctest`, then e.g.
`gcovr --gcov-executable <gcov> -r source --filter 'source/(app|shared|plugins)/'
--exclude '.*thirdparty.*' --exclude '.*_autogen.*' --gcov-ignore-errors=all
--gcov-ignore-parse-errors=all --txt cov.txt <build dir>`. Template code is
counted per instantiation, so files like `conditionfncreator.cpp` look less
covered than they are.

To see which lines of a file aren't covered, after merging the profiles,
run `llvm-cov show` with the first test program as a positional argument,
the rest with `-object`, and `-sources <file>`; passing a source file
without `-sources` makes `llvm-cov` take it for an object file.

Line coverage of `app`, `shared` and `plugins` was 63.7% at the last
measurement; most of the rest is rendering, interaction, plotting, the
updater and platform code. The biggest gaps outside those are the parts of
`Document` that need the renderer, the updater (the update manifest is signed
with a key that's compiled in, so tests can't sign one), and the rest of
the correlation plugin's instance (`correlationplugin.cpp`, used through
`Document` by `tst_correlationplugin`). XLSX files can be written by tests
with `tests/common/xlsxwriter.h`.

A plugin's QML types, such as the correlation plugin's
`CorrelationTabularDataParser`, are registered when the `Application`
loads the plugin, so a test can create them with a `QQmlEngine` and a
`QQmlComponent` importing the plugin's module; `tst_correlationplugin`
does.

`CommandManager` drops a command that reports itself cancelled, without
undoing it, so a command must roll itself back when cancelled.
`ApplyTransformsCommand` now does, but a cancel that arrives after it has
decided whether it was cancelled still drops it from the undo stack with its
transforms applied. The window is tiny, so this was left alone.

`CompoundCommand::undo` undoes each of its commands, even one whose
`execute` returned false, i.e. did nothing. No command executed as part
of an undoable compound returns false at the moment, so this was left.

Destroying a Document while a command it's executing waits for the main
thread (`Document::executeOnMainThreadAndWait`, used by refreshing
visualisations and others) would deadlock, as `~Document` joins the
command's thread before running deferred tasks. The UI waits for commands
to finish before closing a tab, so this was left.

`BarnesHutTree::differenceEpsilon` increments a `mutable` counter, and
`evaluateKernel` is called from several threads at once by the force
directed layout. That's a data race, though a harmless one in practice, as
the counter is always in range; a ThreadSanitizer build would report it.

## Checking CI

The `Build` workflow runs on every push to the branch. Look at the latest runs
for `claude/unit-test-infrastructure` on GitHub, or use the GitHub tools
available in the session, and read the logs of any failed job.

Every push also uploads the builds, about 400MB, to graphia.dev. In October
2026 the "Upload Builds to graphia.dev" job started failing with "No space
left on device" on the server, which has nothing to do with the branch, so
batch pushes rather than pushing every commit.

## Static analysis

CI's static analysis jobs (clang-tidy, clazy, cppcheck, include-what-you-use,
and the compilers' own warnings) don't fail the build. Their findings for each
branch are collected in the `graphia-app/defects` repository, on a branch of the
same name, e.g. `claude/unit-test-infrastructure`; `master`'s is empty, so
anything listed there came from the branch. Check it after pushing, and fix
what's listed, folding each fix into the commit that introduced the problem.

CI uses clang 19; clang-tidy 18 misses most of what 19 reports.
`scripts/static-analysis.sh` runs the same analysis locally, building the
clazy commit CI uses; it needs `clang-tidy-19`, `llvm-19-dev` and
`libclang-19-dev`. To run a tool on a few files by hand instead, note that the
local builds are unity builds, whose `compile_commands.json` has one entry per
batch of files; expand it into one entry per file, from the `#include`s in
each `Unity/unity_*.cxx`, first.

CI's clang-tidy goes through ctcache, which keys its results on each file's
preprocessed source, so without comments. Adding a `NOLINT` to a file whose
code doesn't otherwise change therefore changes nothing there: the cached
warning is reported again. Prefer changing the code so that the warning
doesn't apply.

Conventions that keep the tests clean:
- `QFETCH(const T, name)` and `const QSignalSpy`, unless they're modified.
- `reserve` before appending in a loop, or construct the container directly.
- Don't loop over a temporary Qt container; hold it in a `const` local first.
- Suppress false positives the way the rest of the codebase does, with
  `// clazy:exclude=...` or `// NOLINT check-name` on the line.
- moc misparses some raw strings, e.g. ones containing `</b>`, and then finds
  no classes, which shows up as an undefined vtable for the test class. Use an
  escaped string there, with `// NOLINT modernize-raw-string-literal`.
- MSVC wrongly sees escape sequences in raw strings in macro arguments, so put
  such strings in a variable outside `QCOMPARE` and friends.
