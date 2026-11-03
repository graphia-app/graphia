<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="nl_NL" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Laden van {0} mislukt.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} laden…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Inleiding</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlatiegraafanalyse&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>De correlatieplug-in maakt grafen op basis van de gelijkenis tussen variabelen in een gegevensset.&lt;br&gt;&lt;br&gt;Een {0} levert deze gelijkenismaat. Desgewenst kunnen de invoergegevens worden geschaald en genormaliseerd, waarna correlatiescores bepalen of er een verbinding wordt gemaakt tussen de knooppunten die rijen gegevens voorstellen.&lt;br&gt;&lt;br&gt;Nadat de graaf is gemaakt, kunnen de verbindingen met transformaties worden gefilterd.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>correlatiecoëfficiënt</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Gegevensselectie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Gegevensselectie - Selecteren en aanpassen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Er is automatisch een dataframe voor uw gegevensset geselecteerd. Als u dit wilt aanpassen, selecteert u hieronder de nieuwe begincel. Het dataframe loopt altijd door tot de cel helemaal rechtsonder in de gegevensset.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Transponeren</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Transponeren spiegelt de gegevens over de diagonaal, zodat rijen kolommen worden en omgekeerd.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Gegevenstype:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Continu</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discreet</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Gegevenstype</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Continu:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>De geselecteerde gegevens interpreteren als rijen continue gegevens. Tussen rijen wordt een numerieke correlatiemaat gebruikt.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discreet:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>De geselecteerde gegevens interpreteren als rijen discrete waarden. De waarden kunnen numeriek zijn, maar zijn normaal gesproken tekstueel. Tussen rijen wordt een discrete correlatiemaat gebruikt.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Als binair behandelen</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Als dit is ingeschakeld, worden voor het correlatiealgoritme lege cellen en cellen met de waarden &lt;i&gt;0&lt;/i&gt; of &lt;i&gt;false&lt;/i&gt; als onwaar beschouwd, en alle andere waarden als waar.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;WAARSCHUWING: het geselecteerde dataframe lijkt continue gegevens te bevatten; als discreet interpreteren levert mogelijk slechts een lage mate van correlatie op.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlatie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlatie - Parameters aanpassen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Geavanceerd</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Op de horizontale as van de grafiek hieronder staat 1 voor perfect gecorreleerde rijen, terwijl 0 geen correlatie aangeeft. Alle absolute waarden onder de geselecteerde minimale correlatiedrempel worden genegeerd en niet gebruikt om verbindingen te maken. Standaard wordt een transformatie toegevoegd die verbindingen verwijdert voor alle absolute waarden onder de opgegeven initiële correlatiedrempel.&lt;br&gt;&lt;br&gt;Voor meer controle over de gebruikte parameters vinkt u rechtsboven de optie &lt;b&gt;Geavanceerd&lt;/b&gt; aan.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Algoritme:</translation>
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
        <translation>Correlatiealgoritme</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman-rang</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Euclidische gelijkenis</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Cosinusgelijkenis</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polariteit:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positief</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negatief</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Beide</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Polariteit</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Standaard worden bij het maken van de graaf alleen positieve correlaties meegenomen. Dat is bijna altijd de juiste instelling, maar bij sommige gegevensbronnen kan het zinvoller zijn rekening te houden met de polariteit van de correlatie. Niet alle correlatiealgoritmen kunnen negatieve verbanden vinden.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputatie:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Constante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Rij-interpolatie</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Kolomgemiddelde</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputatie</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Constante:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Alle ontbrekende waarden vervangen door een constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Rij-interpolatie:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Ontbrekende waarden lineair interpoleren met behulp van de dichtstbijzijnde omringende waarden in de rij en hun relatieve posities. Als er maar één omringende waarde beschikbaar is, wordt de waarde daaraan gelijkgesteld. Als de rij leeg is, wordt de waarde op nul gezet.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Kolomgemiddelde:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Ontbrekende waarden vervangen door de gemiddelde waarde van hun kolom, exclusief ontbrekende waarden.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Afkappen:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorisatie</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>percentiel</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Afkappen</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>De maximale waarde van de gegevenstabel beperken tot een opgegeven constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorisatie:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>De maximale gegevenswaarde per rij beperken tot die van een opgegeven percentiel. Als bijvoorbeeld het 95e percentiel wordt gekozen, krijgen de hoogste 5% van de waarden de waarde van het 95e percentiel.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Schalen:</translation>
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
        <translation>Schaaltypen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>De logaritme van &lt;i&gt;x&lt;/i&gt; + ε met grondtal &lt;i&gt;b&lt;/i&gt; berekenen, waarbij &lt;i&gt;x&lt;/i&gt; de invoergegevens zijn en ε een zeer kleine constante is.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>&lt;i&gt;b&lt;/i&gt; verheffen tot de macht &lt;i&gt;x&lt;/i&gt;, waarbij &lt;i&gt;x&lt;/i&gt; de invoergegevens zijn.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>De arcsinus van &lt;i&gt;x&lt;/i&gt; berekenen, waarbij &lt;i&gt;x&lt;/i&gt; de invoergegevens zijn. Dit is handig als u een logtransformatie nodig hebt maar de gegevensset negatieve getallen of nullen bevat.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalisatie:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Min./max.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Gemiddelde</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Standaardisatie</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Eenheidsschaling</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Kwantiel</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Normalisatietypen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min./max.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>De gegevens herschalen zodat de waarden binnen het bereik [0, 1] liggen: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Dit is handig als de kolommen in de gegevensset verschillende schalen of eenheden hebben. Opmerking: als alle elementen in een kolom dezelfde waarde hebben, worden de waarden herschaald naar 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Gemiddelde:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Vergelijkbaar met min./max.-schaling, behalve dat de resulterende waarden rond het kolomgemiddelde worden gecentreerd: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Opmerking: als alle elementen in een kolom dezelfde waarde hebben, worden de waarden herschaald naar 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standaardisatie:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Deze methode, ook bekend als Z-scorenormalisatie, centreert de waarde rond het gemiddelde en schaalt met de standaardafwijking: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Eenheidsschaling:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Waarden worden gedeeld door de euclidische lengte van de vector die door de gegevenskolom wordt gevormd: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Kwantiel:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>De gegevens normaliseren zodat de kolommen gelijke verdelingen hebben.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>De softmaxfunctie zet elke gegevenskolom om in een kansverdeling door elk element exponentieel te verheffen en te normaliseren door te delen door de som. Uiteindelijk worden zo de grootste waarden in elke kolom uitvergroot.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filteren:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Drempel</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filteren</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Drempel:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Eenvoudig filter waarbij correlaties onder een gekozen drempel worden genegeerd.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k dichtstbijzijnde buren:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Correlaties tussen rijparen bijhouden en de correlaties negeren die niet tot de hoogste &lt;i&gt;k&lt;/i&gt; correlaties van elke rij behoren. Dit is vooral nuttig voor gegevenssets die extreem sterk gecorreleerd zijn.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Minimale correlatiewaarde</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>De minimale correlatiewaarde waarboven een verbinding in de graaf wordt gemaakt. Een lagere minimumwaarde verhoogt de reken- en geheugenvereisten.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>Maximale k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Maximale correlatie-&lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Het maximale aantal correlaties dat per gegevensrij wordt bewaard, waardoor uiteindelijk het aantal verbindingen in de graaf wordt beperkt. Een hogere maximumwaarde verhoogt de reken- en geheugenvereisten.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Initieel:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>Initiële k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Correlatiedrempel</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Correlatie-&lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>De initiële correlatiedrempel bepaalt de grootte van de resulterende graaf. Een lagere waarde filtert minder verbindingen en levert een grotere graaf op. Deze waarde kan later worden gewijzigd, nadat de graaf is gemaakt.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>De initiële k-waarde bepaalt de grootte van de resulterende graaf. Een hogere waarde filtert minder verbindingen en levert een grotere graaf op. Deze waarde kan later worden gewijzigd, nadat de graaf is gemaakt.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Bezig…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Lege graaf</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Sjabloon</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Sjabloon&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Een sjabloon is een benoemde set transformaties en visualisaties die na het maken op de graaf kan worden toegepast. Selecteer een sjabloon om toe te passen. &lt;b&gt;Opmerking:&lt;/b&gt; transformaties die hier worden toegepast, worden altijd onvoorwaardelijk toegevoegd, ongeacht de toepassingsmethode van het sjabloon zelf.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Naam:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Als standaard instellen</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Samenvatting</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Samenvatting&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Er wordt een graaf gemaakt met de volgende parameters:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Getransponeerd&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Dataframe: [ Kolom: {0} Rij: {1} Breedte: {2} Hoogte: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Minimale correlatiewaarde: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Initiële correlatiedrempel: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Maximale k-waarde: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Initiële k-waarde: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Continue correlatiemaat: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Correlatiepolariteit: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputatie: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputatie: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Afkappen: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Afkappen: {0} ({1}e percentiel)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Afkappen: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Schalen: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalisatie: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Discrete correlatiemaat: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Sjabloon: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Geschatte grootte van de graaf vóór transformaties: {0}{3} knooppunten{2}, {1}{4} verbindingen{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Het verhogen van de minimale correlatiewaarde verkleint meestal de graaf.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Het verlagen van de maximale &lt;i&gt;k&lt;/i&gt; verkleint meestal de graaf.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}WAARSCHUWING: dit is een zeer grote graaf die de systeembronnen kan uitputten en tot instabiliteit of vastlopen kan leiden. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}WAARSCHUWING: de gegenereerde graaf is waarschijnlijk leeg.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 kolom</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n kolom</numerusform>
            <numerusform>%n kolommen</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…en nog %1</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Vergroten om kolominformatie te tonen</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Vergroten om kolomnamen te tonen</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n unieke waarde</numerusform>
            <numerusform>%n unieke waarden</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Gemiddelde van %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Gemiddelde van selectie</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Mediaan van %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Mediaan van selectie</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Gemiddeld histogram van %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Gemiddeld histogram van selectie</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Afwijking</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 boven</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 onder</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Std.afw.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Std.fout</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 en nog %3…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>De extensie van %1 geeft aan dat het een ‘%2’ is, maar de inhoud kan niet worden bepaald.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>De extensie van %1 geeft aan dat het een ‘%2’ is, maar de inhoud lijkt op een ‘%3’-bestand.</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlatie</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Maakt een graaf waarin knooppunten rijen gegevens voorstellen en verbindingen de correlaties tussen die rijen.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>De gegevenswaarde is een geparametriseerd attribuut waarmee naar een specifieke kolom in de gecorreleerde gegevens kan worden verwezen.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>De naam van de gegevenskolom met de grootste absolute waarde in de gegevensrij die bij het knooppunt hoort.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>De gemiddelde gegevenswaarde is het gemiddelde van de waarden die bij het knooppunt horen.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>De minimale gegevenswaarde is de kleinste waarde die bij het knooppunt hoort.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>De maximale gegevenswaarde is de grootste waarde die bij het knooppunt hoort.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>De %1 is een maat voor de spreiding van de waarden die bij het knooppunt horen. Deze is gedefinieerd als ∑(&lt;i&gt;x&lt;/i&gt;-µ)², waarbij &lt;i&gt;x&lt;/i&gt; de waarde is en µ het gemiddelde.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variantie</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>De %1 is een maat voor de spreiding van de waarden die bij het knooppunt horen. Deze is gedefinieerd als √∑(&lt;i&gt;x&lt;/i&gt;-µ)², waarbij &lt;i&gt;x&lt;/i&gt; de waarde is en µ het gemiddelde.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standaardafwijking</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>De %1 is een maat voor de spreiding van de waarden die bij het knooppunt horen. Deze is gedefinieerd als de standaardafwijking gedeeld door het gemiddelde.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Variatiecoëfficiënt</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>In de plug-ingegevens ontbreekt de sleutel ‘%1’.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Dataframe: [ Kolom: %1 Rij: %2 Breedte: %3 Hoogte: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Getransponeerd</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Continue correlatiemaat: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positief</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Negatief</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Beide</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Correlatiepolariteit: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Discrete correlatiemaat: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Minimale correlatiewaarde: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Maximale k-waarde: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Constante (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Kolomgemiddelde</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Rij-interpolatie</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputatie: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Afkappen: constante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Afkappen: winsorisatie (%1e percentiel)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Schalen: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Schalen: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Schalen: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Schalen: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Schalen: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalisatie: min./max.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalisatie: kwantiel</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalisatie: gemiddelde</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalisatie: standaardisatie</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalisatie: eenheidsschaling</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalisatie: softmaxschaling</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Kolomannotaties importeren</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Geen geschikte sleutels</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Laden van {0} mislukt.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} laden…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Inleiding</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Annotaties importeren&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Annotatiegegevens kunnen uit een externe bron worden geïmporteerd en over de grafiek worden gelegd. Elk van de ondersteunde tabelgegevensindelingen kan worden gebruikt. Na het importeren kunnen nieuwe annotaties op de gebruikelijke manier worden gesorteerd en gegroepeerd.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Sleutelselectie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Sleutelselectie&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Om annotatiegegevens te importeren, moet de rij in de invoergegevens worden bepaald die overeenkomt met de kolomnamen van de bestaande gegevens. Hiermee worden kolommen in de tabel gekoppeld aan kolommen in de grafiek. Selecteer hieronder de juiste rij. U kunt ook op &lt;i&gt;{0}&lt;/i&gt; klikken, dan wordt de beste overeenkomst automatisch bepaald.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Automatisch detecteren</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>De geselecteerde rij &lt;b&gt;{0}&lt;/b&gt; bevat dubbele waarden, sommige kolommen worden genegeerd!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>{0}% overeenkomst gevonden</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Geen overeenkomst gevonden</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Annotatieselectie</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Annotatieselectie&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Selecteer de gewenste annotaties in de lijst rechts. U kunt meerdere annotaties selecteren door &lt;i&gt;Ctrl&lt;/i&gt; ingedrukt te houden, of een bereik door &lt;i&gt;Shift&lt;/i&gt; ingedrukt te houden en te klikken waar nodig.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Bestaande annotaties vervangen</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Alles selecteren</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Niets selecteren</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Samenvatting</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Samenvatting&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Sleutel detecteren…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>De volgende annotaties worden geïmporteerd met rij &lt;b&gt;{0}{1}&lt;/b&gt; als sleutel:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(bevat dubbele waarden)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Annotaties kunnen pas worden geïmporteerd als er een sleutel is geselecteerd.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Sleutel detecteren</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Interface &amp;naast elkaar weergeven</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>&amp;Kolomnamen tonen</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Interessante rijen zoeken</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Zichtbare kolomannotaties selecteren</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Rasterlijnen</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Nul opnemen in Y-as</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Uitschieters tonen</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Zoom herstellen</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Alle kolommen tonen</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Geen</translation>
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
        <translation>Centreren op gemiddelde</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Schalen naar eenheidsvariantie</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Pareto-schaling</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Foutbalken</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Gebied</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standaardafwijking</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Standaardfout</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Afzonderlijke lijn</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Gemiddeldelijn</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Mediaanlijn</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Gemiddeld histogram</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Interkwartielafstand</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Groeperen op annotatie</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Annotaties uit tabel importeren…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-correlatiegrafiek</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Opslaan als &amp;afbeelding…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Label X-as instellen…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Label Y-as instellen…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Label X-as</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Voer het label van de X-as in:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Label Y-as</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Voer het label van de Y-as in:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Grafiek</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Aslabels</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Schalen</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Op attribuut</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Middeling</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Alle</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Spreiding</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Kolommen sorteren op</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Groepen kleuren op</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Opslaan als afbeeldingen per…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Rij</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Natuurlijke volgorde</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Kolomnaam</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Gegevenswaarde</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Hiërarchische clustering</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Klik op een kolom of kolomannotatie om deze te selecteren. Houd &lt;i&gt;Ctrl&lt;/i&gt; ingedrukt en klik om meerdere kolommen te selecteren.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Sorteervolgorde op gegevenswaarde vastzetten</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Grafiek opslaan als afbeelding</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>PNG-afbeelding (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>JPEG-afbeelding (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>PDF-document (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Alle annotaties tonen</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Alle annotaties verbergen</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Relatief</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Absoluut</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Selectieweging</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Rijen worden geselecteerd door de som van de geselecteerde kolommen af te wegen tegen de niet-geselecteerde kolommen. Weging voor de geselecteerde kolommen (aangeduid als &lt;i&gt;Absoluut&lt;/i&gt;) vindt doorgaans rijen met pieken in die kolommen, ongeacht de niet-geselecteerde kolommen. Daarentegen vindt (negatieve) weging voor de niet-geselecteerde kolommen (aangeduid als &lt;i&gt;Relatief&lt;/i&gt;) doorgaans rijen met significante waarden juist in de geselecteerde kolommen.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}e</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}e</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}e</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}e</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Percentiel</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Het aantal rijen om te selecteren en te rangschikken.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Opnieuw selecteren</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Interkwartielafstand van %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Interkwartielafstand</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Grafiekafbeelding(en) opslaan</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>De correlatiecoëfficiënt van Pearson geeft de lineaire correlatie tussen twee variabelen aan.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Pearson-correlatiewaarde</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>De %1 geeft het lineaire verband tussen twee variabelen aan.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Correlatiecoëfficiënt van Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman-rang</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>De rangcorrelatiecoëfficiënt van Spearman is een niet-parametrische maat voor de statistische afhankelijkheid tussen de rangschikkingen van twee variabelen. Hij geeft aan hoe goed het verband tussen twee variabelen kan worden beschreven met een monotone functie.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Spearman-rangcorrelatiewaarde</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>De %1 geeft het monotone verband tussen twee variabelen aan.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Rangcorrelatiecoëfficiënt van Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Euclidische gelijkenis</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Euclidische gelijkenis is in wezen het omgekeerde van de euclidische afstand tussen twee vectoren.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1 is in wezen het omgekeerde van de euclidische afstand tussen twee vectoren.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Cosinusgelijkenis</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Cosinusgelijkenis is een maat voor de gelijkenis tussen twee vectoren ongelijk aan nul in een inproductruimte. Ze is gedefinieerd als de cosinus van de hoek ertussen, wat ook gelijk is aan het inproduct van dezelfde vectoren, beide genormaliseerd tot lengte 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1 is een maat voor de gelijkenis tussen twee vectoren ongelijk aan nul in een inproductruimte.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Biweight midcorrelation (ook Bicor genoemd) is een maat voor de gelijkenis tussen steekproeven. Ze is gebaseerd op de mediaan in plaats van het gemiddelde en daardoor minder gevoelig voor uitschieters.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Bicor-correlatiewaarde</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 is een op de mediaan gebaseerde maat voor de gelijkenis tussen steekproeven.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>De Jaccard-index is een statistiek om de gelijkenis en diversiteit van steekproefverzamelingen te bepalen.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Jaccard-correlatiewaarde</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>De %1 is een statistiek om de gelijkenis en diversiteit van steekproefverzamelingen te bepalen.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Jaccard-index</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Eenvoudige overeenkomstcoëfficiënt</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>De eenvoudige overeenkomstcoëfficiënt is een statistiek om de gelijkenis en diversiteit van steekproefverzamelingen te bepalen. Hij is gelijk aan Jaccard, behalve dat hij ook wederzijdse afwezigheid meetelt.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Eenvoudige overeenkomstcoëfficiënt (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Gegevenswaarde</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Sterkste gegevenskolom</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Gemiddelde gegevenswaarde</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Minimale gegevenswaarde</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Maximale gegevenswaarde</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variantie</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standaardafwijking</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Variatiecoëfficiënt</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>Absolute %1</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Gegevens</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Correlatiewaarden</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Correlatie-CSV-bestand</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Correlatie-CSV-bestanden</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Correlatie-TSV-bestand</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Correlatie-TSV-bestanden</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Correlatie-SSV-bestand</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Correlatie-SSV-bestanden</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Correlatie-Excel-bestand</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Correlatie-Excel-bestanden</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Correleren</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Clustering</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Kolomannotaties importeren</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Kolomannotatie importeren</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Kolomannotaties importeren</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Kolomannotatie importeren</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n annotatie geïmporteerd</numerusform>
            <numerusform>%n annotaties geïmporteerd</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Annotatie %1 geïmporteerd</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributen</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalisatie</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlatie</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Graaf opbouwen</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Kolommen sorteren</translation>
    </message>
</context>
</TS>
