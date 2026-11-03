<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pl_PL" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Nie udało się wczytać {0}.</translation>
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
        <source>Introduction</source>
        <translation>Wprowadzenie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Korelacyjna analiza grafu&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>Wtyczka korelacji tworzy grafy na podstawie podobieństwa między zmiennymi w zbiorze danych.&lt;br&gt;&lt;br&gt;Miarą tego podobieństwa jest {0}. Jeśli tak określono, dane wejściowe mogą zostać przeskalowane i znormalizowane, po czym na podstawie wyników korelacji ustala się, czy między węzłami reprezentującymi wiersze danych zostanie utworzona krawędź.&lt;br&gt;&lt;br&gt;Po utworzeniu grafu krawędzie można filtrować za pomocą przekształceń.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>współczynnik korelacji</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Wybór danych</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Wybór danych – zaznacz i dostosuj&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Dla zbioru danych automatycznie wybrano ramkę danych. Jeśli chcesz ją dostosować, wybierz poniżej nową komórkę początkową. Ramka danych zawsze sięga do skrajnej prawej dolnej komórki zbioru danych.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Transponuj</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Transpozycja odbija dane względem przekątnej, zamieniając wiersze w kolumny i odwrotnie.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Typ danych:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Ciągłe</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Dyskretne</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Typ danych</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ciągłe:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpretuj zaznaczone dane jako wiersze danych ciągłych. Między wierszami zostanie użyta liczbowa miara korelacji.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Dyskretne:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpretuj zaznaczone dane jako wiersze wartości dyskretnych. Wartości mogą być liczbowe, ale zwykle są tekstowe. Między wierszami zostanie użyta dyskretna miara korelacji.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Traktuj jako binarne</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Gdy ta opcja jest włączona, na potrzeby algorytmu korelacji puste komórki oraz komórki zawierające wartości &lt;i&gt;0&lt;/i&gt; lub &lt;i&gt;false&lt;/i&gt; są uznawane za fałsz, a wszystkie inne wartości za prawdę.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;OSTRZEŻENIE: wybrana ramka danych zawiera najprawdopodobniej dane ciągłe; interpretowanie ich jako dyskretnych może dać jedynie niski poziom korelacji.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korelacja</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Korelacja – dostosuj parametry&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Zaawansowane</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Na osi poziomej poniższego wykresu wartość 1 oznacza idealnie skorelowane wiersze, a 0 brak korelacji. Wszystkie wartości bezwzględne poniżej wybranego minimalnego progu korelacji są odrzucane i nie będą użyte do tworzenia krawędzi. Domyślnie dodawane jest przekształcenie, które usuwa krawędzie dla wszystkich wartości bezwzględnych poniżej określonego początkowego progu korelacji.&lt;br&gt;&lt;br&gt;Aby mieć większą kontrolę nad parametrami, zaznacz opcję &lt;b&gt;Zaawansowane&lt;/b&gt; w prawym górnym rogu.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Algorytm:</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>SMC</source>
        <translation>SMC</translation>
    </message>
    <message>
        <source>Correlation Algorithm</source>
        <translation>Algorytm korelacji</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rangowa Spearmana</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Podobieństwo euklidesowe</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Podobieństwo kosinusowe</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Znak:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Dodatnia</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Ujemna</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Obie</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Znak</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Domyślnie przy tworzeniu grafu uwzględniane są tylko korelacje dodatnie. Niemal zawsze jest to właściwe ustawienie, ale w przypadku niektórych źródeł danych sensowniej może być uwzględnić znak korelacji. Nie wszystkie algorytmy korelacji potrafią znajdować zależności ujemne.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputacja:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Stała</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolacja w wierszu</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Średnia kolumny</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputacja</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Stała:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Zastąp wszystkie brakujące wartości stałą.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Interpolacja w wierszu:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Interpoluj liniowo brakujące wartości na podstawie najbliższych sąsiednich wartości w wierszu i ich względnych pozycji. Jeśli dostępna jest tylko jedna sąsiednia wartość, zostanie ona przyjęta. Jeśli wiersz jest pusty, wartość zostanie ustawiona na zero.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Średnia kolumny:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Zastąp brakujące wartości średnią wartością ich kolumny, z pominięciem brakujących wartości.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Przycinanie:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsoryzacja</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>percentyl</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Przycinanie</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Ogranicz maksymalną wartość tabeli danych do określonej stałej.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsoryzacja:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Ogranicz maksymalną wartość danych do wartości określonego percentyla, osobno dla każdego wiersza. Na przykład po wybraniu 95. percentyla górne 5% wartości otrzyma wartość 95. percentyla.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Skalowanie:</translation>
    </message>
    <message>
        <source>Log2(x + ε)</source>
        <translation>Log2(x + ε)</translation>
    </message>
    <message>
        <source>Log10(x + ε)</source>
        <translation>Log10(x + ε)</translation>
    </message>
    <message>
        <source>AntiLog2(x)</source>
        <translation>AntiLog2(x)</translation>
    </message>
    <message>
        <source>AntiLog10(x)</source>
        <translation>AntiLog10(x)</translation>
    </message>
    <message>
        <source>Arcsin(x)</source>
        <translation>Arcsin(x)</translation>
    </message>
    <message>
        <source>Scaling Types</source>
        <translation>Rodzaje skalowania</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Oblicz logarytm z &lt;i&gt;x&lt;/i&gt; + ε o podstawie &lt;i&gt;b&lt;/i&gt;, gdzie &lt;i&gt;x&lt;/i&gt; to dane wejściowe, a ε to bardzo mała stała.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Podnieś &lt;i&gt;b&lt;/i&gt; do potęgi &lt;i&gt;x&lt;/i&gt;, gdzie &lt;i&gt;x&lt;/i&gt; to dane wejściowe.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Oblicz arcus sinus z &lt;i&gt;x&lt;/i&gt;, gdzie &lt;i&gt;x&lt;/i&gt; to dane wejściowe. Przydaje się, gdy potrzebne jest przekształcenie logarytmiczne, ale zbiór danych zawiera liczby ujemne lub zera.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalizacja:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Min./maks.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Średnia</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Standaryzacja</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Skalowanie do jednostki</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Kwantylowa</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Rodzaje normalizacji</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min./maks.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Przeskaluj dane tak, aby ich wartości mieściły się w przedziale [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Przydaje się, gdy kolumny zbioru danych mają różne skale lub jednostki. Uwaga: jeśli wszystkie elementy kolumny mają tę samą wartość, zostaną przeskalowane do 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Średnia:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Podobne do skalowania min./maks., z tą różnicą, że wynikowe wartości są wyśrodkowane wokół średniej ich kolumny: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Uwaga: jeśli wszystkie elementy kolumny mają tę samą wartość, zostaną przeskalowane do 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standaryzacja:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Ta metoda, znana też jako normalizacja wyniku Z, wyśrodkowuje wartość wokół średniej i skaluje ją przez odchylenie standardowe: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Skalowanie do jednostki:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Wartości są dzielone przez długość euklidesową wektora utworzonego przez kolumnę danych: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Kwantylowa:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Znormalizuj dane tak, aby kolumny miały jednakowe rozkłady.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Funkcja softmax przekształca każdą kolumnę danych w rozkład prawdopodobieństwa przez podniesienie e do potęgi każdego elementu i normalizację przez podzielenie przez sumę. W efekcie największe wartości w każdej kolumnie zostają uwydatnione.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filtrowanie:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Próg</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filtrowanie</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Próg:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Prosty filtr odrzucający korelacje poniżej wybranego progu.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k najbliższych sąsiadów:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Śledź korelacje między parami wierszy, odrzucając te, które nie należą do &lt;i&gt;k&lt;/i&gt; najsilniejszych korelacji każdego wiersza. Najbardziej przydatne w przypadku zbiorów danych o wyjątkowo silnej korelacji.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Minimalna wartość korelacji</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Minimalna wartość korelacji, powyżej której w grafie zostanie utworzona krawędź. Niższa wartość minimalna zwiększa zapotrzebowanie na obliczenia i pamięć.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>Maksymalne k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Maksymalne &lt;i&gt;k&lt;/i&gt; korelacji</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Maksymalna liczba korelacji zachowywanych dla każdego wiersza danych, co ostatecznie ogranicza liczbę krawędzi tworzonych w grafie. Większa wartość maksymalna zwiększa zapotrzebowanie na obliczenia i pamięć.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Początkowy:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>Początkowe k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Próg korelacji</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; korelacji</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Początkowy próg korelacji określa rozmiar wynikowego grafu. Niższa wartość odfiltrowuje mniej krawędzi i daje większy graf. Wartość tę można zmienić później, po utworzeniu grafu.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Początkowa wartość k określa rozmiar wynikowego grafu. Wyższa wartość odfiltrowuje mniej krawędzi i daje większy graf. Wartość tę można zmienić później, po utworzeniu grafu.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Przetwarzanie…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Pusty graf</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Szablon</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Szablon&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Szablon to nazwany zestaw przekształceń i wizualizacji, który można zastosować do grafu po jego utworzeniu. Wybierz szablon do zastosowania. &lt;b&gt;Uwaga:&lt;/b&gt; przekształcenia zastosowane tutaj zawsze zostaną dołączone bezwarunkowo, niezależnie od sposobu stosowania określonego w szablonie.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nazwa:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Ustaw jako domyślne</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Podsumowanie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Podsumowanie&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Graf zostanie utworzony z następującymi parametrami:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Transponowane&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Ramka danych: [ Kolumna: {0} Wiersz: {1} Szerokość: {2} Wysokość: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Minimalna wartość korelacji: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Początkowy próg korelacji: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Maksymalna wartość k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Początkowa wartość k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Miara korelacji danych ciągłych: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Znak korelacji: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputacja: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputacja: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Przycinanie: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Przycinanie: {0} ({1}. percentyl)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Przycinanie: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Skalowanie: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalizacja: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Miara korelacji danych dyskretnych: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Szablon: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Szacowany rozmiar grafu przed przekształceniami: {0}{3} węzłów{2}, {1}{4} krawędzi{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Zwiększenie minimalnej wartości korelacji zwykle zmniejsza rozmiar grafu.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Zmniejszenie maksymalnego &lt;i&gt;k&lt;/i&gt; zwykle zmniejsza rozmiar grafu.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}OSTRZEŻENIE: to bardzo duży graf, który może wyczerpać zasoby systemowe i spowodować niestabilność lub zawieszenie. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}OSTRZEŻENIE: wygenerowany graf najprawdopodobniej będzie pusty.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 kolumna</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n kolumna</numerusform>
            <numerusform>%n kolumny</numerusform>
            <numerusform>%n kolumn</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…i %1 więcej</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Powiększ, aby zobaczyć informacje o kolumnach</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Powiększ, aby zobaczyć nazwy kolumn</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n unikatowa wartość</numerusform>
            <numerusform>%n unikatowe wartości</numerusform>
            <numerusform>%n unikatowych wartości</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Średnia z %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Średnia z zaznaczenia</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Mediana z %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Mediana z zaznaczenia</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Średni histogram z %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Średni histogram z zaznaczenia</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Odchylenie</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 górne</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 dolne</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Odch. std.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Bł. std.</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 i %3 więcej…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>Rozszerzenie %1 wskazuje, że jest to „%2”, jednak nie udało się ustalić jego zawartości.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>Rozszerzenie %1 wskazuje, że jest to „%2”, jednak jego zawartość przypomina plik „%3”.</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korelacja</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Tworzy graf, w którym węzły reprezentują wiersze danych, a krawędzie korelacje między tymi wierszami.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>Wartość danych to atrybut z parametrem, który pozwala odwołać się do określonej kolumny skorelowanych danych.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Nazwa kolumny danych o największej wartości bezwzględnej w wierszu danych powiązanym z węzłem.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>Średnia wartość danych to średnia wartości powiązanych z węzłem.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>Minimalna wartość danych to najmniejsza wartość powiązana z węzłem.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>Maksymalna wartość danych to największa wartość powiązana z węzłem.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 to miara rozrzutu wartości powiązanych z węzłem. Definiuje się ją jako ∑(&lt;i&gt;x&lt;/i&gt;-µ)², gdzie &lt;i&gt;x&lt;/i&gt; to wartość, a µ to średnia.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Wariancja</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 to miara rozrzutu wartości powiązanych z węzłem. Definiuje się je jako √∑(&lt;i&gt;x&lt;/i&gt;-µ)², gdzie &lt;i&gt;x&lt;/i&gt; to wartość, a µ to średnia.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Odchylenie standardowe</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>%1 to miara rozrzutu wartości powiązanych z węzłem. Definiuje się go jako odchylenie standardowe podzielone przez średnią.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Współczynnik zmienności</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>W danych wtyczki brakuje klucza „%1”.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Ramka danych: [ Kolumna: %1 Wiersz: %2 Szerokość: %3 Wysokość: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Transponowane</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Miara korelacji danych ciągłych: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Dodatnia</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Ujemna</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Obie</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Znak korelacji: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Miara korelacji danych dyskretnych: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Minimalna wartość korelacji: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Maksymalna wartość k: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Stała (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Średnia kolumny</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolacja w wierszu</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputacja: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Przycinanie: stała (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Przycinanie: winsoryzacja (%1. percentyl)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Skalowanie: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Skalowanie: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Skalowanie: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Skalowanie: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Skalowanie: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalizacja: min./maks.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalizacja: kwantylowa</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalizacja: średnia</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalizacja: standaryzacja</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalizacja: skalowanie do jednostki</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalizacja: skalowanie softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importuj adnotacje kolumn</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Brak odpowiednich kluczy</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Nie udało się wczytać {0}.</translation>
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
        <source>Introduction</source>
        <translation>Wprowadzenie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Importuj adnotacje&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Dane adnotacji można zaimportować z zewnętrznego źródła i nałożyć na wykres. Można użyć dowolnego z obsługiwanych formatów danych tabelarycznych. Po zaimportowaniu nowe adnotacje można sortować i grupować w zwykły sposób.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Wybór kluczy</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Wybór kluczy&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Aby zaimportować dane adnotacji, trzeba wskazać wiersz danych wejściowych odpowiadający nazwom kolumn istniejących danych. Służy on do dopasowania kolumn tabeli do kolumn wykresu. Wybierz poniżej właściwy wiersz. Możesz też kliknąć &lt;i&gt;{0}&lt;/i&gt;, a najlepsze dopasowanie zostanie wskazane automatycznie.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Wykryj automatycznie</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>Wybrany wiersz &lt;b&gt;{0}&lt;/b&gt; zawiera powtarzające się wartości, niektóre kolumny zostaną pominięte!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Znaleziono dopasowanie {0}%</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Nie znaleziono dopasowania</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Wybór adnotacji</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Wybór adnotacji&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Wybierz potrzebne adnotacje z listy po prawej. Aby wybrać kilka adnotacji, przytrzymaj &lt;i&gt;Ctrl&lt;/i&gt;; aby wybrać zakres, przytrzymaj &lt;i&gt;Shift&lt;/i&gt; i klikaj odpowiednie pozycje.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Zastąp istniejące adnotacje</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Zaznacz wszystko</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Odznacz wszystko</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Podsumowanie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Podsumowanie&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Wykrywanie klucza…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Następujące adnotacje zostaną zaimportowane z użyciem wiersza &lt;b&gt;{0}{1}&lt;/b&gt; jako klucza:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(zawiera powtarzające się wartości)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Adnotacji nie można zaimportować, dopóki nie zostanie wybrany klucz.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Wykrywanie klucza</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Interfejs &amp;obok wykresu</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Pokaż nazwy &amp;kolumn</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Znajdź interesujące wiersze</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Wybierz widoczne adnotacje kolumn</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Linie siatki</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Uwzględnij zero na osi Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Pokaż wartości odstające</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Resetuj powiększenie</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Pokaż wszystkie kolumny</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>Log(x + ε)</source>
        <translation>Log(x + ε)</translation>
    </message>
    <message>
        <source>AntiLog</source>
        <translation>AntiLog</translation>
    </message>
    <message>
        <source>Mean Centre Scaling</source>
        <translation>Centrowanie względem średniej</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Skalowanie do wariancji jednostkowej</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Skalowanie Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Słupki błędów</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Obszar</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Odchylenie standardowe</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Błąd standardowy</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Pojedyncze linie</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Linia średniej</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Linia mediany</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Średni histogram</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Rozstęp międzykwartylowy</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Grupuj według adnotacji</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Importuj adnotacje z tabeli…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-wykres-korelacji</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Zapisz jako &amp;obraz…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Ustaw etykietę osi X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Ustaw etykietę osi Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Etykieta osi X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Wprowadź etykietę osi X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Etykieta osi Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Wprowadź etykietę osi Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Wykres</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Etykiety osi</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Skalowanie</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Według atrybutu</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Uśrednianie</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Wszystkie</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Rozrzut</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Sortuj kolumny według</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Koloruj grupy według</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Zapisz jako obrazy według…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Wiersz</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Kolejność naturalna</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Nazwa kolumny</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Wartość danych</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Klasteryzacja hierarchiczna</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Kliknij kolumnę lub adnotację kolumny, aby ją zaznaczyć. Przytrzymaj &lt;i&gt;Ctrl&lt;/i&gt; i klikaj, aby zaznaczyć wiele kolumn.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Ustal kolejność sortowania według wartości danych</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Zapisz wykres jako obraz</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Obraz PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Obraz JPEG (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>Dokument PDF (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Pokaż wszystkie adnotacje</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Ukryj wszystkie adnotacje</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Względne</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Bezwzględne</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Ważenie zaznaczenia</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Wiersze są wybierane na podstawie ważenia sumy zaznaczonych kolumn względem kolumn niezaznaczonych. Ważenie zaznaczonych kolumn (oznaczone jako &lt;i&gt;Bezwzględne&lt;/i&gt;) zwykle znajduje wiersze mające szczyty w tych kolumnach, niezależnie od kolumn niezaznaczonych. Natomiast (ujemne) ważenie kolumn niezaznaczonych (oznaczone jako &lt;i&gt;Względne&lt;/i&gt;) zwykle znajduje wiersze, które mają istotne wartości właśnie w zaznaczonych kolumnach.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}.</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}.</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}.</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}.</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Percentyl</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Liczba wierszy do wybrania i uszeregowania.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Zaznacz ponownie</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Rozstęp międzykwartylowy %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Rozstęp międzykwartylowy</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Zapisywanie obrazów wykresu</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Współczynnik korelacji Pearsona wskazuje stopień liniowej korelacji między dwiema zmiennymi.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Wartość korelacji Pearsona</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>%1 wskazuje stopień liniowej zależności między dwiema zmiennymi.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Współczynnik korelacji Pearsona</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rangowa Spearmana</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Współczynnik korelacji rang Spearmana to nieparametryczna miara zależności statystycznej między rangami dwóch zmiennych. Ocenia, jak dobrze zależność między dwiema zmiennymi można opisać funkcją monotoniczną.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Wartość korelacji rang Spearmana</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>%1 wskazuje stopień monotonicznej zależności między dwiema zmiennymi.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Współczynnik korelacji rang Spearmana</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Podobieństwo euklidesowe</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Podobieństwo euklidesowe to w istocie odwrotność odległości euklidesowej między dwoma wektorami.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1 to w istocie odwrotność odległości euklidesowej między dwoma wektorami.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Podobieństwo kosinusowe</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Podobieństwo kosinusowe to miara podobieństwa między dwoma niezerowymi wektorami przestrzeni unitarnej. Jest zdefiniowane jako kosinus kąta między nimi, co jest równe iloczynowi skalarnemu tych samych wektorów znormalizowanych do długości 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1 to miara podobieństwa między dwoma niezerowymi wektorami przestrzeni unitarnej.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Korelacja biweight midcorrelation (zwana też Bicor) to miara podobieństwa między próbkami. Opiera się na medianie, a nie na średniej, dzięki czemu jest mniej wrażliwa na wartości odstające.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Wartość korelacji Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 to oparta na medianie miara podobieństwa między próbkami.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Indeks Jaccarda to statystyka służąca do oceny podobieństwa i różnorodności zbiorów próbek.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Wartość korelacji Jaccarda</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>%1 to statystyka służąca do oceny podobieństwa i różnorodności zbiorów próbek.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Indeks Jaccarda</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Prosty współczynnik dopasowania</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Prosty współczynnik dopasowania to statystyka służąca do oceny podobieństwa i różnorodności zbiorów próbek. Jest identyczny z indeksem Jaccarda, z tą różnicą, że uwzględnia też wspólny brak.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Prosty współczynnik dopasowania (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Wartość danych</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Najsilniejsza kolumna danych</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Średnia wartość danych</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Minimalna wartość danych</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Maksymalna wartość danych</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Wariancja</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Odchylenie standardowe</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Współczynnik zmienności</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 bezwzględna</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Dane</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Wartości korelacji</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Plik CSV korelacji</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Pliki CSV korelacji</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Plik TSV korelacji</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Pliki TSV korelacji</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Plik SSV korelacji</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Pliki SSV korelacji</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Plik Excel korelacji</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Pliki Excel korelacji</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Obliczanie korelacji</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Klasteryzacja</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importuj adnotacje kolumn</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Importuj adnotację kolumny</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Importowanie adnotacji kolumn</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Importowanie adnotacji kolumny</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>Zaimportowano %n adnotację</numerusform>
            <numerusform>Zaimportowano %n adnotacje</numerusform>
            <numerusform>Zaimportowano %n adnotacji</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Zaimportowano adnotację %1</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atrybuty</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalizacja</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korelacja</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Budowanie grafu</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Sortowanie kolumn</translation>
    </message>
</context>
</TS>
