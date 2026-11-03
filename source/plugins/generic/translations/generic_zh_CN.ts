<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en_GB">
<context>
    <name>GenericPlugin</name>
    <message>
        <source>Generic</source>
        <translation>通用</translation>
    </message>
    <message>
        <source>A plugin that loads generic graphs from a variety of file formats.</source>
        <translation>从多种文件格式载入通用图的插件。</translation>
    </message>
</context>
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>矩阵参数</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>无法载入 {0}</translation>
    </message>
    <message>
        <source>:

{0}</source>
        <translation>：

{0}</translation>
    </message>
    <message>
        <source>.</source>
        <translation>。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>正在加载 {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>邻接矩阵是用于表示图的方阵。矩阵的元素表示图中各对节点是否相邻，即节点之间是否有边相连。</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>过滤边</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>此矩阵不是二值的，即非零单元格表示实体之间关系的强度。在某些情况下，过滤掉低于指定最小绝对阈值的边可能更有利，可在此处启用。</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>最小值：</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>最小阈值</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>高于此最小阈值时将在图中创建边。最小值越低，计算量和内存需求越大。</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>初始值：</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>阈值</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>初始阈值决定所生成图的大小。值越低，过滤掉的边越少，得到的图越大。创建图之后仍可更改此值。</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>忽略重复边</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>邻接矩阵通常是对称的；也就是说，矩阵中每个有值的非零单元格，都有一个沿矩阵对角线对称的对应单元格。将矩阵表示为图时，最终结果是每对节点都由两条边相连。这通常并不理想，选择此选项可以避免。如果两个权重不同，则使用绝对值较大的那个。</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>预计变换前的图大小：{0}{3} 个节点{2}，{1}{4} 条边{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>预计图大小：{0}{3} 个节点{2}，{1}{4} 条边{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}警告：这是一个非常大的图，可能耗尽系统资源，导致不稳定或卡死。</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>提高最小相关值通常会减小图的大小。</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
</context>
<context>
    <name>PairwiseParameters</name>
    <message>
        <source>Pairwise Parameters</source>
        <translation>成对参数</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>边权重</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>无法载入 {0}</translation>
    </message>
    <message>
        <source>:

{0}</source>
        <translation>：

{0}</translation>
    </message>
    <message>
        <source>.</source>
        <translation>。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>正在加载 {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>可以根据成对信息创建图，也就是说，可以用表格中的两列来表示由边相连的节点对。单击下方表格的列标题即可选择这两列。此外，其他列可以解释为属性，与边或其连接的节点关联。</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>将第一行视为标题</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;未使用&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;源节点&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;目标节点&lt;/i&gt;</translation>
    </message>
    <message>
        <source>► {0}</source>
        <translation>► {0}</translation>
    </message>
    <message>
        <source>◇ {0}</source>
        <translation>◇ {0}</translation>
    </message>
    <message>
        <source>◆ {0}</source>
        <translation>◆ {0}</translation>
    </message>
    <message>
        <source>⨯ Unused</source>
        <translation>⨯ 未使用</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ 源节点</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● 目标节点</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► 边属性</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ 源节点属性</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ 目标节点属性</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>属性名称</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>确定</translation>
    </message>
</context>
</TS>
