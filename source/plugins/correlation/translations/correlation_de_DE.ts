<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>{0} konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} wird geladen…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Einführung</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Korrelationsgraphanalyse&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>Das Korrelations-Plugin erstellt Graphen anhand der Ähnlichkeit zwischen Variablen eines Datensatzes.&lt;br&gt;&lt;br&gt;Ein {0} liefert dieses Ähnlichkeitsmaß. Auf Wunsch können die Eingabedaten skaliert und normalisiert werden; anschließend wird anhand der Korrelationswerte entschieden, ob zwischen den Knoten, die Datenzeilen darstellen, eine Kante erzeugt wird.&lt;br&gt;&lt;br&gt;Nach dem Erstellen des Graphen können die Kanten mithilfe von Transformationen gefiltert werden.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>Korrelationskoeffizient</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Datenauswahl</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Datenauswahl – Auswählen und anpassen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Für Ihren Datensatz wurde automatisch ein Datenbereich ausgewählt. Wenn Sie ihn anpassen möchten, wählen Sie unten die neue Startzelle. Beachten Sie, dass der Datenbereich immer bis zur Zelle ganz unten rechts im Datensatz reicht.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Transponieren</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Beim Transponieren werden die Daten an ihrer Diagonale gespiegelt, d. h. Zeilen werden zu Spalten und umgekehrt.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Datentyp:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Kontinuierlich</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Diskret</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Datentyp</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Kontinuierlich:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Die ausgewählten Daten als Zeilen kontinuierlicher Daten interpretieren. Zwischen den Zeilen wird ein numerisches Korrelationsmaß verwendet.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Diskret:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Die ausgewählten Daten als Zeilen diskreter Werte interpretieren. Die Werte können numerisch sein, sind aber normalerweise textuell. Zwischen den Zeilen wird ein diskretes Korrelationsmaß verwendet.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Als binär behandeln</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Ist dies aktiviert, gelten für den Korrelationsalgorithmus leere Zellen oder Zellen mit den Werten &lt;i&gt;0&lt;/i&gt; oder &lt;i&gt;false&lt;/i&gt; als falsch, alle anderen Werte als wahr.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;WARNUNG: Der ausgewählte Datenbereich scheint kontinuierliche Daten zu enthalten; werden sie als diskret interpretiert, ergibt sich möglicherweise nur eine geringe Korrelation.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korrelation</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Korrelation – Parameter anpassen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Erweitert</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Auf der horizontalen Achse des Diagramms unten stehen Werte von 1 für perfekt korrelierte Zeilen, während 0 keine Korrelation bedeutet. Alle Absolutwerte unterhalb der gewählten minimalen Korrelationsschwelle werden verworfen und nicht zum Erzeugen von Kanten verwendet. Standardmäßig wird eine Transformation hinzugefügt, die Kanten für alle Absolutwerte unterhalb der angegebenen anfänglichen Korrelationsschwelle entfernt.&lt;br&gt;&lt;br&gt;Für mehr Kontrolle über die verwendeten Parameter aktivieren Sie oben rechts die Option &lt;b&gt;Erweitert&lt;/b&gt;.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Algorithmus:</translation>
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
        <translation>Korrelationsalgorithmus</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman-Rang</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Euklidische Ähnlichkeit</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Kosinus-Ähnlichkeit</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polarität:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positiv</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negativ</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Beide</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Polarität</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Standardmäßig werden beim Erstellen des Graphen nur positive Korrelationen berücksichtigt. Das ist fast immer die richtige Einstellung, bei manchen Datenquellen kann es jedoch sinnvoller sein, die Polarität der Korrelation zu berücksichtigen. Beachten Sie, dass nicht alle Korrelationsalgorithmen negative Beziehungen finden können.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputation:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Konstante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Zeileninterpolation</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Spaltenmittelwert</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputation</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Konstante:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Alle fehlenden Werte durch eine Konstante ersetzen.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Zeileninterpolation:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Fehlende Werte linear aus den nächstgelegenen umgebenden Werten der Zeile und ihren relativen Positionen interpolieren. Ist nur ein umgebender Wert vorhanden, wird dieser übernommen. Ist die Zeile leer, wird der Wert auf null gesetzt.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Spaltenmittelwert:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Fehlende Werte durch den Mittelwert ihrer Spalte ersetzen, wobei fehlende Werte ausgenommen sind.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Begrenzung:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorisierung</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>Perzentil</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Begrenzung</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Den maximalen Wert der Datentabelle auf eine angegebene Konstante begrenzen.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorisierung:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Den maximalen Datenwert zeilenweise auf den Wert eines angegebenen Perzentils begrenzen. Wird zum Beispiel das 95. Perzentil gewählt, erhalten die obersten 5 % der Werte den Wert des 95. Perzentils.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Skalierung:</translation>
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
        <translation>Skalierungsarten</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Den Logarithmus von &lt;i&gt;x&lt;/i&gt; + ε zur Basis &lt;i&gt;b&lt;/i&gt; berechnen, wobei &lt;i&gt;x&lt;/i&gt; die Eingabedaten sind und ε eine sehr kleine Konstante ist.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>&lt;i&gt;b&lt;/i&gt; mit &lt;i&gt;x&lt;/i&gt; potenzieren, wobei &lt;i&gt;x&lt;/i&gt; die Eingabedaten sind.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Den Arkussinus von &lt;i&gt;x&lt;/i&gt; berechnen, wobei &lt;i&gt;x&lt;/i&gt; die Eingabedaten sind. Nützlich, wenn eine Log-Transformation benötigt wird, der Datensatz aber negative Zahlen oder Nullen enthält.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalisierung:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Min./Max.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Mittelwert</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Standardisierung</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Einheitsskalierung</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Quantil</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Normalisierungsarten</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min./Max.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Die Daten so umskalieren, dass ihre Werte im Bereich [0, 1] liegen: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Nützlich, wenn die Spalten des Datensatzes unterschiedliche Skalen oder Einheiten haben. Hinweis: Haben alle Elemente einer Spalte denselben Wert, werden die Werte auf 0.0 skaliert.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mittelwert:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Ähnlich wie die Min./Max.-Skalierung, nur dass die resultierenden Werte um ihren Spaltenmittelwert zentriert werden: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Hinweis: Haben alle Elemente einer Spalte denselben Wert, werden die Werte auf 0.0 skaliert.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standardisierung:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Diese auch als Z-Score-Normalisierung bekannte Methode zentriert den Wert um den Mittelwert und skaliert ihn mit der Standardabweichung: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Einheitsskalierung:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Die Werte werden durch die euklidische Länge des Vektors geteilt, den die Datenspalte bildet: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Quantil:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Die Daten so normalisieren, dass die Spalten gleiche Verteilungen haben.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Die Softmax-Funktion wandelt jede Datenspalte in eine Wahrscheinlichkeitsverteilung um, indem jedes Element potenziert und durch die Summe geteilt wird. Dadurch werden die größten Werte jeder Spalte überhöht.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filterung:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Schwellenwert</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filterung</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Schwellenwert:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Einfacher Filter, der Korrelationen unterhalb eines gewählten Schwellenwerts verwirft.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k-nächste Nachbarn:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Korrelationen zwischen Zeilenpaaren erfassen und diejenigen verwerfen, die nicht zu den &lt;i&gt;k&lt;/i&gt; stärksten Korrelationen jeder Zeile gehören. Dies ist besonders nützlich bei extrem stark korrelierten Datensätzen.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Minimaler Korrelationswert</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Der minimale Korrelationswert, oberhalb dessen eine Kante im Graphen erzeugt wird. Ein niedrigerer Minimalwert erhöht den Rechen- und Speicherbedarf.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>Maximales k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Maximales Korrelations-&lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Die maximale Anzahl an Korrelationen, die pro Datenzeile behalten werden, wodurch letztlich die Zahl der im Graphen erzeugten Kanten begrenzt wird. Ein größerer Maximalwert erhöht den Rechen- und Speicherbedarf.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Anfangswert:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>Anfängliches k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Korrelationsschwelle</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Korrelations-&lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Die anfängliche Korrelationsschwelle bestimmt die Größe des resultierenden Graphen. Ein niedrigerer Wert filtert weniger Kanten und ergibt einen größeren Graphen. Dieser Wert kann später, nach dem Erstellen des Graphen, geändert werden.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Der anfängliche k-Wert bestimmt die Größe des resultierenden Graphen. Ein höherer Wert filtert weniger Kanten und ergibt einen größeren Graphen. Dieser Wert kann später, nach dem Erstellen des Graphen, geändert werden.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>In Arbeit…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Leerer Graph</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Vorlage</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Vorlage&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Eine Vorlage ist eine benannte Zusammenstellung von Transformationen und Visualisierungen, die nach dem Erstellen auf den Graphen angewendet werden kann. Bitte wählen Sie eine Vorlage aus. &lt;b&gt;Hinweis:&lt;/b&gt; Hier angewendete Transformationen werden immer bedingungslos angehängt, unabhängig von der Anwendungsweise der jeweiligen Vorlage.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Als Standard festlegen</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Zusammenfassung&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Ein Graph wird mit den folgenden Parametern erstellt:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Transponiert&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Datenbereich: [ Spalte: {0} Zeile: {1} Breite: {2} Höhe: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Minimaler Korrelationswert: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Anfängliche Korrelationsschwelle: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Maximaler k-Wert: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Anfänglicher k-Wert: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Korrelationsmaß für kontinuierliche Daten: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Korrelationspolarität: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputation: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputation: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Begrenzung: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Begrenzung: {0} ({1}. Perzentil)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Begrenzung: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Skalierung: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalisierung: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Korrelationsmaß für diskrete Daten: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Vorlage: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Geschätzte Graphgröße vor Transformationen: {0}{3} Knoten{2}, {1}{4} Kanten{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Eine Erhöhung des minimalen Korrelationswerts verkleinert den Graphen in der Regel.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Eine Verringerung des maximalen &lt;i&gt;k&lt;/i&gt; verkleinert den Graphen in der Regel.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}WARNUNG: Dies ist ein sehr großer Graph, der die Systemressourcen erschöpfen und zu Instabilität oder Einfrieren führen kann. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}WARNUNG: Der erzeugte Graph wird wahrscheinlich leer sein.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 Spalte</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n Spalte</numerusform>
            <numerusform>%n Spalten</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…und %1 weitere</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Vergrößern, um Spalteninformationen anzuzeigen</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Vergrößern, um Spaltennamen anzuzeigen</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n eindeutiger Wert</numerusform>
            <numerusform>%n eindeutige Werte</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Mittelwert von %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Mittelwert der Auswahl</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Median von %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Median der Auswahl</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Mittleres Histogramm von %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Mittleres Histogramm der Auswahl</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Abweichung</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 oben</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 unten</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Std.-Abw.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Std.-Fehler</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 und %3 weitere…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>Die Dateiendung von %1 deutet auf „%2“ hin, der Inhalt konnte jedoch nicht bestimmt werden.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>Die Dateiendung von %1 deutet auf „%2“ hin, der Inhalt ähnelt jedoch einer „%3“-Datei.</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korrelation</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Erstellt einen Graphen, dessen Knoten Datenzeilen und dessen Kanten Korrelationen zwischen diesen Zeilen darstellen.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>Der Datenwert ist ein parametrisiertes Attribut, mit dem sich eine bestimmte Spalte der korrelierten Daten referenzieren lässt.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Der Name der Datenspalte mit dem größten Absolutwert in der dem Knoten zugeordneten Datenzeile.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>Der mittlere Datenwert ist der Mittelwert der dem Knoten zugeordneten Werte.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>Der minimale Datenwert ist der kleinste dem Knoten zugeordnete Wert.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>Der maximale Datenwert ist der größte dem Knoten zugeordnete Wert.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>Die %1 ist ein Maß für die Streuung der dem Knoten zugeordneten Werte. Sie ist definiert als ∑(&lt;i&gt;x&lt;/i&gt;-µ)², wobei &lt;i&gt;x&lt;/i&gt; der Wert und µ der Mittelwert ist.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianz</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>Die %1 ist ein Maß für die Streuung der dem Knoten zugeordneten Werte. Sie ist definiert als √∑(&lt;i&gt;x&lt;/i&gt;-µ)², wobei &lt;i&gt;x&lt;/i&gt; der Wert und µ der Mittelwert ist.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standardabweichung</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>Der %1 ist ein Maß für die Streuung der dem Knoten zugeordneten Werte. Er ist definiert als die Standardabweichung geteilt durch den Mittelwert.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Variationskoeffizient</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>In den Plugin-Daten fehlt der Schlüssel „%1“.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Datenbereich: [ Spalte: %1 Zeile: %2 Breite: %3 Höhe: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Transponiert</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Korrelationsmaß für kontinuierliche Daten: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positiv</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negativ</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Beide</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Korrelationspolarität: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Korrelationsmaß für diskrete Daten: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Minimaler Korrelationswert: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Maximaler k-Wert: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Konstante (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Spaltenmittelwert</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Zeileninterpolation</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputation: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Begrenzung: Konstante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Begrenzung: Winsorisierung (%1. Perzentil)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Skalierung: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Skalierung: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Skalierung: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Skalierung: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Skalierung: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalisierung: Min./Max.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalisierung: Quantil</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalisierung: Mittelwert</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalisierung: Standardisierung</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalisierung: Einheitsskalierung</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalisierung: Softmax-Skalierung</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Spaltenannotationen importieren</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Keine geeigneten Schlüssel</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>{0} konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} wird geladen…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Einführung</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Annotationen importieren&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Annotationsdaten können aus einer externen Quelle importiert und über das Diagramm gelegt werden. Jedes der unterstützten tabellarischen Datenformate kann verwendet werden. Nach dem Import lassen sich neue Annotationen wie gewohnt sortieren und gruppieren.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Schlüsselauswahl</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Schlüsselauswahl&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Um Annotationsdaten zu importieren, muss die Zeile der Eingabedaten bestimmt werden, die den Spaltennamen der vorhandenen Daten entspricht. Damit werden Spalten der Tabelle den Spalten im Diagramm zugeordnet. Bitte wählen Sie unten die richtige Zeile aus. Alternativ klicken Sie auf &lt;i&gt;{0}&lt;/i&gt;, dann wird die beste Übereinstimmung automatisch ermittelt.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Automatisch erkennen</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>Die ausgewählte Zeile &lt;b&gt;{0}&lt;/b&gt; enthält doppelte Werte, einige Spalten werden ignoriert!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>{0} % Übereinstimmung gefunden</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Keine Übereinstimmung gefunden</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Annotationsauswahl</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Annotationsauswahl&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Bitte wählen Sie die gewünschten Annotationen aus der Liste rechts aus. Mehrere Annotationen wählen Sie mit gedrückter &lt;i&gt;Strg&lt;/i&gt;-Taste aus, einen Bereich mit gedrückter &lt;i&gt;Umschalt&lt;/i&gt;-Taste und Klicken.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Vorhandene Annotationen ersetzen</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Alles auswählen</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Nichts auswählen</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Zusammenfassung&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Schlüssel wird erkannt…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Die folgenden Annotationen werden mit der Zeile &lt;b&gt;{0}{1}&lt;/b&gt; als Schlüssel importiert:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(enthält doppelte Werte)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Annotationen können erst importiert werden, wenn ein Schlüssel ausgewählt ist.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Schlüssel wird erkannt</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Oberfläche &amp;nebeneinander anzeigen</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>&amp;Spaltennamen anzeigen</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Interessante Zeilen finden</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Sichtbare Spaltenannotationen auswählen</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Gitterlinien</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legende</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Null in Y-Achse einbeziehen</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Ausreißer anzeigen</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Zoom zurücksetzen</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Alle Spalten anzeigen</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Keine</translation>
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
        <translation>Mittelwertzentrierung</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Skalierung auf Einheitsvarianz</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Pareto-Skalierung</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Fehlerbalken</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Fläche</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standardabweichung</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Standardfehler</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Einzellinie</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Mittelwertlinie</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Medianlinie</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Mittleres Histogramm</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Interquartilsabstand</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Nach Annotation gruppieren</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Annotationen aus Tabelle importieren…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-Korrelationsdiagramm</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Als &amp;Bild speichern…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>X-Achsenbeschriftung festlegen…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Y-Achsenbeschriftung festlegen…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>X-Achsenbeschriftung</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Bitte geben Sie die X-Achsenbeschriftung ein:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Y-Achsenbeschriftung</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Bitte geben Sie die Y-Achsenbeschriftung ein:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Diagramm</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Achsenbeschriftungen</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Skalierung</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Nach Attribut</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Mittelung</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Alle</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Streuung</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Spalten sortieren nach</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Gruppen einfärben nach</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Als Bilder speichern nach…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Zeile</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Natürliche Reihenfolge</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Spaltenname</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Datenwert</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Hierarchisches Clustering</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Klicken Sie auf eine Spalte oder Spaltenannotation, um sie auszuwählen. Halten Sie &lt;i&gt;Strg&lt;/i&gt; gedrückt und klicken Sie, um mehrere Spalten auszuwählen.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Sortierreihenfolge nach Datenwert fixieren</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Diagramm als Bild speichern</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>PNG-Bild (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>JPEG-Bild (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>PDF-Dokument (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Alle Annotationen anzeigen</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Alle Annotationen ausblenden</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Relativ</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Absolut</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Auswahlgewichtung</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Zeilen werden ausgewählt, indem die Summe der ausgewählten Spalten gegen die nicht ausgewählten Spalten gewichtet wird. Eine Gewichtung der ausgewählten Spalten (&lt;i&gt;Absolut&lt;/i&gt;) findet tendenziell Zeilen mit Spitzen in diesen Spalten, unabhängig von den nicht ausgewählten Spalten. Eine (negative) Gewichtung der nicht ausgewählten Spalten (&lt;i&gt;Relativ&lt;/i&gt;) findet dagegen tendenziell Zeilen, die gerade in den ausgewählten Spalten signifikante Werte haben.</translation>
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
        <translation>Perzentil</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Die Anzahl der auszuwählenden und zu ordnenden Zeilen.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Neu auswählen</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Interquartilsabstand von %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Interquartilsabstand</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Diagrammbilder werden gespeichert</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Der Pearson-Korrelationskoeffizient gibt die lineare Korrelation zwischen zwei Variablen an.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Pearson-Korrelationswert</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>Der %1 gibt den linearen Zusammenhang zwischen zwei Variablen an.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Pearson-Korrelationskoeffizient</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman-Rang</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Der Rangkorrelationskoeffizient nach Spearman ist ein nichtparametrisches Maß für die statistische Abhängigkeit zwischen den Rängen zweier Variablen. Er bewertet, wie gut sich der Zusammenhang zwischen zwei Variablen durch eine monotone Funktion beschreiben lässt.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Spearman-Rangkorrelationswert</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>Der %1 gibt den monotonen Zusammenhang zwischen zwei Variablen an.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Spearman-Rangkorrelationskoeffizient</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Euklidische Ähnlichkeit</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Die euklidische Ähnlichkeit ist im Wesentlichen der Kehrwert des euklidischen Abstands zwischen zwei Vektoren.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Die %1 ist im Wesentlichen der Kehrwert des euklidischen Abstands zwischen zwei Vektoren.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Kosinus-Ähnlichkeit</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Die Kosinus-Ähnlichkeit ist ein Maß für die Ähnlichkeit zweier vom Nullvektor verschiedener Vektoren eines Innenproduktraums. Sie ist als Kosinus des Winkels zwischen ihnen definiert, was dem Skalarprodukt derselben, jeweils auf Länge 1 normierten Vektoren entspricht.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>Die %1 ist ein Maß für die Ähnlichkeit zweier vom Nullvektor verschiedener Vektoren eines Innenproduktraums.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Die Biweight Midcorrelation (auch Bicor genannt) ist ein Maß für die Ähnlichkeit zwischen Stichproben. Sie basiert auf dem Median statt auf dem Mittelwert und ist daher weniger anfällig für Ausreißer.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Bicor-Korrelationswert</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 ist ein auf dem Median basierendes Maß für die Ähnlichkeit zwischen Stichproben.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Der Jaccard-Index ist eine Kennzahl zur Beurteilung der Ähnlichkeit und Diversität von Stichprobenmengen.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Jaccard-Korrelationswert</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Der %1 ist eine Kennzahl zur Beurteilung der Ähnlichkeit und Diversität von Stichprobenmengen.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Jaccard-Index</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Einfacher Übereinstimmungskoeffizient</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Der einfache Übereinstimmungskoeffizient ist eine Kennzahl zur Beurteilung der Ähnlichkeit und Diversität von Stichprobenmengen. Er entspricht dem Jaccard-Index, zählt aber zusätzlich gemeinsames Fehlen.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Einfacher Übereinstimmungskoeffizient (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Datenwert</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Stärkste Datenspalte</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Mittlerer Datenwert</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Minimaler Datenwert</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Maximaler Datenwert</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianz</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standardabweichung</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Variationskoeffizient</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>Absolute %1</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Daten</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Korrelationswerte</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Korrelations-CSV-Datei</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Korrelations-CSV-Dateien</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Korrelations-TSV-Datei</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Korrelations-TSV-Dateien</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Korrelations-SSV-Datei</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Korrelations-SSV-Dateien</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Korrelations-Excel-Datei</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Korrelations-Excel-Dateien</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Korrelation wird berechnet</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Clustering</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Spaltenannotationen importieren</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Spaltenannotation importieren</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Spaltenannotationen werden importiert</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Spaltenannotation wird importiert</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n Annotation importiert</numerusform>
            <numerusform>%n Annotationen importiert</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Annotation %1 importiert</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attribute</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalisierung</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Korrelation</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Graph wird aufgebaut</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Spalten werden sortiert</translation>
    </message>
</context>
</TS>
