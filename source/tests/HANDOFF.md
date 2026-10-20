# Unit test work: notes for continuing elsewhere

This describes the state of the unit test work on the
`claude/unit-test-infrastructure` branch, so that it can be picked up in a new
session, for example Claude Code on the desktop. Read this first, then
`PENDING_DECISIONS.md`, which lists questions waiting on Tim. Delete both files
before the branch is merged.

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
  - Each fix is its own commit, which also adds the test that exercises it.
  - Check every fix by building and running the test without it and seeing it
    fail, then with it and seeing it pass. Do this by temporarily reverting
    only the source change, e.g. with `git stash push <source paths>`.
- **Test commits:** a commit adding tests for an area comes before the fix
  commits for that area, and must pass on the code as it was. Each fix commit
  then adds the checks that fail without it.
- **Behaviour changes:** if a fix changes behaviour that could have been
  intentional, add it to `PENDING_DECISIONS.md` rather than asking.
- **CI:** watch it and fix any failures. Fold CI fixes into the earlier commits
  they belong to, rather than adding "fix CI" commits. Rewriting history on
  this branch is fine; push with `--force-with-lease`.
- **No pull request** until Tim asks for one. The branch has around 70
  commits, and where to draw the line for a PR is one of the pending
  decisions.
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
  transforms and visualisations with the real parsers, so that the QML tests
  can stand in for a `Document` with a small `QtObject`.
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
| `tst_conditionfncreator` | every condition operator and type |
| `tst_structuraltransforms` | filters, contraction, edge reduction and so on |
| `tst_analysistransforms` | clustering, centrality, k-NN and so on |
| `tst_attributetransforms` | attribute synthesis, combination, averaging and so on |
| `tst_enrichmentcalculator` | Fisher's exact test against an exact calculation, over-representation on a small graph |
| `tst_enrichmenttablemodel` | sorting, filtering and resets |
| `tst_correlation` | correlation types, threshold, k-NN for every type, matrices, cancellation, the k-NN proto graph, normalisation, scaling, clipping, imputation (expected values from numpy) |
| `tst_tabulardata` | CSV, TSV, SSV, TXT and XLSX tokenising and detection, the data rectangle the correlation plugin finds, row and column types and duplicates, key match percentages |
| `tst_graphparsers` | pairwise, GML, GraphML, DOT, JSON Graph, CX, BioPAX, adjacency matrix and MATLAB files (written with matio by the test): valid, precise, malformed, syntax error locations, missing values |
| `tst_savers` | GML, GraphML, JSON Graph and pairwise export, then re-import |
| `tst_nativeloader` | native files: every format version, Tutorial.graphia, malformed files, save then load |
| `tst_commands` | execute, undo and redo of delete nodes, the edit, clone, remove and import attribute commands, and applying transforms, including the deferred form and cancelling it |
| `tst_visualisationconfigparser` | visualisation config round trips, malformed configs, the display form |
| `tst_qml` | `Utils.js`, `AttributeUtils.js`, `MenuUtils.js`, `VisualisationUtils.js`, and `TransformConfig.js`: building a transform's controls (including attribute parameters), then its expression, gives back the same transform; locked transforms' text |
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
| `tst_tabulardataparser` | the parsers the table import and matrix dialogs use: headers for the key column by type, duplicates, failures, and the matrix size estimate |
| `tst_headless` | headless mode, end to end with the real plugins and `Document`: GML, pairwise and correlation files to native files, which are loaded back, and failures |
| `tst_document` | `Document`, as the UI uses it: loading, transforms (applied, edited, moved, removed, undone), selection, Find, deleting nodes, bookmarks, attribute commands, an edited attribute reapplying transforms, visualisations, saving and loading again, the edit attribute table, finding the key for and importing attributes, enrichment results, what the transform and visualisation panels ask of it, exporting tables, saving and loading node positions, the transforms offered when adding one, selecting by value, stepping through found nodes, moving visualisations |
| `tst_correlationplugin` | the correlation plugin through `Document`: the graph, correlation values, default transforms, statistics, column annotations, the panel's table, threshold, polarity, type and k-NN parameters, saving and loading, hierarchical clustering, rows of interest, importing annotations, discrete and transposed data, the log; the import dialog's parser (data rectangle, transposing, graph size estimates) and annotation key detection, created through QML |
| `tst_random` | the random seed, which `TestEnvironment` sets so that anything random is reproducible |
| `tst_crypto` | RSA signing and verification with keys generated by the test, RSA and AES encryption |
| `tst_string`, `tst_statistics`, `tst_container`, `tst_datastructures`, `tst_json`, `tst_color` | `shared/utils`: strings and natural order, statistics (against numpy), containers and set operations, `CircularBuffer`, `FixedSizeStack`, `Flags`, JSON conversions, colours |

## Bugs fixed so far

Each one is a commit; see `git log` for the details.

- **Build:** the Windows Clang manifest.
- **Conditions:** 4 fixes, including string operators.
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
  - remove's undo lost the description and column order, and removing user
    data used a freed name, so on macOS undo didn't restore it at all;
  - an import that only replaces columns couldn't be undone.
- **Transform configs:** quotes in condition strings weren't escaped when
  transforms were rewritten on loading, so the transform was lost.
- **Transform panel (QML):**
  - editing a value in a condition on an attribute with `%` and a digit in
    its name changed the name;
  - labels with quotes or backslashes lost them, or failed with an
    exception;
  - attribute names starting with "source" or "target" were capitalised;
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
- **Exported tables:** values with line breaks split their rows.
- **Node positions files:** a non-numeric id or coordinate crashed Graphia.
- **Edit attribute table:** a value edited back to what it was kept the
  earlier edit, which was shown and applied.
- **Visualisation configs:**
  - whole number parameters couldn't be parsed;
  - numbers were written with 6 significant figures;
  - quotes in attribute names weren't escaped.

## Gotchas

- **CryptoPP's assertions** are enabled in debug builds, where `_DEBUG` is
  defined, and raise SIGTRAP; the Clang Debug CI job runs the tests that way,
  so a test that only passes in release builds fails there.
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
  - `AttributeUtils.prettify` puts thin spaces (U+2009) around `›`.
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

1. **Coverage gaps:** see "Coverage" below.
2. **More of `Document`:** `tst_document` makes it possible to test
   the other QML facing models that need a loaded `Document`.
3. **Later:** the corpus comparison of release and candidate builds.

The original plan, for reference, had these priorities:
1. wrong analysis results (config grammars, conditions, transforms,
   correlation, enrichment);
2. crashes and loading and saving (parsers, native files, savers, graph core,
   commands);
3. UI logic (JavaScript helpers, one `Document` level test);
4. `shared/utils`.

Its mutation check plants deliberate bugs (an off-by-one in PageRank, a
flipped operator in `condtionfnops.h`, a changed default in the pairwise
parser, a dropped field in `nativesaver.cpp`) and expects each to make a test
fail.

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

Line coverage was about 57% at the last measurement. The biggest gaps
outside rendering and UI code are the parts of `Document` that
`tst_document` doesn't use yet, the updater (the update manifest is signed
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

`MCLTransform`'s second recovery step, after selecting the 1100 largest
values of a column, takes `nth_element` at 1400 even when the column has
fewer than 1400 values. In practice that keeps every value, which is what
recovery wants, but it's undefined; it needs a node with over 1100
neighbours after expansion to reach.

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
