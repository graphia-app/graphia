<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es_ES" sourcelanguage="en_GB">
<context>
    <name>GenericPlugin</name>
    <message>
        <source>Generic</source>
        <translation>Genérico</translation>
    </message>
    <message>
        <source>A plugin that loads generic graphs from a variety of file formats.</source>
        <translation>Un complemento que carga grafos genéricos desde diversos formatos de archivo.</translation>
    </message>
</context>
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>Parámetros de la matriz</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>No se pudo cargar {0}</translation>
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
        <translation>Cargando {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>Una matriz de adyacencia es una matriz cuadrada que se usa para representar un grafo. Los elementos de la matriz indican si los pares de nodos son adyacentes o no en el grafo, es decir, si los nodos están conectados mediante aristas.</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>Filtrar aristas</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>Esta matriz no es binaria, es decir, las celdas distintas de cero indican la intensidad de la relación entre entidades. En algunos casos puede ser conveniente filtrar las aristas por debajo de un umbral absoluto mínimo especificado, lo que se activa aquí.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Mínimo:</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>Valor de umbral mínimo</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Valor de umbral mínimo por encima del cual se creará una arista en el grafo. Un valor mínimo más bajo aumenta los requisitos de cálculo y de memoria.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Inicial:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Umbral</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>El umbral inicial determina el tamaño del grafo resultante. Un valor más bajo filtra menos aristas y da como resultado un grafo más grande. Este valor puede cambiarse más adelante, una vez creado el grafo.</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>Ignorar aristas duplicadas</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>Las matrices de adyacencia suelen presentarse de forma simétrica; es decir, para cada celda de la matriz distinta de cero existe una celda pareja reflejada respecto a la diagonal. Al representar la matriz como grafo, el resultado es que cada par de nodos queda conectado por dos aristas. A menudo esto no es deseable, y esta opción lo evita. Si los dos pesos difieren, se usa el mayor de los valores absolutos.</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamaño estimado del grafo antes de las transformaciones: {0}{3} nodos{2}, {1}{4} aristas{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamaño estimado del grafo: {0}{3} nodos{2}, {1}{4} aristas{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ADVERTENCIA: este grafo es muy grande y podría agotar los recursos del sistema y provocar inestabilidad o bloqueos.</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentar el valor mínimo de correlación suele reducir el tamaño del grafo.</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
<context>
    <name>PairwiseParameters</name>
    <message>
        <source>Pairwise Parameters</source>
        <translation>Parámetros por pares</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>Peso de la arista</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>No se pudo cargar {0}</translation>
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
        <translation>Cargando {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Cerrar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>Se puede crear un grafo a partir de información por pares, es decir, dos columnas de una tabla pueden indicar pares de nodos conectados por una arista. Estas columnas se seleccionan haciendo clic en los encabezados de la tabla de abajo. Además, otras columnas pueden interpretarse como atributos, asociados a la arista o a los nodos que conecta.</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>Tratar la primera fila como encabezado</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Sin usar&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Nodo de origen&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Nodo de destino&lt;/i&gt;</translation>
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
        <translation>⨯ Sin usar</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ Nodo de origen</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● Nodo de destino</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► Atributo de arista</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Atributo del nodo de origen</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Atributo del nodo de destino</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>Nombre del atributo</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>Aceptar</translation>
    </message>
</context>
</TS>
