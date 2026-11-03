<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt_BR" sourcelanguage="en_GB">
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>Parâmetros da matriz</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Falha ao carregar {0}</translation>
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
        <translation>Carregando {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>Uma matriz de adjacência é uma matriz quadrada utilizada para representar um grafo. Os elementos da matriz indicam se pares de nós são ou não adjacentes no grafo, ou seja, se os nós estão conectados por arestas.</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>Filtrar arestas</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>Esta matriz não é binária, ou seja, as células não nulas indicam a força da relação entre entidades. Em algumas circunstâncias pode ser vantajoso filtrar as arestas abaixo de um limiar absoluto mínimo especificado, o que se ativa aqui.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Mínimo:</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>Valor de limiar mínimo</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>O valor de limiar mínimo acima do qual será criada uma aresta no grafo. Um valor mínimo mais baixo aumenta os requisitos de computação e memória.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Inicial:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Limiar</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>O limiar inicial determina o tamanho do grafo resultante. Um valor mais baixo filtra menos arestas e resulta em um grafo maior. Este valor pode ser alterado mais tarde, depois de o grafo ser criado.</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>Ignorar arestas duplicadas</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>As matrizes de adjacência são frequentemente simétricas; ou seja, para cada célula não nula da matriz existe uma célula parceira refletida em relação à diagonal. Ao representar a matriz como grafo, o resultado é que cada par de nós fica conectado por duas arestas. Muitas vezes isto não é desejável, e esta opção evita-o. Se os dois pesos forem diferentes, é utilizado o maior dos valores absolutos.</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamanho estimado do grafo antes das transformações: {0}{3} nós{2}, {1}{4} arestas{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamanho estimado do grafo: {0}{3} nós{2}, {1}{4} arestas{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}AVISO: este é um grafo muito grande, que pode esgotar os recursos do sistema e causar instabilidade ou bloqueios.</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentar o valor mínimo de correlação normalmente reduz o tamanho do grafo.</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>PairwiseParameters</name>
    <message>
        <source>Pairwise Parameters</source>
        <translation>Parâmetros de pares</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>Peso da aresta</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Falha ao carregar {0}</translation>
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
        <translation>Carregando {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fechar</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Cancelar</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>Um grafo pode ser criado a partir de informações de pares, ou seja, duas colunas de uma tabela podem indicar pares de nós conectados por uma aresta. Estas colunas são selecionadas clicando nos cabeçalhos da tabela abaixo. Além disso, outras colunas podem ser interpretadas como atributos, associados à aresta ou aos nós que esta conecta.</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>Tratar a primeira linha como cabeçalho</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Não utilizada&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Nó de origem&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Nó de destino&lt;/i&gt;</translation>
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
        <translation>⨯ Não utilizada</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ Nó de origem</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● Nó de destino</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► Atributo da aresta</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Atributo do nó de origem</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Atributo do nó de destino</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>Nome do atributo</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>WebSearchPlugin</name>
    <message>
        <source>Web Search</source>
        <translation>Pesquisa na Web</translation>
    </message>
    <message>
        <source>An embedded web browser that searches for the node selection using a URL template.</source>
        <translation>Um navegador Web incorporado que pesquisa a seleção de nós utilizando um modelo de URL.</translation>
    </message>
</context>
</TS>
