<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt_BR" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Falha ao carregar {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introdução</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Análise de grafos de correlação&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>O plugin de correlação cria grafos com base na semelhança entre variáveis de um conjunto de dados.&lt;br&gt;&lt;br&gt;Um {0} fornece esta medida de semelhança. Se especificado, os dados de entrada podem ser escalados e normalizados, após o que as pontuações de correlação são utilizadas para determinar se é criada ou não uma aresta entre os nós que representam linhas de dados.&lt;br&gt;&lt;br&gt;As arestas podem ser filtradas com transformações depois de o grafo ser criado.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>coeficiente de correlação</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Seleção de dados</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Seleção de dados - Selecionar e ajustar&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Foi selecionado automaticamente um dataframe para o seu conjunto de dados. Se desejar ajustá-lo, selecione abaixo a nova célula inicial. Note que o dataframe se estende sempre até a célula mais abaixo e à direita do conjunto de dados.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Transpor</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>A transposição inverte os dados sobre a sua diagonal, passando as linhas a colunas e vice-versa.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Tipo de dados:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Contínuos</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discretos</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Tipo de dados</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Contínuos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpretar os dados selecionados como linhas de dados contínuos. Será utilizada uma medida de correlação numérica entre as linhas.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discretos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpretar os dados selecionados como linhas de valores discretos. Os valores podem ser numéricos, mas normalmente serão textuais. Será utilizada uma medida de correlação discreta entre as linhas.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Tratar como binário</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Se esta opção estiver ativada, para efeitos do algoritmo de correlação, as células vazias ou que contenham os valores &lt;i&gt;0&lt;/i&gt; ou &lt;i&gt;false&lt;/i&gt; são consideradas falsas, e todos os outros valores são considerados verdadeiros.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;AVISO: o dataframe selecionado parece conter dados contínuos; interpretá-los como discretos pode resultar apenas em um baixo nível de correlação.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlação</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlação - Ajustar parâmetros&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Avançado</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>No eixo horizontal do gráfico abaixo, os valores 1 representam linhas perfeitamente correlacionadas, enquanto 0 indica ausência de correlação. Todos os valores absolutos abaixo do limiar mínimo de correlação selecionado são descartados e não serão utilizados para criar arestas. Por padrão, é adicionada uma transformação que remove as arestas de todos os valores absolutos abaixo do limiar de correlação inicial especificado.&lt;br&gt;&lt;br&gt;Para ter mais controle sobre os parâmetros utilizados, marque a opção &lt;b&gt;Avançado&lt;/b&gt; no canto superior direito.</translation>
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
        <translation>Algoritmo de correlação</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Ordem de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Semelhança euclidiana</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Semelhança de cosseno</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Polaridade:</translation>
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
        <translation>Polaridade</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Por padrão, apenas as correlações positivas são consideradas ao criar o grafo. Em quase todos os casos esta é a definição correta, mas para algumas fontes de dados pode fazer mais sentido ter em conta a polaridade da correlação. Note que nem todos os algoritmos de correlação conseguem encontrar relações negativas.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Imputação:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Constante</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolação por linha</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Média da coluna</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Imputação</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Constante:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Substituir todos os valores em falta por uma constante.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Interpolação por linha:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Interpolar linearmente os valores em falta utilizando os valores vizinhos mais próximos na linha e as suas posições relativas. Se só estiver disponível um valor vizinho, o valor será igual a este. O valor será definido como zero se a linha estiver vazia.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Média da coluna:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Substituir os valores em falta pelo valor médio da respectiva coluna, excluindo os valores em falta.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Corte:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nenhum</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorização</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>percentil</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Corte</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Limitar o valor máximo da tabela de dados a uma constante especificada.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorização:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Limitar o valor máximo dos dados ao de um percentil especificado, linha a linha. Por exemplo, se for escolhido o percentil 95, os 5% de valores mais altos ficarão com o valor do percentil 95.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Escala:</translation>
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
        <translation>Tipos de escala</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Calcular o logaritmo de &lt;i&gt;x&lt;/i&gt; + ε na base &lt;i&gt;b&lt;/i&gt;, em que &lt;i&gt;x&lt;/i&gt; são os dados de entrada e ε é uma constante muito pequena.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Elevar &lt;i&gt;b&lt;/i&gt; à potência &lt;i&gt;x&lt;/i&gt;, em que &lt;i&gt;x&lt;/i&gt; são os dados de entrada.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Calcular o arco-seno de &lt;i&gt;x&lt;/i&gt;, em que &lt;i&gt;x&lt;/i&gt; são os dados de entrada. É útil quando é necessária uma transformação logarítmica mas o conjunto de dados contém números negativos ou zeros.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Normalização:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Mín./Máx.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Média</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Padronização</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Escala unitária</translation>
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
        <translation>Tipos de normalização</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mín./Máx.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Reescalar os dados de forma que os seus valores fiquem no intervalo [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). É útil se as colunas do conjunto de dados tiverem escalas ou unidades diferentes. Nota: se todos os elementos de uma coluna tiverem o mesmo valor, os valores serão reescalados para 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Média:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Semelhante à escala Mín./Máx., mas os valores resultantes ficam centrados na média da respectiva coluna: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Nota: se todos os elementos de uma coluna tiverem o mesmo valor, os valores serão reescalados para 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Padronização:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Também conhecido como normalização Z-score, este método centra o valor na média e escala-o pelo desvio-padrão: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Escala unitária:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Os valores são divididos pelo comprimento euclidiano do vetor formado pela coluna de dados: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Quantil:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Normalizar os dados de forma que as colunas tenham distribuições iguais.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>A função softmax converte cada coluna de dados em uma distribuição de probabilidade, aplicando a exponencial a cada elemento e normalizando por divisão pela soma. Em última análise, isto faz com que os maiores valores de cada coluna sejam exagerados.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Filtragem:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Limiar</translation>
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
        <translation>&lt;b&gt;Limiar:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Filtro simples que descarta as correlações abaixo de um limiar selecionado.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k vizinhos mais próximos:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Acompanhar as correlações encontradas entre pares de linhas, descartando as que não estão entre as &lt;i&gt;k&lt;/i&gt; maiores correlações de cada linha. É mais útil para conjuntos de dados extremamente correlacionados.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Mínimo:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Valor mínimo de correlação</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>O valor mínimo de correlação acima do qual será criada uma aresta no grafo. Um valor mínimo mais baixo aumenta os requisitos de computação e memória.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>k máximo:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; máximo de correlação</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>O número máximo de correlações a manter por linha de dados, o que limita o número de arestas que serão criadas no grafo. Um valor máximo maior aumenta os requisitos de computação e memória.</translation>
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
        <translation>Limiar de correlação</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; de correlação</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>O limiar de correlação inicial determina o tamanho do grafo resultante. Um valor mais baixo filtra menos arestas e resulta em um grafo maior. Este valor pode ser alterado mais tarde, depois de o grafo ser criado.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>O valor k inicial determina o tamanho do grafo resultante. Um valor mais alto filtra menos arestas e resulta em um grafo maior. Este valor pode ser alterado mais tarde, depois de o grafo ser criado.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Processando…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Grafo vazio</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Modelo</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Modelo&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Um modelo é um conjunto com nome de transformações e visualizações que pode ser aplicado ao grafo depois de este ser criado. Selecione um modelo a aplicar. &lt;b&gt;Nota:&lt;/b&gt; as transformações aplicadas aqui serão sempre acrescentadas incondicionalmente, independentemente do método de aplicação específico do modelo.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Definir como padrão</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Resumo&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Será criado um grafo com os seguintes parâmetros:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Transposto&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Dataframe: [ Coluna: {0} Linha: {1} Largura: {2} Altura: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Valor mínimo de correlação: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Limiar de correlação inicial: {0}&lt;br&gt;</translation>
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
        <translation>Medida de correlação contínua: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Polaridade da correlação: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputação: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Imputação: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Corte: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Corte: {0} (percentil {1})</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Corte: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Escala: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalização: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Medida de correlação discreta: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Modelo: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Tamanho estimado do grafo antes das transformações: {0}{3} nós{2}, {1}{4} arestas{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Aumentar o valor mínimo de correlação normalmente reduz o tamanho do grafo.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Diminuir o &lt;i&gt;k&lt;/i&gt; máximo normalmente reduz o tamanho do grafo.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}AVISO: este é um grafo muito grande, que pode esgotar os recursos do sistema e causar instabilidade ou bloqueios. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}AVISO: é provável que o grafo gerado fique vazio.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 coluna</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n coluna</numerusform>
            <numerusform>%n colunas</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…e mais %1</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Redimensionar para mostrar informações das colunas</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Redimensionar para mostrar nomes das colunas</translation>
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
        <translation>Média de %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Média da seleção</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Mediana de %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Mediana da seleção</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Histograma médio de %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Histograma médio da seleção</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Desvio</translation>
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
        <translation>Desvio-padrão</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Erro-padrão</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 e mais %3…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>A extensão de %1 indica que é um “%2”, mas não foi possível determinar o seu conteúdo.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>A extensão de %1 indica que é um “%2”, mas o seu conteúdo assemelha-se a um arquivo “%3”.</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlação</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Cria um grafo em que os nós representam linhas de dados e as arestas representam correlações entre essas linhas.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>O valor de dados é um atributo parametrizado que permite referenciar uma coluna específica dos dados correlacionados.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>O nome da coluna de dados com o maior valor absoluto na linha de dados associada ao nó.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>O valor médio de dados é a média dos valores associados ao nó.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>O valor mínimo de dados é o menor valor associado ao nó.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>O valor máximo de dados é o maior valor associado ao nó.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>A %1 é uma medida da dispersão dos valores associados ao nó. É definida como ∑(&lt;i&gt;x&lt;/i&gt;-µ)², em que &lt;i&gt;x&lt;/i&gt; é o valor e µ é a média.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variância</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>O %1 é uma medida da dispersão dos valores associados ao nó. É definido como √∑(&lt;i&gt;x&lt;/i&gt;-µ)², em que &lt;i&gt;x&lt;/i&gt; é o valor e µ é a média.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desvio padrão</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>O %1 é uma medida da dispersão dos valores associados ao nó. É definido como o desvio-padrão dividido pela média.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coeficiente de variação</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>Falta a chave “%1” nos dados do plugin.</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Dataframe: [ Coluna: %1 Linha: %2 Largura: %3 Altura: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Transposto</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Medida de correlação contínua: %1</translation>
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
        <translation>Polaridade da correlação: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Medida de correlação discreta: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Valor mínimo de correlação: %1</translation>
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
        <translation>Média da coluna</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Interpolação por linha</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Imputação: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Corte: constante (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Corte: winsorização (percentil %1)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Escala: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Escala: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Escala: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Escala: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Escala: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Normalização: Mín./Máx.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Normalização: quantil</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Normalização: média</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Normalização: padronização</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalização: escala unitária</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalização: escala softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importar anotações de colunas</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Sem chaves adequadas</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Falha ao carregar {0}.</translation>
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
        <source>Introduction</source>
        <translation>Introdução</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Importar anotações&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Os dados de anotação podem ser importados de uma origem externa e sobrepostos ao gráfico. Você pode ser utilizado qualquer um dos formatos de dados tabulares suportados. Depois de importadas, as novas anotações podem ser ordenadas e agrupadas da forma habitual.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Seleção de chaves</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Seleção de chaves&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Para importar dados de anotação, é necessário identificar a linha dos dados de entrada que corresponde aos nomes das colunas dos dados existentes. É utilizada para fazer corresponder as colunas da tabela às colunas do gráfico. Selecione abaixo a linha correta. Como alternativa, clique em &lt;i&gt;{0}&lt;/i&gt; e a melhor correspondência será identificada automaticamente.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Detectar automaticamente</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>A linha selecionada &lt;b&gt;{0}&lt;/b&gt; tem valores duplicados; algumas colunas serão ignoradas!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Encontrada correspondência de {0}%</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Nenhuma correspondência encontrada</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Seleção de anotações</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Seleção de anotações&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Selecione as anotações desejadas na lista à direita. Você pode selecionar várias anotações mantendo pressionada a tecla &lt;i&gt;Ctrl&lt;/i&gt;, ou um intervalo mantendo pressionada a tecla &lt;i&gt;Shift&lt;/i&gt; e clicando conforme desejado.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Substituir anotações existentes</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Selecionar tudo</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Não selecionar nada</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Resumo</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Resumo&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Detectando a chave…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>As seguintes anotações serão importadas utilizando a linha &lt;b&gt;{0}{1}&lt;/b&gt; como chave:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(tem valores duplicados)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Não é possível importar anotações sem selecionar uma chave.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Detectando a chave</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Mostrar interface &amp;lado a lado</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Mostrar nomes das &amp;colunas</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Localizar linhas de interesse</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Selecionar anotações de colunas visíveis</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Linhas de grade</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Incluir zero no eixo Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Mostrar valores atípicos</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Redefinir zoom</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Mostrar todas as colunas</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Nenhum</translation>
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
        <translation>Centralização na média</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Escala de variância unitária</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Escala de Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Barras de erro</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Área</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desvio padrão</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Erro-padrão</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Linha individual</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Linha da média</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Linha da mediana</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Histograma médio</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Amplitude interquartil</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Agrupar por anotação</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Importar anotações de tabela…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-grafico-correlacao</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Salvar como &amp;imagem…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Definir rótulo do eixo X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Definir rótulo do eixo Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Rótulo do eixo X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Digite o rótulo do eixo X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Rótulo do eixo Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Digite o rótulo do eixo Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Gráfico</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Rótulos dos eixos</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Escala</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Por atributo</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Média</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Todos</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Dispersão</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Ordenar colunas por</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Colorir grupos por</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Salvar como imagens por…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Linha</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Ordem natural</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Nome da coluna</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valor de dados</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Agrupamento hierárquico</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Clique em uma coluna ou em uma anotação de coluna para selecioná-la. Mantenha &lt;i&gt;Ctrl&lt;/i&gt; pressionado e clique para selecionar várias colunas.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Fixar ordem de ordenação por valor de dados</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Salvar gráfico como imagem</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Imagem PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Imagem JPEG (*.jpg *.jpeg)</translation>
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
        <translation>Mostrar todas as anotações</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Ocultar todas as anotações</translation>
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
        <translation>Ponderação da seleção</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>As linhas são selecionadas ponderando a soma das colunas selecionadas face às colunas não selecionadas. Ponderar as colunas selecionadas (indicado como &lt;i&gt;Absoluta&lt;/i&gt;) tende a encontrar linhas com picos nessas colunas, independentemente das colunas não selecionadas. Pelo contrário, ponderar (negativamente) as colunas não selecionadas (indicado como &lt;i&gt;Relativa&lt;/i&gt;) tende a encontrar linhas com valores significativos especificamente nas colunas selecionadas.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}º</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}º</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}º</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}º</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Percentil</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>A quantidade de linhas a selecionar e classificar.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Voltar a selecionar</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Amplitude interquartil de %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Amplitude interquartil</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Salvando imagens do gráfico</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>O coeficiente de correlação de Pearson indica a correlação linear entre duas variáveis.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Valor de correlação de Pearson</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>O %1 indica a relação linear entre duas variáveis.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Coeficiente de correlação de Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Ordem de Spearman</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>O coeficiente de correlação de postos de Spearman é uma medida não paramétrica da dependência estatística entre as ordenações de duas variáveis. Avalia até que ponto a relação entre duas variáveis pode ser descrita por uma função monótona.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Valor de correlação de postos de Spearman</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>O %1 indica a relação monótona entre duas variáveis.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Coeficiente de correlação de postos de Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Semelhança euclidiana</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>A semelhança euclidiana é essencialmente o inverso da distância euclidiana entre dois vetores.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>A %1 é essencialmente o inverso da distância euclidiana entre dois vetores.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Semelhança de cosseno</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>A semelhança de cosseno é uma medida de semelhança entre dois vetores não nulos de um espaço com produto interno. É definida como o cosseno do ângulo entre eles, o que equivale também ao produto interno dos mesmos vetores normalizados para terem ambos comprimento 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>A %1 é uma medida de semelhança entre dois vetores não nulos de um espaço com produto interno.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>A Biweight Midcorrelation (também chamada Bicor) é uma medida de semelhança entre amostras. Baseia-se na mediana em vez da média, pelo que é menos sensível a valores atípicos.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Valor de correlação Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 é uma medida de semelhança entre amostras baseada na mediana.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>O índice de Jaccard é uma estatística utilizada para avaliar a semelhança e a diversidade de conjuntos de amostras.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Valor de correlação de Jaccard</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>O %1 é uma estatística utilizada para avaliar a semelhança e a diversidade de conjuntos de amostras.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Índice de Jaccard</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Coeficiente de correspondência simples</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>O coeficiente de correspondência simples é uma estatística utilizada para avaliar a semelhança e a diversidade de conjuntos de amostras. É idêntico ao de Jaccard, exceto que também conta as ausências mútuas.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Coeficiente de correspondência simples (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Valor de dados</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Coluna de dados mais forte</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Valor médio de dados</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Valor mínimo de dados</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Valor máximo de dados</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Variância</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Desvio padrão</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Coeficiente de variação</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 absoluta</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Dados</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Valores de correlação</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Arquivo CSV de correlação</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Arquivos CSV de correlação</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Arquivo TSV de correlação</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Arquivos TSV de correlação</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Arquivo SSV de correlação</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Arquivos SSV de correlação</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Arquivo Excel de correlação</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Arquivos Excel de correlação</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Calculando correlação</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Agrupamento</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Importar anotações de colunas</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Importar anotação de coluna</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Importando anotações de colunas</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Importando anotação de coluna</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n anotação importada</numerusform>
            <numerusform>%n anotações importadas</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Anotação %1 importada</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Normalização</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Correlação</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Construindo o grafo</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Ordenando colunas</translation>
    </message>
</context>
</TS>
