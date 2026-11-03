<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/CorrelationParameters.qml" line="+138"/>
        <source>Failed to Load {0}.</source>
        <translation>Failed to Load {0}.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Loading {0}…</source>
        <translation>Loading {0}…</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Close</source>
        <translation>Close</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cancel</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Introduction</source>
        <translation>Introduction</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalized, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>correlation coefficient</source>
        <translation>correlation coefficient</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Data Selection</source>
        <translation>Data Selection</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+13"/>
        <source>Transpose</source>
        <translation>Transpose</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Data Type:</source>
        <translation>Data Type:</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Continuous</source>
        <translation>Continuous</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Discrete</source>
        <translation>Discrete</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Data Type</source>
        <translation>Data Type</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Continuous:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Discrete:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+12"/>
        <source>Treat As Binary</source>
        <translation>Treat As Binary</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Correlation</source>
        <translation>Correlation</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Advanced</source>
        <translation>Advanced</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+43"/>
        <source>Algorithm:</source>
        <translation>Algorithm:</translation>
    </message>
    <message>
        <location line="-30"/>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>SMC</source>
        <translation>SMC</translation>
    </message>
    <message>
        <location line="+18"/>
        <location line="+46"/>
        <source>Correlation Algorithm</source>
        <translation>Correlation Algorithm</translation>
    </message>
    <message>
        <location line="-22"/>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spearman Rank</source>
        <translation>Spearman Rank</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Euclidean Similarity</source>
        <translation>Euclidean Similarity</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Cosine Similarity</source>
        <translation>Cosine Similarity</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Polarity:</source>
        <translation>Polarity:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Positive</source>
        <translation>Positive</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Negative</source>
        <translation>Negative</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Both</source>
        <translation>Both</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Polarity</source>
        <translation>Polarity</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Imputation:</source>
        <translation>Imputation:</translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+113"/>
        <source>Constant</source>
        <translation>Constant</translation>
    </message>
    <message>
        <location line="-112"/>
        <source>Row Interpolate</source>
        <translation>Row Interpolate</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Column Mean</source>
        <translation>Column Mean</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Imputation</source>
        <translation>Imputation</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+146"/>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Constant:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="-139"/>
        <source>Replace all missing values with a constant.</source>
        <translation>Replace all missing values with a constant.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Row Interpolate:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Column Mean:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Replace missing values with the mean value of their column, excluding missing values.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Clipping:</source>
        <translation>Clipping:</translation>
    </message>
    <message>
        <location line="+13"/>
        <location line="+131"/>
        <location line="+90"/>
        <location line="+549"/>
        <source>None</source>
        <translation>None</translation>
    </message>
    <message>
        <location line="-768"/>
        <source>Winsorization</source>
        <translation>Winsorization</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>percentile</source>
        <translation>percentile</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Clipping</source>
        <translation>Clipping</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Limit the maximum data table value to a specified constant.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorization:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Scaling:</source>
        <translation>Scaling:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Log2(x + ε)</source>
        <translation>Log2(x + ε)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Log10(x + ε)</source>
        <translation>Log10(x + ε)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AntiLog2(x)</source>
        <translation>AntiLog2(x)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AntiLog10(x)</source>
        <translation>AntiLog10(x)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Arcsin(x)</source>
        <translation>Arcsin(x)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Scaling Types</source>
        <translation>Scaling Types</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normalisation:</source>
        <translation>Normalization:</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Min/Max</source>
        <translation>Min/Max</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mean</source>
        <translation>Mean</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Standardisation</source>
        <translation>Standardization</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Unit Scaling</source>
        <translation>Unit Scaling</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Quantile</source>
        <translation>Quantile</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Normalisation Types</source>
        <translation>Normalization Types</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Min/Max:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mean:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Similar to Min/Max scaling except the resultant values are centered around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Standardization:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Also known as Z-score normalization, this method centers the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Unit Scaling:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Quantile:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Normalize the data so that the columns have equal distributions.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalizing by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Filtering:</source>
        <translation>Filtering:</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Threshold</source>
        <translation>Threshold</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Filtering</source>
        <translation>Filtering</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Threshold:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Simple filter where correlations below a selected threshold are discarded.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k-Nearest Neighbors:&lt;/b&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Minimum Correlation Value</source>
        <translation>Minimum Correlation Value</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Maximum k:</source>
        <translation>Maximum k:</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Initial:</source>
        <translation>Initial:</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Initial k:</source>
        <translation>Initial k:</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Correlation Threshold</source>
        <translation>Correlation Threshold</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Correlation &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Working…</source>
        <translation>Working…</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Empty Graph</source>
        <translation>Empty Graph</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Template</source>
        <translation>Template</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Template&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>A template is a named set of transforms and visualizations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Set As Default</source>
        <translation>Set As Default</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>Summary</source>
        <translation>Summary</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Summary&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>A graph will be created using the following parameters:</source>
        <translation>A graph will be created using the following parameters:</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Transposed&lt;br&gt;</source>
        <translation>Transposed&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+8"/>
        <location line="+5"/>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Minimum Correlation Value: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="-4"/>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Initial Correlation Threshold: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Maximum k Value: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Initial k Value: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Continuous Correlation Metric: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Correlation Polarity: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Imputation: {0} ({1})</source>
        <translation>Imputation: {0} ({1})</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Imputation: {0}</source>
        <translation>Imputation: {0}</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Clipping: {0} ({1})</source>
        <translation>Clipping: {0} ({1})</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Clipping: {0} ({1} percentile)</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Clipping: {0}</source>
        <translation>Clipping: {0}</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Scaling: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Normalization: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Discrete Correlation Metric: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Template: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Increasing the Minimum Correlation Value will usually reduce the graph size.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplot.cpp" line="+722"/>
        <source>1 column</source>
        <translation>1 column</translation>
    </message>
    <message numerus="yes">
        <location line="+0"/>
        <source>%n columns</source>
        <translation>
            <numerusform>%n columns</numerusform>
            <numerusform>%n columns</numerusform>
        </translation>
    </message>
    <message>
        <location line="+5"/>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <location line="+223"/>
        <source>…and %1 more</source>
        <translation>…and %1 more</translation>
    </message>
    <message>
        <location line="+807"/>
        <source>Resize To Expose Column Information</source>
        <translation>Resize To Expose Column Information</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Resize To Expose Column Names</source>
        <translation>Resize To Expose Column Names</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplot_columnannotations.cpp" line="+405"/>
        <source>%n unique values</source>
        <translation>
            <numerusform>%n unique value</numerusform>
            <numerusform>%n unique values</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplot_continuous.cpp" line="+323"/>
        <source>Mean average of %1: %2</source>
        <translation>Mean average of %1: %2</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Mean average of selection</source>
        <translation>Mean average of selection</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Median average of %1: %2</source>
        <translation>Median average of %1: %2</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Median average of selection</source>
        <translation>Median average of selection</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Mean histogram of %1: %2</source>
        <translation>Mean histogram of %1: %2</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Mean histogram of selection</source>
        <translation>Mean histogram of selection</translation>
    </message>
    <message>
        <location line="+127"/>
        <source>Deviation</source>
        <translation>Deviation</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>%1 Top</source>
        <translation>%1 Top</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 Bottom</source>
        <translation>%1 Bottom</translation>
    </message>
    <message>
        <location line="+63"/>
        <source>Std Dev</source>
        <translation>Std Dev</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Std Err</source>
        <translation>Std Err</translation>
    </message>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplot_discrete.cpp" line="+133"/>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 and %3 more…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <location filename="../correlationplugin.cpp" line="+1593"/>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</translation>
    </message>
    <message>
        <location filename="../correlationplugin.h" line="+246"/>
        <source>Correlation</source>
        <translation>Correlation</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <location filename="../correlationplugin.cpp" line="-1319"/>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>The Data Value is a parameterized attribute that permits referencing a specific column in the correlated data.</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>The name of the data column which has the largest absolute value for the data row associated with the node.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>The Mean Data Value is the mean of the values associated with the node.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>The Minimum Data Value is the minimum value associated with the node.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>The Maximum Data Value is the maximum value associated with the node.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Variance</source>
        <translation>Variance</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Standard Deviation</source>
        <translation>Standard Deviation</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Coefficient of Variation</source>
        <translation>Coefficient of Variation</translation>
    </message>
    <message>
        <location line="+974"/>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>Plugin data is missing &apos;%1&apos; key.</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Transposed</source>
        <translation>Transposed</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Continuous Correlation Metric: %1</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Positive</source>
        <translation>Positive</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Negative</source>
        <translation>Negative</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Both</source>
        <translation>Both</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Correlation Polarity: %1</source>
        <translation>Correlation Polarity: %1</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Discrete Correlation Metric: %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Minimum Correlation Value: %1</source>
        <translation>Minimum Correlation Value: %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Maximum k Value: %1</source>
        <translation>Maximum k Value: %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Constant (%1)</source>
        <translation>Constant (%1)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Column Mean</source>
        <translation>Column Mean</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Row Interpolate</source>
        <translation>Row Interpolate</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Imputation: %1</source>
        <translation>Imputation: %1</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Clipping: Constant (%1)</source>
        <translation>Clipping: Constant (%1)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Clipping: Winsorization (%1 percentile)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Scaling: Log2(x + ε)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Scaling: Log10(x + ε)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Scaling: AntiLog2(x)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Scaling: AntiLog10(x)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Scaling: Arcsin(x)</source>
        <translation>Scaling: Arcsin(x)</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Normalisation: Min/Max</source>
        <translation>Normalization: Min/Max</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normalisation: Quantile</source>
        <translation>Normalization: Quantile</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normalisation: Mean</source>
        <translation>Normalization: Mean</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normalisation: Standardisation</source>
        <translation>Normalization: Standardization</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normalisation: Unit Scaling</source>
        <translation>Normalization: Unit Scaling</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Normalization: Softmax Scaling</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/ImportAnnotationsDialog.qml" line="+36"/>
        <source>Import Column Annotations</source>
        <translation>Import Column Annotations</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>No Suitable Keys</source>
        <translation>No Suitable Keys</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Failed to Load {0}.</source>
        <translation>Failed to Load {0}.</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Loading {0}…</source>
        <translation>Loading {0}…</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Close</source>
        <translation>Close</translation>
    </message>
    <message>
        <location line="+0"/>
        <location line="+422"/>
        <source>Cancel</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location line="-377"/>
        <source>Introduction</source>
        <translation>Introduction</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Import Annotations&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Key Selection</source>
        <translation>Key Selection</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Key Selection&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Auto Detect</source>
        <translation>Auto Detect</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Found {0}% Match</source>
        <translation>Found {0}% Match</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>No Match Found</source>
        <translation>No Match Found</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Annotation Selection</source>
        <translation>Annotation Selection</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Replace Existing Annotations</source>
        <translation>Replace Existing Annotations</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Select All</source>
        <translation>Select All</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Select None</source>
        <translation>Select None</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Summary</source>
        <translation>Summary</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Summary&lt;/h2&gt;</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Detecting key…</source>
        <translation>Detecting key…</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</translation>
    </message>
    <message>
        <location line="+4"/>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Detecting Key</source>
        <translation>Detecting Key</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/Main.qml" line="+48"/>
        <source>Display UI &amp;Side By Side</source>
        <translation>Display UI &amp;Side By Side</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Show &amp;Column Names</source>
        <translation>Show &amp;Column Names</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Find Rows Of Interest</source>
        <translation>Find Rows Of Interest</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Select Visible Column Annotations</source>
        <translation>Select Visible Column Annotations</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Grid Lines</source>
        <translation>Grid Lines</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Legend</source>
        <translation>Legend</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Include Zero In Y Axis</source>
        <translation>Include Zero In Y Axis</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Show Outliers</source>
        <translation>Show Outliers</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Reset Zoom</source>
        <translation>Reset Zoom</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Show All Columns</source>
        <translation>Show All Columns</translation>
    </message>
    <message>
        <location line="+16"/>
        <location line="+83"/>
        <location line="+484"/>
        <source>None</source>
        <translation>None</translation>
    </message>
    <message>
        <location line="-558"/>
        <source>Log(x + ε)</source>
        <translation>Log(x + ε)</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>AntiLog</source>
        <translation>AntiLog</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Mean Centre Scaling</source>
        <translation>Mean Center Scaling</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unit Variance Scaling</source>
        <translation>Unit Variance Scaling</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Pareto Scaling</source>
        <translation>Pareto Scaling</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Error Bars</source>
        <translation>Error Bars</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Area</source>
        <translation>Area</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Standard Deviation</source>
        <translation>Standard Deviation</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Standard Error</source>
        <translation>Standard Error</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Individual Line</source>
        <translation>Individual Line</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Mean Line</source>
        <translation>Mean Line</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Median Line</source>
        <translation>Median Line</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Mean Histogram</source>
        <translation>Mean Histogram</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>IQR</source>
        <translation>IQR</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Group By Annotation</source>
        <translation>Group By Annotation</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Import Annotations From Table…</source>
        <translation>Import Annotations From Table…</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-correlation-plot</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Save As &amp;Image…</source>
        <translation>Save As &amp;Image…</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Set X Axis Label…</source>
        <translation>Set X Axis Label…</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Set Y Axis Label…</source>
        <translation>Set Y Axis Label…</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>X Axis Label</source>
        <translation>X Axis Label</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Please enter the X Axis label:</source>
        <translation>Please enter the X Axis label:</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Y Axis Label</source>
        <translation>Y Axis Label</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Please enter the Y Axis label:</source>
        <translation>Please enter the Y Axis label:</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>&amp;Plot</source>
        <translation>&amp;Plot</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Axis Labels</source>
        <translation>Axis Labels</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Scaling</source>
        <translation>Scaling</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+29"/>
        <source>By Attribute</source>
        <translation>By Attribute</translation>
    </message>
    <message>
        <location line="-10"/>
        <source>Averaging</source>
        <translation>Averaging</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>All</source>
        <translation>All</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Dispersion</source>
        <translation>Dispersion</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Sort Columns By</source>
        <translation>Sort Columns By</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Colour Groups By</source>
        <translation>Color Groups By</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Save As Images By…</source>
        <translation>Save As Images By…</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Row</source>
        <translation>Row</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Natural Order</source>
        <translation>Natural Order</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Column Name</source>
        <translation>Column Name</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Data Value</source>
        <translation>Data Value</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Hierarchical Clustering</source>
        <translation>Hierarchical Clustering</translation>
    </message>
    <message>
        <location line="+279"/>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Fix Data Value Sort Order</source>
        <translation>Fix Data Value Sort Order</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Save Plot As Image</source>
        <translation>Save Plot As Image</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>PNG Image (*.png)</source>
        <translation>PNG Image (*.png)</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>JPEG Image (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PDF Document (*.pdf)</source>
        <translation>PDF Document (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/PlotModalControls.qml" line="+81"/>
        <source>Show All Annotations</source>
        <translation>Show All Annotations</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Hide All Annotations</source>
        <translation>Hide All Annotations</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Relative</source>
        <translation>Relative</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Absolute</source>
        <translation>Absolute</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Selection Weighting</source>
        <translation>Selection Weighting</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>{0}st</source>
        <translation>{0}st</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>{0}nd</source>
        <translation>{0}nd</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>{0}rd</source>
        <translation>{0}rd</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>{0}th</source>
        <translation>{0}th</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Percentile</source>
        <translation>Percentile</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The quantity of rows to select and rank.</source>
        <translation>The quantity of rows to select and rank.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Reselect</source>
        <translation>Reselect</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplot_continuous.cpp" line="-217"/>
        <source>IQR of %1</source>
        <translation>IQR of %1</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>IQR</source>
        <translation>IQR</translation>
    </message>
    <message>
        <location filename="../ui/qml/Graphia/Plugins/Correlation/correlationplotsaveimagecommand.h" line="+65"/>
        <source>Saving Plot Image(s)</source>
        <translation>Saving Plot Image(s)</translation>
    </message>
    <message>
        <location filename="../correlation.cpp" line="+256"/>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pearson Correlation Value</source>
        <translation>Pearson Correlation Value</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>The %1 is an indication of the linear relationship between two variables.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pearson Correlation Coefficient</source>
        <translation>Pearson Correlation Coefficient</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Spearman Rank</source>
        <translation>Spearman Rank</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Spearman Rank Correlation Value</source>
        <translation>Spearman Rank Correlation Value</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>The %1 is an indication of the monotonic relationship between two variables.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Spearman Rank Correlation Coefficient</translation>
    </message>
    <message>
        <location line="+12"/>
        <location line="+8"/>
        <location line="+5"/>
        <source>Euclidean Similarity</source>
        <translation>Euclidean Similarity</translation>
    </message>
    <message>
        <location line="-10"/>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1 is essentially the inverse of the Euclidean distance between two vectors.</translation>
    </message>
    <message>
        <location line="+14"/>
        <location line="+10"/>
        <location line="+5"/>
        <source>Cosine Similarity</source>
        <translation>Cosine Similarity</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1 is a measure of similarity between two non-zero vectors of an inner product space.</translation>
    </message>
    <message>
        <location line="+19"/>
        <location line="+13"/>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <location line="-10"/>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Bicor Correlation Value</source>
        <translation>Bicor Correlation Value</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 is a median-based measure of similarity between samples.</translation>
    </message>
    <message>
        <location line="+182"/>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Jaccard Correlation Value</source>
        <translation>Jaccard Correlation Value</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+23"/>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</translation>
    </message>
    <message>
        <location line="-21"/>
        <source>Jaccard Index</source>
        <translation>Jaccard Index</translation>
    </message>
    <message>
        <location line="+9"/>
        <location line="+9"/>
        <source>Simple Matching Coefficient</source>
        <translation>Simple Matching Coefficient</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Simple Matching Coefficient (SMC)</translation>
    </message>
    <message>
        <location filename="../correlationplugin.cpp" line="-1226"/>
        <source>Data Value</source>
        <translation>Data Value</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Strongest Data Column</source>
        <translation>Strongest Data Column</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Mean Data Value</source>
        <translation>Mean Data Value</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Minimum Data Value</source>
        <translation>Minimum Data Value</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Maximum Data Value</source>
        <translation>Maximum Data Value</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Variance</source>
        <translation>Variance</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Standard Deviation</source>
        <translation>Standard Deviation</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Coefficient of Variation</source>
        <translation>Coefficient of Variation</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Absolute %1</source>
        <translation>Absolute %1</translation>
    </message>
    <message>
        <location line="+775"/>
        <location line="+195"/>
        <source>Data</source>
        <translation>Data</translation>
    </message>
    <message>
        <location line="-140"/>
        <location line="+237"/>
        <source>Correlation Values</source>
        <translation>Correlation Values</translation>
    </message>
    <message>
        <location line="+164"/>
        <source>Correlation CSV File</source>
        <translation>Correlation CSV File</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Correlation CSV Files</source>
        <translation>Correlation CSV Files</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Correlation TSV File</source>
        <translation>Correlation TSV File</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Correlation TSV Files</source>
        <translation>Correlation TSV Files</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Correlation SSV File</source>
        <translation>Correlation SSV File</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Correlation SSV Files</source>
        <translation>Correlation SSV Files</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Correlation Excel File</source>
        <translation>Correlation Excel File</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Correlation Excel Files</source>
        <translation>Correlation Excel Files</translation>
    </message>
    <message>
        <location filename="../hierarchicalclusteringcommand.cpp" line="+92"/>
        <source>Correlating</source>
        <translation>Correlating</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Clustering</source>
        <translation>Clustering</translation>
    </message>
    <message>
        <location filename="../importannotationscommand.cpp" line="+53"/>
        <source>Import Column Annotations</source>
        <translation>Import Column Annotations</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Import Column Annotation</source>
        <translation>Import Column Annotation</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Importing Column Annotations</source>
        <translation>Importing Column Annotations</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Importing Column Annotation</source>
        <translation>Importing Column Annotation</translation>
    </message>
    <message numerus="yes">
        <location line="+6"/>
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n Annotation Imported</numerusform>
            <numerusform>%n Annotations Imported</numerusform>
        </translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Annotation %1 Imported</source>
        <translation>Annotation %1 Imported</translation>
    </message>
    <message>
        <location filename="../loading/correlationfileparser.cpp" line="+115"/>
        <source>Attributes</source>
        <translation>Attributes</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normalisation</source>
        <translation>Normalization</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Correlation</source>
        <translation>Correlation</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Building Graph</source>
        <translation>Building Graph</translation>
    </message>
    <message>
        <location filename="../hierarchicalclusteringcommand.h" line="+47"/>
        <source>Sorting Columns</source>
        <translation>Sorting Columns</translation>
    </message>
</context>
</TS>
