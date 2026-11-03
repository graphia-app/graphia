<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it_IT" sourcelanguage="en_GB">
<context>
    <name>GenericPlugin</name>
    <message>
        <source>Generic</source>
        <translation>Generico</translation>
    </message>
    <message>
        <source>A plugin that loads generic graphs from a variety of file formats.</source>
        <translation>Un plugin che carica grafi generici da diversi formati di file.</translation>
    </message>
</context>
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>Parametri della matrice</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Impossibile caricare {0}</translation>
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
        <translation>Caricamento di {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>Una matrice di adiacenza è una matrice quadrata usata per rappresentare un grafo. Gli elementi della matrice indicano se le coppie di nodi sono adiacenti o meno nel grafo, cioè se i nodi sono collegati da archi.</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>Filtra gli archi</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>Questa matrice non è binaria, cioè le celle diverse da zero indicano la forza della relazione tra le entità. In alcuni casi può essere vantaggioso filtrare gli archi sotto una soglia assoluta minima specificata, cosa che si attiva qui.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimo:</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>Valore di soglia minimo</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Il valore di soglia minimo oltre il quale verrà creato un arco nel grafo. Un valore minimo più basso aumenta i requisiti di calcolo e di memoria.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Iniziale:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Soglia</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>La soglia iniziale determina la dimensione del grafo risultante. Un valore più basso filtra meno archi e produce un grafo più grande. Questo valore può essere modificato in seguito, dopo la creazione del grafo.</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>Ignora archi duplicati</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>Le matrici di adiacenza sono spesso simmetriche: per ogni cella diversa da zero della matrice esiste una cella corrispondente riflessa rispetto alla diagonale. Rappresentando la matrice come grafo, il risultato è che ogni coppia di nodi è collegata da due archi. Spesso ciò non è desiderabile, e questa opzione lo evita. Se i due pesi della coppia differiscono, viene usato il maggiore dei valori assoluti.</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Dimensione stimata del grafo prima delle trasformazioni: {0}{3} nodi{2}, {1}{4} archi{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Dimensione stimata del grafo: {0}{3} nodi{2}, {1}{4} archi{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ATTENZIONE: questo è un grafo molto grande, che potrebbe esaurire le risorse di sistema e causare instabilità o blocchi.</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentare il valore di correlazione minimo di solito riduce la dimensione del grafo.</translation>
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
        <translation>Parametri a coppie</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>Peso dell’arco</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Impossibile caricare {0}</translation>
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
        <translation>Caricamento di {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>Un grafo può essere creato da informazioni a coppie: due colonne di una tabella possono indicare coppie di nodi collegati da un arco. Queste colonne si selezionano facendo clic sulle intestazioni della tabella qui sotto. Inoltre, altre colonne possono essere interpretate come attributi, associati all’arco o ai nodi che esso collega.</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>Tratta la prima riga come intestazione</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Non usata&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Nodo di origine&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Nodo di destinazione&lt;/i&gt;</translation>
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
        <translation>⨯ Non usata</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ Nodo di origine</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● Nodo di destinazione</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► Attributo dell’arco</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Attributo del nodo di origine</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Attributo del nodo di destinazione</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>Nome dell’attributo</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
</TS>
