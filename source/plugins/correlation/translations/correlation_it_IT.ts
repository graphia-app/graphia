<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it_IT" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Impossibile caricare {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introduzione</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Analisi del grafo di correlazione&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>Il plugin di correlazione crea grafi basati sulla somiglianza tra le variabili di un set di dati.&lt;br&gt;&lt;br&gt;Un {0} fornisce questa misura di somiglianza. Se specificato, i dati di input possono essere scalati e normalizzati; dopodiché i punteggi di correlazione servono a stabilire se creare o meno un arco tra i nodi che rappresentano le righe di dati.&lt;br&gt;&lt;br&gt;Una volta creato il grafo, gli archi possono essere filtrati con le trasformazioni.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>coefficiente di correlazione</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Selezione dei dati</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selezione dei dati - Seleziona e regola&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Per il tuo set di dati è stato selezionato automaticamente un dataframe. Se vuoi modificarlo, seleziona qui sotto la nuova cella iniziale. Il dataframe si estende sempre fino alla cella in basso a destra del set di dati.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Trasponi</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>La trasposizione ribalta i dati lungo la diagonale, trasformando le righe in colonne e viceversa.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Tipo di dati:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Continui</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discreti</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Tipo di dati</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Continui:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpreta i dati selezionati come righe di dati continui. Tra le righe verrà usata una misura di correlazione numerica.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discreti:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpreta i dati selezionati come righe di valori discreti. I valori possono essere numerici, ma di norma sono testuali. Tra le righe verrà usata una misura di correlazione discreta.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Tratta come binari</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Se questa opzione è attiva, ai fini dell’algoritmo di correlazione le celle vuote o contenenti i valori &lt;i&gt;0&lt;/i&gt; o &lt;i&gt;false&lt;/i&gt; sono considerate false, mentre tutti gli altri valori sono considerati veri.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;ATTENZIONE: il dataframe selezionato sembra contenere dati continui; interpretarli come discreti potrebbe produrre solo un basso livello di correlazione.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlazione</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlazione - Regola i parametri&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Avanzate</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Sull’asse orizzontale del grafico qui sotto, i valori pari a 1 rappresentano righe perfettamente correlate, mentre 0 indica assenza di correlazione. Tutti i valori assoluti inferiori alla soglia minima di correlazione selezionata vengono scartati e non saranno usati per creare archi. Per impostazione predefinita viene aggiunta una trasformazione che rimuove gli archi per tutti i valori assoluti inferiori alla soglia di correlazione iniziale specificata.&lt;br&gt;&lt;br&gt;Per un maggiore controllo sui parametri usati, attiva l’opzione &lt;b&gt;Avanzate&lt;/b&gt; in alto a destra.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Algoritmo:</translation>
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
        <translation>Algoritmo di correlazione</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rango di Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Somiglianza euclidea</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Somiglianza del coseno</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polarità:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positiva</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negativa</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Entrambe</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Polarità</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Per impostazione predefinita, nella creazione del grafo vengono considerate solo le correlazioni positive. Quasi sempre è l’impostazione corretta, ma per alcune fonti di dati può avere più senso tenere conto della polarità della correlazione. Non tutti gli algoritmi di correlazione sono in grado di trovare relazioni negative.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputazione:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Costante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolazione per riga</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Media della colonna</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputazione</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Costante:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Sostituisci tutti i valori mancanti con una costante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Interpolazione per riga:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Interpola linearmente i valori mancanti usando i valori circostanti più vicini nella riga e le loro posizioni relative. Se è disponibile un solo valore circostante, verrà usato quello. Se la riga è vuota, il valore verrà impostato a zero.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Media della colonna:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Sostituisci i valori mancanti con il valore medio della loro colonna, esclusi i valori mancanti.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Limitazione:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nessuno</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorizzazione</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>percentile</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Limitazione</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Limita il valore massimo della tabella dei dati a una costante specificata.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorizzazione:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Limita il valore massimo dei dati a quello di un percentile specificato, riga per riga. Ad esempio, se viene scelto il 95° percentile, il 5% superiore dei valori verrà impostato al valore del 95° percentile.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Scala:</translation>
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
        <translation>Tipi di scala</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Calcola il logaritmo in base &lt;i&gt;b&lt;/i&gt; di &lt;i&gt;x&lt;/i&gt; + ε, dove &lt;i&gt;x&lt;/i&gt; sono i dati di input ed ε è una costante molto piccola.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Eleva &lt;i&gt;b&lt;/i&gt; alla potenza &lt;i&gt;x&lt;/i&gt;, dove &lt;i&gt;x&lt;/i&gt; sono i dati di input.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Calcola l’arcoseno di &lt;i&gt;x&lt;/i&gt;, dove &lt;i&gt;x&lt;/i&gt; sono i dati di input. È utile quando serve una trasformazione logaritmica ma il set di dati contiene numeri negativi o zeri.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalizzazione:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Min/Max</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Standardizzazione</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Scala unitaria</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Quantile</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Tipi di normalizzazione</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min/Max:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Riscala i dati in modo che i valori rientrino nell’intervallo [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). È utile se le colonne del set di dati hanno scale o unità diverse. Nota: se tutti gli elementi di una colonna hanno lo stesso valore, i valori verranno riscalati a 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Media:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Simile alla scala Min/Max, ma i valori risultanti sono centrati sulla media della colonna: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Nota: se tutti gli elementi di una colonna hanno lo stesso valore, i valori verranno riscalati a 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standardizzazione:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Nota anche come normalizzazione Z-score, questo metodo centra il valore sulla media e lo scala per la deviazione standard: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Scala unitaria:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>I valori vengono divisi per la lunghezza euclidea del vettore formato dalla colonna di dati: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Quantile:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Normalizza i dati in modo che le colonne abbiano distribuzioni uguali.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>La funzione softmax converte ogni colonna di dati in una distribuzione di probabilità elevando a esponenziale ogni elemento e normalizzando con la divisione per la somma. In definitiva, ciò amplifica i valori più grandi di ogni colonna.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filtro:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Soglia</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filtraggio</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Soglia:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Filtro semplice che scarta le correlazioni inferiori a una soglia selezionata.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k vicini più prossimi:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Tiene traccia delle correlazioni trovate tra coppie di righe, scartando quelle che non rientrano tra le prime &lt;i&gt;k&lt;/i&gt; correlazioni di ciascuna riga. È particolarmente utile per set di dati con correlazione estremamente elevata.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimo:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Valore di correlazione minimo</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Il valore di correlazione minimo oltre il quale verrà creato un arco nel grafo. Un valore minimo più basso aumenta i requisiti di calcolo e di memoria.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>k massimo:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; di correlazione massimo</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Il numero massimo di correlazioni da mantenere per ogni riga di dati, che in definitiva limita il numero di archi creati nel grafo. Un valore massimo più alto aumenta i requisiti di calcolo e di memoria.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Iniziale:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>k iniziale:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Soglia di correlazione</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; di correlazione</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>La soglia di correlazione iniziale determina la dimensione del grafo risultante. Un valore più basso filtra meno archi e produce un grafo più grande. Questo valore può essere modificato in seguito, dopo la creazione del grafo.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Il valore k iniziale determina la dimensione del grafo risultante. Un valore più alto filtra meno archi e produce un grafo più grande. Questo valore può essere modificato in seguito, dopo la creazione del grafo.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Elaborazione…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Grafo vuoto</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Modello</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Modello&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Un modello è un insieme denominato di trasformazioni e visualizzazioni che può essere applicato al grafo dopo la sua creazione. Seleziona un modello da applicare. &lt;b&gt;Nota:&lt;/b&gt; le trasformazioni applicate qui verranno sempre aggiunte incondizionatamente, indipendentemente dal metodo di applicazione specifico del modello.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Imposta come predefinito</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Riepilogo</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Riepilogo&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Verrà creato un grafo con i seguenti parametri:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Trasposto&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Dataframe: [ Colonna: {0} Riga: {1} Larghezza: {2} Altezza: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Valore di correlazione minimo: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Soglia di correlazione iniziale: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Valore k massimo: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Valore k iniziale: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Misura di correlazione continua: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Polarità della correlazione: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputazione: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputazione: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Limitazione: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Limitazione: {0} ({1}° percentile)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Limitazione: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Scala: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalizzazione: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Misura di correlazione discreta: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Modello: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Dimensione stimata del grafo prima delle trasformazioni: {0}{3} nodi{2}, {1}{4} archi{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentare il valore di correlazione minimo di solito riduce la dimensione del grafo.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Ridurre il &lt;i&gt;k&lt;/i&gt; massimo di solito riduce la dimensione del grafo.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ATTENZIONE: questo è un grafo molto grande, che potrebbe esaurire le risorse di sistema e causare instabilità o blocchi. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}ATTENZIONE: è probabile che il grafo generato sia vuoto.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 colonna</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n colonna</numerusform>
            <numerusform>%n colonne</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…e altri %1</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Ridimensiona per mostrare le informazioni sulle colonne</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Ridimensiona per mostrare i nomi delle colonne</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n valore univoco</numerusform>
            <numerusform>%n valori univoci</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Media di %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Media della selezione</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Mediana di %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Mediana della selezione</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Istogramma medio di %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Istogramma medio della selezione</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Deviazione</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 superiore</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 inferiore</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Dev. std.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Err. std.</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 e altri %3…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>L’estensione di %1 indica che si tratta di un file “%2”, ma non è stato possibile determinarne il contenuto.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>L’estensione di %1 indica che si tratta di un file “%2”, ma il contenuto somiglia a un file “%3”.</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlazione</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Crea un grafo in cui i nodi rappresentano righe di dati e gli archi rappresentano le correlazioni tra tali righe.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>Il valore dei dati è un attributo parametrizzato che consente di fare riferimento a una colonna specifica dei dati correlati.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Il nome della colonna di dati con il valore assoluto più grande per la riga di dati associata al nodo.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>Il valore medio dei dati è la media dei valori associati al nodo.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>Il valore minimo dei dati è il valore più piccolo associato al nodo.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>Il valore massimo dei dati è il valore più grande associato al nodo.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>La %1 è una misura della dispersione dei valori associati al nodo. È definita come ∑(&lt;i&gt;x&lt;/i&gt;-µ)², dove &lt;i&gt;x&lt;/i&gt; è il valore e µ è la media.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianza</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>La %1 è una misura della dispersione dei valori associati al nodo. È definita come √∑(&lt;i&gt;x&lt;/i&gt;-µ)², dove &lt;i&gt;x&lt;/i&gt; è il valore e µ è la media.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Deviazione standard</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>Il %1 è una misura della dispersione dei valori associati al nodo. È definito come la deviazione standard divisa per la media.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coefficiente di variazione</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>Nei dati del plugin manca la chiave “%1”.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Dataframe: [ Colonna: %1 Riga: %2 Larghezza: %3 Altezza: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Trasposto</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Misura di correlazione continua: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positiva</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negativa</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Entrambe</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Polarità della correlazione: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Misura di correlazione discreta: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Valore di correlazione minimo: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Valore k massimo: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Costante (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Media della colonna</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolazione per riga</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputazione: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Limitazione: costante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Limitazione: winsorizzazione (%1° percentile)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Scala: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Scala: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Scala: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Scala: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Scala: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalizzazione: Min/Max</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalizzazione: quantile</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalizzazione: media</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalizzazione: standardizzazione</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalizzazione: scala unitaria</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalizzazione: scala softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importa annotazioni delle colonne</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Nessuna chiave adatta</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Impossibile caricare {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introduzione</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Importa annotazioni&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>I dati di annotazione possono essere importati da una fonte esterna e sovrapposti al grafico. È possibile usare uno qualsiasi dei formati di dati tabellari supportati. Dopo l’importazione, le nuove annotazioni possono essere ordinate e raggruppate nel modo consueto.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Selezione delle chiavi</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selezione delle chiavi&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Per importare i dati di annotazione è necessario individuare la riga dei dati di input che corrisponde ai nomi delle colonne dei dati esistenti. Serve ad associare le colonne della tabella alle colonne del grafico. Seleziona qui sotto la riga corretta. In alternativa, fai clic su &lt;i&gt;{0}&lt;/i&gt; e la corrispondenza migliore verrà individuata automaticamente.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Rilevamento automatico</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>La riga selezionata &lt;b&gt;{0}&lt;/b&gt; contiene valori duplicati, alcune colonne verranno ignorate!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Trovata corrispondenza del {0}%</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Nessuna corrispondenza trovata</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Selezione delle annotazioni</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selezione delle annotazioni&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Seleziona le annotazioni necessarie dall’elenco a destra. Puoi selezionare più annotazioni tenendo premuto &lt;i&gt;Ctrl&lt;/i&gt;, oppure un intervallo tenendo premuto &lt;i&gt;Maiusc&lt;/i&gt; e facendo clic dove serve.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Sostituisci le annotazioni esistenti</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Seleziona tutto</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Deseleziona tutto</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Riepilogo</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Riepilogo&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Rilevamento della chiave…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Le seguenti annotazioni verranno importate usando la riga &lt;b&gt;{0}{1}&lt;/b&gt; come chiave:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(contiene valori duplicati)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Le annotazioni non possono essere importate se non viene selezionata una chiave.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Rilevamento della chiave</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Mostra l’interfaccia &amp;affiancata</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Mostra i nomi delle &amp;colonne</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Trova righe di interesse</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Seleziona le annotazioni delle colonne visibili</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Linee della griglia</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Includi lo zero nell’asse Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Mostra valori anomali</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Reimposta zoom</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Mostra tutte le colonne</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nessuno</translation>
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
        <translation>Centratura sulla media</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Scala a varianza unitaria</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Scala di Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Barre di errore</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Area</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Deviazione standard</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Errore standard</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Linea singola</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Linea della media</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Linea della mediana</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Istogramma medio</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Scarto interquartile</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Raggruppa per annotazione</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Importa annotazioni da tabella…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-grafico-correlazione</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Salva come &amp;immagine…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Imposta etichetta asse X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Imposta etichetta asse Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Etichetta asse X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Inserisci l’etichetta dell’asse X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Etichetta asse Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Inserisci l’etichetta dell’asse Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Grafico</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Etichette degli assi</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Scala</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Per attributo</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Tutti</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Dispersione</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Ordina le colonne per</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Colora i gruppi per</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Salva come immagini per…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Riga</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Ordine naturale</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Nome colonna</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valore dei dati</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Clustering gerarchico</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Fai clic su una colonna o su un’annotazione di colonna per selezionarla. Tieni premuto &lt;i&gt;Ctrl&lt;/i&gt; e fai clic per selezionare più colonne.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Fissa l’ordinamento per valore dei dati</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Salva grafico come immagine</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Immagine PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Immagine JPEG (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>Documento PDF (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Mostra tutte le annotazioni</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Nascondi tutte le annotazioni</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Relativa</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Assoluta</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Ponderazione della selezione</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Le righe vengono selezionate ponderando la somma delle colonne selezionate rispetto alle colonne non selezionate. Ponderare per le colonne selezionate (indicato come &lt;i&gt;Assoluta&lt;/i&gt;) tende a trovare le righe che hanno picchi in quelle colonne, indipendentemente dalle colonne non selezionate. Al contrario, ponderare (negativamente) per le colonne non selezionate (indicato come &lt;i&gt;Relativa&lt;/i&gt;) tende a trovare le righe con valori significativi proprio nelle colonne selezionate.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}°</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}°</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}°</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}°</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Percentile</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Il numero di righe da selezionare e classificare.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Riseleziona</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Scarto interquartile di %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Scarto interquartile</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Salvataggio delle immagini del grafico</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Il coefficiente di correlazione di Pearson indica la correlazione lineare tra due variabili.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Valore di correlazione di Pearson</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>Il %1 indica la relazione lineare tra due variabili.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Coefficiente di correlazione di Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rango di Spearman</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Il coefficiente di correlazione per ranghi di Spearman è una misura non parametrica della dipendenza statistica tra i ranghi di due variabili. Valuta quanto bene la relazione tra due variabili possa essere descritta da una funzione monotona.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Valore di correlazione per ranghi di Spearman</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>Il %1 indica la relazione monotona tra due variabili.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Coefficiente di correlazione per ranghi di Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Somiglianza euclidea</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La somiglianza euclidea è essenzialmente l’inverso della distanza euclidea tra due vettori.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La %1 è essenzialmente l’inverso della distanza euclidea tra due vettori.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Somiglianza del coseno</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>La somiglianza del coseno è una misura di somiglianza tra due vettori non nulli di uno spazio dotato di prodotto interno. È definita come il coseno dell’angolo tra di essi, che equivale anche al prodotto interno degli stessi vettori normalizzati entrambi a lunghezza 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>La %1 è una misura di somiglianza tra due vettori non nulli di uno spazio dotato di prodotto interno.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>La Biweight Midcorrelation (detta anche Bicor) è una misura di somiglianza tra campioni. Si basa sulla mediana anziché sulla media, quindi è meno sensibile ai valori anomali.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Valore di correlazione Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 è una misura di somiglianza tra campioni basata sulla mediana.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>L’indice di Jaccard è una statistica usata per valutare la somiglianza e la diversità di insiemi di campioni.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Valore di correlazione di Jaccard</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>L’%1 è una statistica usata per valutare la somiglianza e la diversità di insiemi di campioni.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Indice di Jaccard</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Coefficiente di corrispondenza semplice</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Il coefficiente di corrispondenza semplice è una statistica usata per valutare la somiglianza e la diversità di insiemi di campioni. È identico a Jaccard, salvo che conta anche le assenze reciproche.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Coefficiente di corrispondenza semplice (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valore dei dati</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Colonna di dati più forte</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Valore medio dei dati</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Valore minimo dei dati</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Valore massimo dei dati</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianza</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Deviazione standard</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coefficiente di variazione</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 assoluta</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Dati</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Valori di correlazione</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>File CSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>File CSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>File TSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>File TSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>File SSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>File SSV di correlazione</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>File Excel di correlazione</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>File Excel di correlazione</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Calcolo della correlazione</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Clustering</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importa annotazioni delle colonne</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Importa annotazione di colonna</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Importazione delle annotazioni delle colonne</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Importazione dell’annotazione di colonna</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n annotazione importata</numerusform>
            <numerusform>%n annotazioni importate</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Annotazione %1 importata</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributi</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalizzazione</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlazione</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Costruzione del grafo</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Ordinamento delle colonne</translation>
    </message>
</context>
</TS>
