<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr_FR" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Échec du chargement de {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Chargement de {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fermer</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuler</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Introduction</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Analyse de graphe par corrélation&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>L’extension de corrélation crée des graphes à partir de la similarité entre les variables d’un jeu de données.&lt;br&gt;&lt;br&gt;Un {0} fournit cette mesure de similarité. Si demandé, les données d’entrée peuvent être mises à l’échelle et normalisées ; les scores de corrélation servent ensuite à décider si une arête est créée entre les nœuds représentant les lignes de données.&lt;br&gt;&lt;br&gt;Les arêtes peuvent être filtrées à l’aide de transformations une fois le graphe créé.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>coefficient de corrélation</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Sélection des données</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Sélection des données - Sélectionner et ajuster&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Un cadre de données a été automatiquement sélectionné pour votre jeu de données. Pour l’ajuster, sélectionnez ci-dessous la nouvelle cellule de départ. Notez que le cadre de données s’étend toujours jusqu’à la cellule située tout en bas à droite du jeu de données.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Transposer</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>La transposition retourne les données selon leur diagonale : les lignes deviennent des colonnes et inversement.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Type de données :</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Continues</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discrètes</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Type de données</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Continues :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpréter les données sélectionnées comme des lignes de données continues. Une mesure de corrélation numérique sera utilisée entre les lignes.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discrètes :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpréter les données sélectionnées comme des lignes de valeurs discrètes. Les valeurs peuvent être numériques, mais elles sont normalement textuelles. Une mesure de corrélation discrète sera utilisée entre les lignes.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Traiter comme binaire</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Si cette option est activée, l’algorithme de corrélation considère comme faux les cellules vides et celles contenant les valeurs &lt;i&gt;0&lt;/i&gt; ou &lt;i&gt;false&lt;/i&gt;, et comme vraies toutes les autres valeurs.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;AVERTISSEMENT : le cadre de données sélectionné semble contenir des données continues ; les interpréter comme discrètes risque de n’aboutir qu’à un faible niveau de corrélation.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Corrélation</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Corrélation - Ajuster les paramètres&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Avancé</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Sur l’axe horizontal du graphique ci-dessous, une valeur de 1 correspond à des lignes parfaitement corrélées, tandis que 0 indique l’absence de corrélation. Toutes les valeurs absolues inférieures au seuil minimal de corrélation sélectionné sont écartées et ne serviront pas à créer des arêtes. Par défaut, une transformation est ajoutée pour supprimer les arêtes de toutes les valeurs absolues inférieures au seuil de corrélation initial spécifié.&lt;br&gt;&lt;br&gt;Pour mieux contrôler les paramètres utilisés, cochez l’option &lt;b&gt;Avancé&lt;/b&gt; en haut à droite.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Algorithme :</translation>
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
        <translation>Algorithme de corrélation</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rang de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Similarité euclidienne</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Similarité cosinus</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polarité :</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positive</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Négative</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Les deux</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Polarité</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Par défaut, seules les corrélations positives sont prises en compte lors de la création du graphe. C’est presque toujours le bon réglage, mais pour certaines sources de données, il peut être plus judicieux de tenir compte de la polarité de la corrélation. Notez que tous les algorithmes de corrélation ne sont pas capables de détecter des relations négatives.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputation :</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Constante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolation par ligne</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Moyenne de colonne</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputation</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Constante :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Remplacer toutes les valeurs manquantes par une constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Interpolation par ligne :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Interpoler linéairement les valeurs manquantes à partir des valeurs voisines les plus proches dans la ligne et de leurs positions relatives. Si une seule valeur voisine est disponible, la valeur prendra la même. La valeur sera mise à zéro si la ligne est vide.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Moyenne de colonne :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Remplacer les valeurs manquantes par la moyenne de leur colonne, valeurs manquantes exclues.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Écrêtage :</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Aucun</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorisation</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>centile</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Écrêtage</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Limiter la valeur maximale du tableau de données à une constante spécifiée.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorisation :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Limiter la valeur maximale des données à celle d’un centile spécifié, ligne par ligne. Par exemple, si le 95e centile est choisi, les 5 % de valeurs les plus hautes prendront la valeur du 95e centile.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Mise à l’échelle :</translation>
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
        <translation>Types de mise à l’échelle</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε) :</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Calculer le logarithme de &lt;i&gt;x&lt;/i&gt; + ε en base &lt;i&gt;b&lt;/i&gt;, où &lt;i&gt;x&lt;/i&gt; est la donnée d’entrée et ε une très petite constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;) :</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Élever &lt;i&gt;b&lt;/i&gt; à la puissance &lt;i&gt;x&lt;/i&gt;, où &lt;i&gt;x&lt;/i&gt; est la donnée d’entrée.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;) :</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Calculer l’arc sinus de &lt;i&gt;x&lt;/i&gt;, où &lt;i&gt;x&lt;/i&gt; est la donnée d’entrée. Utile lorsqu’une transformation logarithmique est nécessaire mais que le jeu de données contient des nombres négatifs ou des zéros.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalisation :</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Min/Max</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Moyenne</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Standardisation</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Mise à l’échelle unitaire</translation>
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
        <translation>Types de normalisation</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min/Max :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Remettre les données à l’échelle pour que leurs valeurs se situent dans l’intervalle [0, 1] : (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Utile si les colonnes du jeu de données ont des échelles ou des unités différentes. Remarque : si tous les éléments d’une colonne ont la même valeur, celles-ci seront ramenées à 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Moyenne :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Semblable à la mise à l’échelle Min/Max, mais les valeurs obtenues sont centrées sur la moyenne de leur colonne : (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Remarque : si tous les éléments d’une colonne ont la même valeur, celles-ci seront ramenées à 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standardisation :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Également appelée normalisation par score Z, cette méthode centre la valeur sur la moyenne et la divise par l’écart type : (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mise à l’échelle unitaire :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Les valeurs sont divisées par la longueur euclidienne du vecteur formé par la colonne de données : &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Quantile :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Normaliser les données pour que les colonnes aient des distributions identiques.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>La fonction softmax convertit chaque colonne de données en distribution de probabilité en prenant l’exponentielle de chaque élément, puis en normalisant par division par la somme. Au final, les plus grandes valeurs de chaque colonne sont accentuées.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filtrage :</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Seuil</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filtrage</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Seuil :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Filtre simple qui écarte les corrélations inférieures à un seuil sélectionné.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k plus proches voisins :&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Suivre les corrélations trouvées entre paires de lignes et écarter celles qui ne figurent pas parmi les &lt;i&gt;k&lt;/i&gt; plus fortes corrélations de chaque ligne. Particulièrement utile pour les jeux de données extrêmement corrélés.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Minimum :</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Valeur de corrélation minimale</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Valeur de corrélation minimale au-dessus de laquelle une arête est créée dans le graphe. Une valeur minimale plus basse augmente les besoins en calcul et en mémoire.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>k maximal :</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; de corrélation maximal</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Nombre maximal de corrélations à conserver par ligne de données, ce qui limite au final le nombre d’arêtes créées dans le graphe. Une valeur maximale plus élevée augmente les besoins en calcul et en mémoire.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Initial :</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>k initial :</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Seuil de corrélation</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; de corrélation</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Le seuil de corrélation initial détermine la taille du graphe obtenu. Une valeur plus basse filtre moins d’arêtes et donne un graphe plus grand. Cette valeur peut être modifiée ultérieurement, une fois le graphe créé.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>La valeur k initiale détermine la taille du graphe obtenu. Une valeur plus élevée filtre moins d’arêtes et donne un graphe plus grand. Cette valeur peut être modifiée ultérieurement, une fois le graphe créé.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Traitement…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Graphe vide</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Modèle</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Modèle&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Un modèle est un ensemble nommé de transformations et de visualisations qui peut être appliqué au graphe après sa création. Veuillez sélectionner un modèle à appliquer. &lt;b&gt;Remarque :&lt;/b&gt; les transformations appliquées ici seront toujours ajoutées sans condition, quelle que soit la méthode d’application propre au modèle.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nom :</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Définir par défaut</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Récapitulatif</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Récapitulatif&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Un graphe sera créé avec les paramètres suivants :</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Transposé&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Cadre de données : [ Colonne : {0} Ligne : {1} Largeur : {2} Hauteur : {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Valeur de corrélation minimale : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Seuil de corrélation initial : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Valeur k maximale : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Valeur k initiale : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Mesure de corrélation continue : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Polarité de la corrélation : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputation : {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputation : {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Écrêtage : {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Écrêtage : {0} ({1}e centile)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Écrêtage : {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Mise à l’échelle : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalisation : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Mesure de corrélation discrète : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Modèle : {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Taille estimée du graphe avant transformations : {0}{3} nœuds{2}, {1}{4} arêtes{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Augmenter la valeur de corrélation minimale réduit généralement la taille du graphe.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Diminuer le &lt;i&gt;k&lt;/i&gt; maximal réduit généralement la taille du graphe.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}AVERTISSEMENT : ce graphe est très grand et risque d’épuiser les ressources du système, ce qui peut entraîner une instabilité ou des blocages. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}AVERTISSEMENT : le graphe généré sera probablement vide.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 colonne</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n colonne</numerusform>
            <numerusform>%n colonnes</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2 : %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…et %1 de plus</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Agrandir pour afficher les informations des colonnes</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Agrandir pour afficher les noms des colonnes</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n valeur unique</numerusform>
            <numerusform>%n valeurs uniques</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Moyenne de %1 : %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Moyenne de la sélection</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Médiane de %1 : %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Médiane de la sélection</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Histogramme moyen de %1 : %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Histogramme moyen de la sélection</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Écart</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 haut</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 bas</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Écart type</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Erreur type</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 et %3 de plus…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>L’extension de %1 indique qu’il s’agit d’un « %2 », mais son contenu n’a pas pu être déterminé.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>L’extension de %1 indique qu’il s’agit d’un « %2 », mais son contenu ressemble à un fichier « %3 ».</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Corrélation</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Crée un graphe dont les nœuds représentent des lignes de données et les arêtes représentent les corrélations entre ces lignes.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>La valeur de donnée est un attribut paramétré qui permet de faire référence à une colonne précise des données corrélées.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Nom de la colonne de données ayant la plus grande valeur absolue pour la ligne de données associée au nœud.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>La valeur de donnée moyenne est la moyenne des valeurs associées au nœud.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>La valeur de donnée minimale est la plus petite valeur associée au nœud.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>La valeur de donnée maximale est la plus grande valeur associée au nœud.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>La %1 mesure la dispersion des valeurs associées au nœud. Elle est définie par ∑(&lt;i&gt;x&lt;/i&gt;-µ)², où &lt;i&gt;x&lt;/i&gt; est la valeur et µ la moyenne.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variance</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>L’%1 mesure la dispersion des valeurs associées au nœud. Il est défini par √∑(&lt;i&gt;x&lt;/i&gt;-µ)², où &lt;i&gt;x&lt;/i&gt; est la valeur et µ la moyenne.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Écart type</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>Le %1 mesure la dispersion des valeurs associées au nœud. Il est défini comme l’écart type divisé par la moyenne.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coefficient de variation</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>La clé « %1 » est absente des données de l’extension.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Cadre de données : [ Colonne : %1 Ligne : %2 Largeur : %3 Hauteur : %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Transposé</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Mesure de corrélation continue : %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Positive</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Négative</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Les deux</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Polarité de la corrélation : %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Mesure de corrélation discrète : %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Valeur de corrélation minimale : %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Valeur k maximale : %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Constante (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Moyenne de colonne</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolation par ligne</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputation : %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Écrêtage : constante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Écrêtage : winsorisation (%1e centile)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Mise à l’échelle : Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Mise à l’échelle : Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Mise à l’échelle : AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Mise à l’échelle : AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Mise à l’échelle : Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalisation : Min/Max</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalisation : quantile</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalisation : moyenne</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalisation : standardisation</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalisation : mise à l’échelle unitaire</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalisation : mise à l’échelle softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importer des annotations de colonnes</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Aucune clé appropriée</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Échec du chargement de {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Chargement de {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Fermer</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuler</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Introduction</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Importer des annotations&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Des données d’annotation peuvent être importées depuis une source externe et superposées au graphique. Tous les formats de données tabulaires pris en charge peuvent être utilisés. Une fois importées, les nouvelles annotations peuvent être triées et regroupées de la manière habituelle.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Sélection des clés</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Sélection des clés&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Pour importer des données d’annotation, il faut identifier la ligne des données d’entrée qui correspond aux noms des colonnes des données existantes. Elle sert à mettre en correspondance les colonnes du tableau avec celles du graphique. Veuillez sélectionner la bonne ligne ci-dessous. Vous pouvez aussi cliquer sur &lt;i&gt;{0}&lt;/i&gt; pour que la meilleure correspondance soit identifiée automatiquement.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Détection automatique</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>La ligne sélectionnée &lt;b&gt;{0}&lt;/b&gt; contient des valeurs en double, certaines colonnes seront ignorées !</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Correspondance de {0} % trouvée</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Aucune correspondance trouvée</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Sélection des annotations</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Sélection des annotations&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Veuillez sélectionner les annotations voulues dans la liste de droite. Pour en sélectionner plusieurs, maintenez &lt;i&gt;Ctrl&lt;/i&gt; enfoncée ; pour sélectionner une plage, maintenez &lt;i&gt;Maj&lt;/i&gt; enfoncée, puis cliquez selon vos besoins.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Remplacer les annotations existantes</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Tout sélectionner</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Ne rien sélectionner</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Récapitulatif</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Récapitulatif&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Détection de la clé…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Les annotations suivantes seront importées en utilisant la ligne &lt;b&gt;{0}{1}&lt;/b&gt; comme clé :</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(contient des valeurs en double)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Les annotations ne peuvent pas être importées tant qu’aucune clé n’est sélectionnée.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Détection de la clé</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Afficher l’interface &amp;côte à côte</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Afficher les noms des &amp;colonnes</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Rechercher les lignes d’intérêt</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Sélectionner les annotations de colonnes visibles</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Lignes de grille</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Légende</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Inclure zéro dans l’axe Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Afficher les valeurs aberrantes</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Réinitialiser le zoom</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Afficher toutes les colonnes</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Aucun</translation>
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
        <translation>Centrage sur la moyenne</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Mise à l’échelle à variance unitaire</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Mise à l’échelle de Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Barres d’erreur</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Zone</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Écart type</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Erreur type</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Ligne individuelle</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Ligne moyenne</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Ligne médiane</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Histogramme moyen</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Écart interquartile</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Regrouper par annotation</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Importer des annotations depuis un tableau…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-graphique-correlation</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Enregistrer comme &amp;image…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Définir le libellé de l’axe X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Définir le libellé de l’axe Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Libellé de l’axe X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Veuillez saisir le libellé de l’axe X :</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Libellé de l’axe Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Veuillez saisir le libellé de l’axe Y :</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Graphique</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Libellés des axes</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Mise à l’échelle</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Par attribut</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Moyenne</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Tous</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Dispersion</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Trier les colonnes par</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Colorer les groupes par</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Enregistrer comme images par…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Ligne</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Ordre naturel</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Nom de colonne</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valeur de donnée</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Partitionnement hiérarchique</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Cliquez sur une colonne ou une annotation de colonne pour la sélectionner. Maintenez &lt;i&gt;Ctrl&lt;/i&gt; enfoncée et cliquez pour sélectionner plusieurs colonnes.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Figer l’ordre de tri par valeur de donnée</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Enregistrer le graphique comme image</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Image PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Image JPEG (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>Document PDF (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Afficher toutes les annotations</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Masquer toutes les annotations</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Relative</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Absolue</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Pondération de la sélection</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Les lignes sont sélectionnées en pondérant la somme des colonnes sélectionnées par rapport aux colonnes non sélectionnées. Pondérer les colonnes sélectionnées (mode &lt;i&gt;Absolue&lt;/i&gt;) tend à trouver les lignes présentant des pics dans ces colonnes, indépendamment des colonnes non sélectionnées. À l’inverse, pondérer (négativement) les colonnes non sélectionnées (mode &lt;i&gt;Relative&lt;/i&gt;) tend à trouver les lignes ayant des valeurs significatives spécifiquement dans les colonnes sélectionnées.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}er</translation>
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
        <translation>Centile</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Nombre de lignes à sélectionner et à classer.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Resélectionner</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Écart interquartile de %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Écart interquartile</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Enregistrement des images du graphique</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Le coefficient de corrélation de Pearson indique le degré de corrélation linéaire entre deux variables.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Valeur de corrélation de Pearson</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>Le %1 indique le degré de relation linéaire entre deux variables.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Coefficient de corrélation de Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rang de Spearman</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Le coefficient de corrélation de rang de Spearman est une mesure non paramétrique de la dépendance statistique entre les rangs de deux variables. Il évalue dans quelle mesure la relation entre deux variables peut être décrite par une fonction monotone.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Valeur de corrélation de rang de Spearman</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>Le %1 indique le degré de relation monotone entre deux variables.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Coefficient de corrélation de rang de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Similarité euclidienne</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La similarité euclidienne est essentiellement l’inverse de la distance euclidienne entre deux vecteurs.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La %1 est essentiellement l’inverse de la distance euclidienne entre deux vecteurs.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Similarité cosinus</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>La similarité cosinus est une mesure de similarité entre deux vecteurs non nuls d’un espace préhilbertien. Elle est définie comme le cosinus de l’angle qu’ils forment, ce qui équivaut au produit scalaire de ces mêmes vecteurs normalisés à une longueur de 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>La %1 est une mesure de similarité entre deux vecteurs non nuls d’un espace préhilbertien.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>La Biweight Midcorrelation (aussi appelée Bicor) est une mesure de similarité entre échantillons. Elle repose sur la médiane plutôt que sur la moyenne et est donc moins sensible aux valeurs aberrantes.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Valeur de corrélation Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>La %1 est une mesure de similarité entre échantillons fondée sur la médiane.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>L’indice de Jaccard est une statistique servant à évaluer la similarité et la diversité d’ensembles d’échantillons.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Valeur de corrélation de Jaccard</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>L’%1 est une statistique servant à évaluer la similarité et la diversité d’ensembles d’échantillons.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Indice de Jaccard</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Coefficient d’appariement simple</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Le coefficient d’appariement simple est une statistique servant à évaluer la similarité et la diversité d’ensembles d’échantillons. Il est identique à celui de Jaccard, à ceci près qu’il compte aussi les absences communes.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Coefficient d’appariement simple (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valeur de donnée</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Colonne de données la plus forte</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Valeur de donnée moyenne</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Valeur de donnée minimale</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Valeur de donnée maximale</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variance</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Écart type</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coefficient de variation</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 absolue</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Données</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Valeurs de corrélation</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Fichier CSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Fichiers CSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Fichier TSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Fichiers TSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Fichier SSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Fichiers SSV de corrélation</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Fichier Excel de corrélation</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Fichiers Excel de corrélation</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Calcul des corrélations</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Partitionnement</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importer des annotations de colonnes</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Importer une annotation de colonne</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Importation des annotations de colonnes</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Importation de l’annotation de colonne</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n annotation importée</numerusform>
            <numerusform>%n annotations importées</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Annotation %1 importée</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributs</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalisation</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Corrélation</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Construction du graphe</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Tri des colonnes</translation>
    </message>
</context>
</TS>
