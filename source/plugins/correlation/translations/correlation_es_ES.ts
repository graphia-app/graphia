<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es_ES" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>No se ha podido cargar {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introducción</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Análisis de grafos de correlación&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>El complemento de correlación crea grafos basados en la similitud entre variables de un conjunto de datos.&lt;br&gt;&lt;br&gt;Un {0} proporciona esta medida de similitud. Si se especifica, los datos de entrada pueden escalarse y normalizarse; después, las puntuaciones de correlación se usan para decidir si se crea o no una arista entre los nodos que representan filas de datos.&lt;br&gt;&lt;br&gt;Las aristas se pueden filtrar mediante transformaciones una vez creado el grafo.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>coeficiente de correlación</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Selección de datos</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selección de datos - Seleccionar y ajustar&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Se ha seleccionado automáticamente un marco de datos para su conjunto de datos. Si desea ajustarlo, seleccione a continuación la nueva celda inicial. Tenga en cuenta que el marco de datos siempre se extiende hasta la celda inferior derecha del conjunto de datos.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Trasponer</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>La trasposición voltea los datos sobre su diagonal, convirtiendo las filas en columnas y viceversa.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Tipo de datos:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Continuos</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discretos</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Tipo de datos</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Continuos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpretar los datos seleccionados como filas de datos continuos. Se usará una medida de correlación numérica entre filas.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discretos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpretar los datos seleccionados como filas de valores discretos. Los valores pueden ser numéricos, pero lo normal es que sean textuales. Se usará una medida de correlación discreta entre filas.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Tratar como binario</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Si esta opción está activada, a efectos del algoritmo de correlación las celdas vacías o que contienen los valores &lt;i&gt;0&lt;/i&gt; o &lt;i&gt;false&lt;/i&gt; se consideran falsas, mientras que todos los demás valores se consideran verdaderos.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;ADVERTENCIA: el marco de datos seleccionado parece contener datos continuos; interpretarlos como discretos puede dar como resultado solo un nivel bajo de correlación.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlación</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlación - Ajustar parámetros&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Avanzado</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>En el eje horizontal del gráfico siguiente, los valores de 1 representan filas perfectamente correlacionadas, mientras que 0 indica que no hay correlación. Todos los valores absolutos por debajo del umbral mínimo de correlación seleccionado se descartan y no se usarán para crear aristas. De forma predeterminada, se añade una transformación que elimina las aristas de todos los valores absolutos por debajo del umbral de correlación inicial especificado.&lt;br&gt;&lt;br&gt;Para tener más control sobre los parámetros usados, marque la opción &lt;b&gt;Avanzado&lt;/b&gt; arriba a la derecha.</translation>
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
        <translation>Algoritmo de correlación</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rango de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Similitud euclidiana</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Similitud del coseno</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polaridad:</translation>
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
        <translation>Ambas</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Polaridad</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>De forma predeterminada, al crear el grafo solo se tienen en cuenta las correlaciones positivas. Casi siempre es la configuración correcta, pero para algunas fuentes de datos puede tener más sentido tener en cuenta la polaridad de la correlación. Tenga en cuenta que no todos los algoritmos de correlación pueden encontrar relaciones negativas.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputación:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Constante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolación por fila</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Media de la columna</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputación</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Constante:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Sustituir todos los valores que faltan por una constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Interpolación por fila:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Interpolar linealmente los valores que faltan usando los valores circundantes más cercanos de la fila y sus posiciones relativas. Si solo hay un valor circundante disponible, el valor se igualará a él. Si la fila está vacía, el valor será cero.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Media de la columna:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Sustituir los valores que faltan por el valor medio de su columna, sin contar los valores que faltan.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Recorte:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Ninguno</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorización</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>percentil</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Recorte</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Limitar el valor máximo de la tabla de datos a una constante especificada.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorización:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Limitar el valor máximo de los datos al de un percentil especificado, fila por fila. Por ejemplo, si se elige el percentil 95, el 5 % superior de los valores tomará el valor del percentil 95.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Escalado:</translation>
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
        <translation>Tipos de escalado</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Calcular el logaritmo de &lt;i&gt;x&lt;/i&gt; + ε en base &lt;i&gt;b&lt;/i&gt;, donde &lt;i&gt;x&lt;/i&gt; son los datos de entrada y ε es una constante muy pequeña.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Elevar &lt;i&gt;b&lt;/i&gt; a la potencia &lt;i&gt;x&lt;/i&gt;, donde &lt;i&gt;x&lt;/i&gt; son los datos de entrada.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Calcular el arcoseno de &lt;i&gt;x&lt;/i&gt;, donde &lt;i&gt;x&lt;/i&gt; son los datos de entrada. Es útil cuando se necesita una transformación logarítmica pero el conjunto de datos contiene números negativos o ceros.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalización:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Mín./Máx.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Estandarización</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Escalado unitario</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Cuantil</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Tipos de normalización</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mín./Máx.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Reescalar los datos para que sus valores queden en el intervalo [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Es útil si las columnas del conjunto de datos tienen escalas o unidades distintas. Nota: si todos los elementos de una columna tienen el mismo valor, los valores se reescalarán a 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Media:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Similar al escalado Mín./Máx., salvo que los valores resultantes quedan centrados en la media de su columna: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Nota: si todos los elementos de una columna tienen el mismo valor, los valores se reescalarán a 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Estandarización:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>También conocido como normalización Z-score, este método centra el valor en la media y lo escala por la desviación estándar: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Escalado unitario:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Los valores se dividen por la longitud euclidiana del vector formado por la columna de datos: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Cuantil:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Normalizar los datos para que las columnas tengan distribuciones iguales.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>La función softmax convierte cada columna de datos en una distribución de probabilidad aplicando la exponencial a cada elemento y normalizando mediante la división por la suma. En definitiva, esto hace que los valores más grandes de cada columna se exageren.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filtrado:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Umbral</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Filtrando</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Umbral:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Filtro sencillo que descarta las correlaciones por debajo de un umbral seleccionado.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k vecinos más cercanos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Registrar las correlaciones encontradas entre pares de filas y descartar las que no estén entre las &lt;i&gt;k&lt;/i&gt; mayores correlaciones de cada fila. Es especialmente útil para conjuntos de datos con una correlación extremadamente alta.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Mínimo:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Valor mínimo de correlación</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Valor mínimo de correlación por encima del cual se creará una arista en el grafo. Un valor mínimo más bajo aumenta los requisitos de cálculo y de memoria.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>k máximo:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; máximo de correlación</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Número máximo de correlaciones que se conservan por fila de datos, lo que en definitiva limita el número de aristas que se crearán en el grafo. Un valor máximo mayor aumenta los requisitos de cálculo y de memoria.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Inicial:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>k inicial:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Umbral de correlación</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; de correlación</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>El umbral de correlación inicial determina el tamaño del grafo resultante. Un valor más bajo filtra menos aristas y da como resultado un grafo más grande. Este valor puede cambiarse más adelante, una vez creado el grafo.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>El valor k inicial determina el tamaño del grafo resultante. Un valor más alto filtra menos aristas y da como resultado un grafo más grande. Este valor puede cambiarse más adelante, una vez creado el grafo.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Procesando…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Grafo vacío</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Plantilla</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Plantilla&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Una plantilla es un conjunto con nombre de transformaciones y visualizaciones que se puede aplicar al grafo una vez creado. Seleccione la plantilla que desea aplicar. &lt;b&gt;Nota:&lt;/b&gt; las transformaciones aplicadas aquí siempre se añadirán incondicionalmente, independientemente del método de aplicación específico de la plantilla.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Establecer como predeterminado</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Resumen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Se creará un grafo con los siguientes parámetros:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Traspuesto&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Marco de datos: [ Columna: {0} Fila: {1} Ancho: {2} Alto: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Valor mínimo de correlación: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Umbral de correlación inicial: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Valor k máximo: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Valor k inicial: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Medida de correlación continua: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Polaridad de la correlación: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputación: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputación: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Recorte: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Recorte: {0} (percentil {1})</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Recorte: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Escalado: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalización: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Medida de correlación discreta: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Plantilla: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamaño estimado del grafo antes de las transformaciones: {0}{3} nodos{2}, {1}{4} aristas{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentar el valor mínimo de correlación suele reducir el tamaño del grafo.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Reducir el &lt;i&gt;k&lt;/i&gt; máximo suele reducir el tamaño del grafo.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ADVERTENCIA: este grafo es muy grande y podría agotar los recursos del sistema y provocar inestabilidad o bloqueos. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}ADVERTENCIA: es probable que el grafo generado esté vacío.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 columna</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n columna</numerusform>
            <numerusform>%n columnas</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…y %1 más</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Cambiar el tamaño para ver la información de las columnas</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Cambiar el tamaño para ver los nombres de las columnas</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n valor único</numerusform>
            <numerusform>%n valores únicos</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Media de %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Media de la selección</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Mediana de %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Mediana de la selección</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Histograma medio de %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Histograma medio de la selección</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Desviación</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 superior</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 inferior</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Desv. est.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Err. est.</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 y %3 más…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>La extensión de %1 indica que es un «%2», pero no se ha podido determinar su contenido.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>La extensión de %1 indica que es un «%2», pero su contenido se parece a un archivo «%3».</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlación</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Crea un grafo en el que los nodos representan filas de datos y las aristas representan las correlaciones entre dichas filas.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>El valor de datos es un atributo con parámetro que permite hacer referencia a una columna concreta de los datos correlacionados.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Nombre de la columna de datos con el mayor valor absoluto en la fila de datos asociada al nodo.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>El valor medio de datos es la media de los valores asociados al nodo.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>El valor mínimo de datos es el menor valor asociado al nodo.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>El valor máximo de datos es el mayor valor asociado al nodo.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>La %1 es una medida de la dispersión de los valores asociados al nodo. Se define como ∑(&lt;i&gt;x&lt;/i&gt;-µ)², donde &lt;i&gt;x&lt;/i&gt; es el valor y µ es la media.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianza</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>La %1 es una medida de la dispersión de los valores asociados al nodo. Se define como √∑(&lt;i&gt;x&lt;/i&gt;-µ)², donde &lt;i&gt;x&lt;/i&gt; es el valor y µ es la media.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>El %1 es una medida de la dispersión de los valores asociados al nodo. Se define como la desviación estándar dividida por la media.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coeficiente de variación</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>Falta la clave «%1» en los datos del complemento.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Marco de datos: [ Columna: %1 Fila: %2 Ancho: %3 Alto: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Traspuesto</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Medida de correlación continua: %1</translation>
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
        <translation>Ambas</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Polaridad de la correlación: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Medida de correlación discreta: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Valor mínimo de correlación: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Valor k máximo: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Constante (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Media de la columna</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolación por fila</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputación: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Recorte: constante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Recorte: winsorización (percentil %1)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Escalado: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Escalado: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Escalado: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Escalado: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Escalado: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalización: Mín./Máx.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalización: cuantil</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalización: media</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalización: estandarización</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalización: escalado unitario</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalización: escalado softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importar anotaciones de columnas</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>No hay claves adecuadas</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>No se ha podido cargar {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introducción</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Importar anotaciones&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Los datos de anotación pueden importarse desde una fuente externa y superponerse al gráfico. Se puede usar cualquiera de los formatos de datos tabulares admitidos. Tras importarlas, las nuevas anotaciones pueden ordenarse y agruparse de la forma habitual.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Selección de claves</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selección de claves&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Para importar datos de anotación es necesario identificar la fila de los datos de entrada que corresponde a los nombres de columna de los datos existentes. Sirve para hacer corresponder las columnas de la tabla con las del gráfico. Seleccione a continuación la fila correcta. También puede hacer clic en &lt;i&gt;{0}&lt;/i&gt; para que se identifique automáticamente la mejor coincidencia.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Detección automática</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>La fila seleccionada &lt;b&gt;{0}&lt;/b&gt; tiene valores duplicados; se ignorarán algunas columnas.</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Coincidencia del {0}% encontrada</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>No se ha encontrado ninguna coincidencia</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Selección de anotaciones</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Selección de anotaciones&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Seleccione las anotaciones necesarias en la lista de la derecha. Puede seleccionar varias manteniendo pulsada la tecla &lt;i&gt;Ctrl&lt;/i&gt;, o un intervalo manteniendo pulsada &lt;i&gt;Mayús&lt;/i&gt; y haciendo clic donde desee.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Reemplazar anotaciones existentes</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Seleccionar todo</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>No seleccionar nada</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Resumen</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Resumen&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Detectando la clave…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Las siguientes anotaciones se importarán usando la fila &lt;b&gt;{0}{1}&lt;/b&gt; como clave:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(tiene valores duplicados)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;No se pueden importar anotaciones si no se selecciona una clave.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Detectando la clave</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Mostrar interfaz &amp;lado a lado</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Mostrar nombres de &amp;columnas</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Buscar filas de interés</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Seleccionar anotaciones de columnas visibles</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Líneas de cuadrícula</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Leyenda</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Incluir el cero en el eje Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Mostrar valores atípicos</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Restablecer zoom</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Mostrar todas las columnas</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Ninguno</translation>
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
        <translation>Centrado en la media</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Escalado a varianza unitaria</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Escalado de Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Barras de error</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Error estándar</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Línea individual</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Línea de la media</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Línea de la mediana</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Histograma medio</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Rango intercuartílico</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Agrupar por anotación</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Importar anotaciones desde tabla…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-grafico-correlacion</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Guardar como &amp;imagen…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Establecer etiqueta del eje X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Establecer etiqueta del eje Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Etiqueta del eje X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Introduzca la etiqueta del eje X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Etiqueta del eje Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Introduzca la etiqueta del eje Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Gráfico</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Etiquetas de los ejes</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Escalado</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Por atributo</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Promedio</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Dispersión</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Ordenar columnas por</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Colorear grupos por</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Guardar como imágenes por…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Fila</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Orden natural</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Nombre de columna</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valor de datos</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Agrupamiento jerárquico</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Haga clic en una columna o en una anotación de columna para seleccionarla. Mantenga pulsada &lt;i&gt;Ctrl&lt;/i&gt; y haga clic para seleccionar varias columnas.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Fijar el orden por valor de datos</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Guardar gráfico como imagen</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Imagen PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Imagen JPEG (*.jpg *.jpeg)</translation>
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
        <translation>Mostrar todas las anotaciones</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Ocultar todas las anotaciones</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Relativa</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Absoluta</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Ponderación de la selección</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Las filas se seleccionan ponderando la suma de las columnas seleccionadas frente a las no seleccionadas. Ponderar las columnas seleccionadas (indicado como &lt;i&gt;Absoluta&lt;/i&gt;) tiende a encontrar filas con picos en esas columnas, independientemente de las no seleccionadas. En cambio, ponderar (negativamente) las columnas no seleccionadas (indicado como &lt;i&gt;Relativa&lt;/i&gt;) tiende a encontrar filas con valores significativos específicamente en las columnas seleccionadas.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}.º</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}.º</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}.º</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}.º</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Percentil</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>La cantidad de filas que se seleccionan y clasifican.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Volver a seleccionar</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Rango intercuartílico de %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Rango intercuartílico</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Guardando imágenes del gráfico</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>El coeficiente de correlación de Pearson indica la correlación lineal entre dos variables.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Valor de correlación de Pearson</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>El %1 indica la relación lineal entre dos variables.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Coeficiente de correlación de Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Rango de Spearman</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>El coeficiente de correlación de rangos de Spearman es una medida no paramétrica de la dependencia estadística entre los rangos de dos variables. Evalúa hasta qué punto la relación entre dos variables puede describirse mediante una función monótona.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Valor de correlación de rangos de Spearman</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>El %1 indica la relación monótona entre dos variables.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Coeficiente de correlación de rangos de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Similitud euclidiana</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La similitud euclidiana es esencialmente la inversa de la distancia euclidiana entre dos vectores.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>La %1 es esencialmente la inversa de la distancia euclidiana entre dos vectores.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Similitud del coseno</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>La similitud del coseno es una medida de similitud entre dos vectores no nulos de un espacio con producto interior. Se define como el coseno del ángulo que forman, lo que equivale también al producto interior de esos mismos vectores normalizados a longitud 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>La %1 es una medida de similitud entre dos vectores no nulos de un espacio con producto interior.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>La Biweight Midcorrelation (también llamada Bicor) es una medida de similitud entre muestras. Se basa en la mediana en lugar de en la media, por lo que es menos sensible a los valores atípicos.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Valor de correlación Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 es una medida de similitud entre muestras basada en la mediana.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>El índice de Jaccard es un estadístico que se usa para medir la similitud y la diversidad de conjuntos de muestras.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Valor de correlación de Jaccard</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>El %1 es un estadístico que se usa para medir la similitud y la diversidad de conjuntos de muestras.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Índice de Jaccard</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Coeficiente de coincidencia simple</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>El coeficiente de coincidencia simple es un estadístico que se usa para medir la similitud y la diversidad de conjuntos de muestras. Es idéntico al de Jaccard, salvo que también cuenta las ausencias mutuas.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Coeficiente de coincidencia simple (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valor de datos</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Columna de datos más fuerte</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Valor medio de datos</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Valor mínimo de datos</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Valor máximo de datos</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Varianza</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desviación estándar</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coeficiente de variación</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 absoluta</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Datos</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Valores de correlación</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Archivo CSV de correlación</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Archivos CSV de correlación</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Archivo TSV de correlación</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Archivos TSV de correlación</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Archivo SSV de correlación</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Archivos SSV de correlación</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Archivo Excel de correlación</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Archivos Excel de correlación</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Calculando la correlación</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Agrupamiento</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importar anotaciones de columnas</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Importar anotación de columna</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Importando anotaciones de columnas</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Importando anotación de columna</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n anotación importada</numerusform>
            <numerusform>%n anotaciones importadas</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Anotación %1 importada</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalización</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlación</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Construyendo el grafo</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Ordenando columnas</translation>
    </message>
</context>
</TS>
