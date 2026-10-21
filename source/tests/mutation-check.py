#!/usr/bin/env python3
# Plants deliberate bugs in the source, one at a time, and checks that the
# tests that should catch each one fail. Each file is restored afterwards.
#
# Usage: mutation-check.py <build directory>
#
# The build directory must already be configured with GRAPHIA_BUILD_TESTS; the
# test programs named for each mutation are rebuilt with it, then run.

import os
import subprocess
import sys

SOURCE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (description, file, original text, mutated text, test programs)
MUTATIONS = [
    ("PageRank teleport term", "app/transform/transforms/pageranktransform.cpp",
     "((1.0f - PAGERANK_DAMPING) / static_cast<float>(componentNodeCount))",
     "((1.0f - PAGERANK_DAMPING) / static_cast<float>(componentNodeCount + 1))",
     ["tst_analysistransforms"]),
    ("Condition < becomes <=", "app/attributes/conditionfncreator.cpp",
     "return (attribute.*valueOfFn)(elementId) < value; };",
     "return (attribute.*valueOfFn)(elementId) <= value; };",
     ["tst_conditionfncreator"]),
    ("Pairwise source and target swapped", "shared/loading/pairwisefileparser.cpp",
     "auto sourceNodeId = nodeIdFor(valueAt(sourceNodeColumn, rowIndex));\n"
     "        auto targetNodeId = nodeIdFor(valueAt(targetNodeColumn, rowIndex));",
     "auto sourceNodeId = nodeIdFor(valueAt(targetNodeColumn, rowIndex));\n"
     "        auto targetNodeId = nodeIdFor(valueAt(sourceNodeColumn, rowIndex));",
     ["tst_graphparsers", "tst_savers"]),
    ("Native files lose bookmarks", "app/loading/nativesaver.cpp",
     "    content[\"bookmarks\"] = bookmarksAsJson(_content._bookmarks);\n", "",
     ["tst_nativeloader", "tst_document"]),
    ("Native files lose the node size", "app/loading/nativesaver.cpp",
     "    content[\"nodeSize\"] = graphModel->nodeSize();\n", "",
     ["tst_nativeloader", "tst_document"]),
    ("k-NN keeps one edge too many", "app/transform/transforms/knntransform.cpp",
     "std::min(k, edgeIds.size())", "std::min(k + 1, edgeIds.size())",
     ["tst_attributetransforms"]),
    ("Betweenness isn't halved", "app/transform/transforms/betweennesstransform.cpp",
     "nodeBetweenness[nodeId] += arrays.nodeBetweenness[nodeId] * 0.5;",
     "nodeBetweenness[nodeId] += arrays.nodeBetweenness[nodeId];",
     ["tst_analysistransforms"]),
    ("Pearson numerator sign", "plugins/correlation/correlation.cpp",
     "(static_cast<double>(size) * productSum) - (vectorA->sum() * vectorB->sum())",
     "(static_cast<double>(size) * productSum) + (vectorA->sum() * vectorB->sum())",
     ["tst_correlation"]),
    ("Inverting the selection does nothing", "app/ui/selectionmanager.cpp",
     "    _toggleNodes(_selectedNodeIds, _nodeIdsMask, _graphModel->graph().nodeIds());\n", "",
     ["tst_selectionmanager"]),
    ("Component filters ignore inversion", "app/transform/transforms/filtertransform.cpp",
     "if(u::exclusiveOr(conditionFn(*component), _invert))", "if(conditionFn(*component))",
     ["tst_structuraltransforms"]),
    ("Undoing deleting nodes loses their edges", "app/commands/deletenodescommand.cpp",
     "            graph.addEdges(_edges);\n", "",
     ["tst_commands", "tst_document"]),
    ("MCL prune limit raised", "app/transform/transforms/mcltransform.cpp",
     "static const float MCL_PRUNE_LIMIT = 1e-4f;", "static const float MCL_PRUNE_LIMIT = 1e-1f;",
     ["tst_analysistransforms"]),
]


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <build directory>")
        return 1

    build = os.path.abspath(sys.argv[1])
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", QTEST_FUNCTION_TIMEOUT="120000")
    survivors = 0

    for description, fileName, original, mutated, tests in MUTATIONS:
        path = os.path.join(SOURCE, fileName)
        with open(path, encoding="utf-8") as f:
            content = f.read()

        if content.count(original) != 1:
            print(f"{description}: the original text wasn't found once in {fileName}")
            survivors += 1
            continue

        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(content.replace(original, mutated))

            targets = tests + (["correlation"] if fileName.startswith("plugins/correlation") else [])
            if subprocess.run(["cmake", "--build", ".", "--target"] + targets, cwd=build, env=env,
                    capture_output=True).returncode != 0:
                print(f"{description}: the build failed")
                survivors += 1
                continue

            caughtBy = [test for test in tests if subprocess.run(
                [os.path.join(build, "source", "tests", test)], cwd=build, env=env,
                capture_output=True).returncode != 0]

            if caughtBy:
                print(f"{description}: caught by {', '.join(caughtBy)}")
            else:
                print(f"{description}: NOT CAUGHT")
                survivors += 1
        finally:
            with open(path, "w", encoding="utf-8") as f:
                f.write(content)

    print(f"{len(MUTATIONS) - survivors} of {len(MUTATIONS)} caught")
    return 0


if __name__ == "__main__":
    sys.exit(main())
