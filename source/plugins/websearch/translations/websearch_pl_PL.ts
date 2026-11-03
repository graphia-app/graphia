<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pl_PL" sourcelanguage="en_GB">
<context>
    <name>MatrixParameters</name>
    <message>
        <source>Matrix Parameters</source>
        <translation>Parametry macierzy</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Nie udało się wczytać {0}</translation>
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
        <translation>Wczytywanie {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>An adjacency matrix is a square matrix used to represent a graph. The elements of the matrix indicate whether pairs of nodes are adjacent or not in the graph, i.e. the nodes are connected via edges.</source>
        <translation>Macierz sąsiedztwa to macierz kwadratowa reprezentująca graf. Jej elementy wskazują, czy pary węzłów w grafie sąsiadują ze sobą, tj. czy węzły są połączone krawędziami.</translation>
    </message>
    <message>
        <source>Filter Edges</source>
        <translation>Filtruj krawędzie</translation>
    </message>
    <message>
        <source>This matrix is non-binary, i.e. non-zero cells indicate the strength of the relationship between entities. In some circumstances it may be advantageous to filter edges below a specified minimum absolute threshold, which is enabled here.</source>
        <translation>Ta macierz nie jest binarna, tj. niezerowe komórki wskazują siłę relacji między obiektami. W niektórych sytuacjach korzystne może być odfiltrowanie krawędzi poniżej określonego minimalnego progu bezwzględnego, co można tutaj włączyć.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <source>Minimum Threshold Value</source>
        <translation>Minimalna wartość progowa</translation>
    </message>
    <message>
        <source>The minimum threshold value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Minimalna wartość progowa, powyżej której w grafie zostanie utworzona krawędź. Niższa wartość minimalna zwiększa zapotrzebowanie na obliczenia i pamięć.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Początkowy:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Próg</translation>
    </message>
    <message>
        <source>The initial threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Początkowy próg określa rozmiar wynikowego grafu. Niższa wartość odfiltrowuje mniej krawędzi i daje większy graf. Wartość tę można zmienić później, po utworzeniu grafu.</translation>
    </message>
    <message>
        <source>Ignore Duplicate Edges</source>
        <translation>Ignoruj zduplikowane krawędzie</translation>
    </message>
    <message>
        <source>Adjacency matrices are often presented in a symmetric fashion; that is to say that for every non-zero cell in the matrix with a value, there exists a partner cell reflected across the matrix&apos;s diagonal. In terms of representing the matrix as a graph, the net result is that every pair of nodes are connected by two edges. Often this is undesirable. Selecting this option avoids this. In the case where the pair of weights differ, the larger of the absolute values is used.</source>
        <translation>Macierze sąsiedztwa są często symetryczne, tzn. dla każdej niezerowej komórki macierzy istnieje komórka partnerska odbita względem przekątnej. W reprezentacji macierzy jako grafu skutkuje to tym, że każda para węzłów jest połączona dwiema krawędziami. Często jest to niepożądane i ta opcja temu zapobiega. Jeśli wagi w parze się różnią, używana jest większa z wartości bezwzględnych.</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Szacowany rozmiar grafu przed przekształceniami: {0}{3} węzłów{2}, {1}{4} krawędzi{2}</translation>
    </message>
    <message>
        <source>Estimated Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Szacowany rozmiar grafu: {0}{3} węzłów{2}, {1}{4} krawędzi{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes.</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}OSTRZEŻENIE: to bardzo duży graf, który może wyczerpać zasoby systemowe i spowodować niestabilność lub zawieszenie.</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Zwiększenie minimalnej wartości korelacji zwykle zmniejsza rozmiar grafu.</translation>
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
        <translation>Parametry par</translation>
    </message>
    <message>
        <source>Edge Weight</source>
        <translation>Waga krawędzi</translation>
    </message>
    <message>
        <source>Failed to Load {0}</source>
        <translation>Nie udało się wczytać {0}</translation>
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
        <translation>Wczytywanie {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>A graph can be created from pairwise information, that is to say two columns in a table may be used to indicate pairs of nodes which are connected by an edge. These columns are selected by clicking on the table headers below. Furthermore, other columns can be interpreted as attributes, associated either with the edge or its connecting nodes.</source>
        <translation>Graf można utworzyć z informacji o parach, tzn. dwie kolumny tabeli mogą wskazywać pary węzłów połączonych krawędzią. Kolumny te wybiera się, klikając nagłówki tabeli poniżej. Ponadto inne kolumny mogą być interpretowane jako atrybuty, powiązane z krawędzią lub z łączonymi przez nią węzłami.</translation>
    </message>
    <message>
        <source>Treat First Row as Header</source>
        <translation>Traktuj pierwszy wiersz jako nagłówek</translation>
    </message>
    <message>
        <source>⨯ &lt;i&gt;Unused&lt;/i&gt;</source>
        <translation>⨯ &lt;i&gt;Nieużywana&lt;/i&gt;</translation>
    </message>
    <message>
        <source>◯ &lt;i&gt;Source Node&lt;/i&gt;</source>
        <translation>◯ &lt;i&gt;Węzeł źródłowy&lt;/i&gt;</translation>
    </message>
    <message>
        <source>⬤ &lt;i&gt;Target Node&lt;/i&gt;</source>
        <translation>⬤ &lt;i&gt;Węzeł docelowy&lt;/i&gt;</translation>
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
        <translation>⨯ Nieużywana</translation>
    </message>
    <message>
        <source>○ Source Node</source>
        <translation>○ Węzeł źródłowy</translation>
    </message>
    <message>
        <source>● Target Node</source>
        <translation>● Węzeł docelowy</translation>
    </message>
    <message>
        <source>► Edge Attribute</source>
        <translation>► Atrybut krawędzi</translation>
    </message>
    <message>
        <source>◇ Source Node Attribute</source>
        <translation>◇ Atrybut węzła źródłowego</translation>
    </message>
    <message>
        <source>◆ Target Node Attribute</source>
        <translation>◆ Atrybut węzła docelowego</translation>
    </message>
    <message>
        <source>Attribute Name</source>
        <translation>Nazwa atrybutu</translation>
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
        <translation>Wyszukiwanie w sieci</translation>
    </message>
    <message>
        <source>An embedded web browser that searches for the node selection using a URL template.</source>
        <translation>Wbudowana przeglądarka internetowa, która wyszukuje zaznaczone węzły na podstawie szablonu adresu URL.</translation>
    </message>
</context>
</TS>
