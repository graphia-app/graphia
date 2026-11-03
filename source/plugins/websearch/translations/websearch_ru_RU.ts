<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_GB">
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>Параметры матрицы</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Не удалось загрузить {0}</translation>
    </message>
    <message>
        <source>:

{0}</source>
        <translation>:

{0}</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Загрузка {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>Матрица смежности — квадратная матрица, представляющая граф. Её элементы показывают, смежны ли пары узлов в графе, то есть соединены ли узлы рёбрами.</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>Фильтровать рёбра</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>Эта матрица не двоичная, то есть ненулевые ячейки обозначают силу связи между сущностями. Иногда бывает полезно отфильтровать рёбра ниже заданного минимального абсолютного порога; это можно включить здесь.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Минимум:</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>Минимальное пороговое значение</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Минимальное пороговое значение, выше которого в графе создаётся ребро. Чем ниже минимальное значение, тем выше требования к вычислениям и памяти.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Начальный:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Порог</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Начальный порог определяет размер получаемого графа. Чем ниже значение, тем меньше рёбер отфильтровывается и тем больше граф. Это значение можно изменить позже, после создания графа.</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>Пропускать повторяющиеся рёбра</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>Матрицы смежности часто симметричны: для каждой ненулевой ячейки матрицы существует парная ячейка, отражённая относительно диагонали. При представлении матрицы в виде графа в итоге каждая пара узлов соединяется двумя рёбрами. Часто это нежелательно, и этот параметр позволяет этого избежать. Если веса в паре различаются, используется большее по модулю значение.</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Оценка размера графа до преобразований: {0}{3} узлов{2}, {1}{4} рёбер{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Оценка размера графа: {0}{3} узлов{2}, {1}{4} рёбер{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ВНИМАНИЕ: это очень большой граф, который может исчерпать системные ресурсы и привести к нестабильной работе или зависаниям.</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Увеличение минимального значения корреляции обычно уменьшает размер графа.</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
</context>
<context>
    <name>PairwiseParameters</name>
    <message>
        <source>Pairwise Parameters</source>
        <translation>Параметры пар</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>Вес ребра</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Не удалось загрузить {0}</translation>
    </message>
    <message>
        <source>:

{0}</source>
        <translation>:

{0}</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Загрузка {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>Граф можно построить из попарной информации: два столбца таблицы указывают пары узлов, соединённых ребром. Эти столбцы выбираются щелчком по заголовкам таблицы ниже. Кроме того, другие столбцы можно интерпретировать как атрибуты, связанные либо с ребром, либо с соединяемыми им узлами.</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>Считать первую строку заголовком</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Не используется&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Исходный узел&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Целевой узел&lt;/i&gt;</translation>
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
        <translation>⨯ Не используется</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ Исходный узел</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● Целевой узел</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► Атрибут ребра</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Атрибут исходного узла</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Атрибут целевого узла</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>Название атрибута</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>ОК</translation>
    </message>
</context>
<context>
    <name>WebSearchPlugin</name>
    <message>
        <source>Web Search</source>
        <translation>Веб-поиск</translation>
    </message>
    <message>
        <source>An embedded web browser that searches for the node selection using a URL template.</source>
        <translation>Встроенный веб-браузер, который ищет выделенные узлы по шаблону URL.</translation>
    </message>
</context>
</TS>
