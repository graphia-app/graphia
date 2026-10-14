<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US" sourcelanguage="en_GB">
<context>
    <name>GenericPlugin</name>
    <message>
        <location filename="../genericplugin.h" line="+41"/>
        <source>Generic</source>
        <translation>Generic</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A plugin that loads generic graphs from a variety of file formats.</source>
        <translation>A plugin that loads generic graphs from a variety of file formats.</translation>
    </message>
</context>
<context>
    <name>MatrixParameters</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Generic/MatrixParameters.qml" line="+36"/>
        <source>Matrix Parameters</source>
        <translation>Matrix Parameters</translation>
    </message>
    <message>
        <location line="+96"/>
        <source>Failed to Load {0}</source>
        <translation>Failed to Load {0}</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>:

{0}</source>
        <translation>:

{0}</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Loading {0}…</source>
        <translation>Loading {0}…</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Close</source>
        <translation>Close</translation>
    </message>
    <message>
        <location line="+0"/>
        <location line="+333"/>
        <source>Cancel</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location line="-295"/>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</translation>
    </message>
    <message>
        <location line="+30"/>
        <location line="+5"/>
        <source>Filter Edges</source>
        <translation>Filter Edges</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Minimum Threshold Value</source>
        <translation>Minimum Threshold Value</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Initial:</source>
        <translation>Initial:</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Threshold</source>
        <translation>Threshold</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</translation>
    </message>
    <message>
        <location line="+28"/>
        <location line="+6"/>
        <source>Ignore Duplicate Edges</source>
        <translation>Ignore Duplicate Edges</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Increasing the Minimum Correlation Value will usually reduce the graph size.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>PairwiseParameters</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Generic/PairwiseParameters.qml" line="+38"/>
        <source>Pairwise Parameters</source>
        <translation>Pairwise Parameters</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Edge Weight</source>
        <translation>Edge Weight</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Failed to Load {0}</source>
        <translation>Failed to Load {0}</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>:

{0}</source>
        <translation>:

{0}</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Loading {0}…</source>
        <translation>Loading {0}…</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Close</source>
        <translation>Close</translation>
    </message>
    <message>
        <location line="+0"/>
        <location line="+345"/>
        <source>Cancel</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location line="-310"/>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Treat First Row as Header</source>
        <translation>Treat First Row as Header</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+9"/>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Unused&lt;/i&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Source Node&lt;/i&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Target Node&lt;/i&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>► {0}</source>
        <translation>► {0}</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>◇ {0}</source>
        <translation>◇ {0}</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>◆ {0}</source>
        <translation>◆ {0}</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>⨯ Unused</source>
        <translation>⨯ Unused</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>○ Source Node</source>
        <translation>○ Source Node</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>● Target Node</source>
        <translation>● Target Node</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>► Edge Attribute</source>
        <translation>► Edge Attribute</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Source Node Attribute</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Target Node Attribute</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Attribute Name</source>
        <translation>Attribute Name</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
</TS>
