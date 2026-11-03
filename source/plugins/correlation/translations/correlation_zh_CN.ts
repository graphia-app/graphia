<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>加载 {0} 失败。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>正在加载 {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>简介</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;相关图分析&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>相关插件根据数据集中变量之间的相似性来创建图。&lt;br&gt;&lt;br&gt;{0}提供这种相似性度量。如果指定，可以先对输入数据进行缩放和归一化，然后用相关分数来决定是否在代表数据行的节点之间创建边。&lt;br&gt;&lt;br&gt;创建图之后，可以使用变换来过滤边。</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>相关系数</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>数据选择</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;数据选择 - 选择和调整&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>已自动为您的数据集选择了一个数据框。如果要调整，请在下方选择新的起始单元格。请注意，数据框总是延伸到数据集最右下方的单元格。</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>转置</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>转置会沿对角线翻转数据，把行变为列，反之亦然。</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>数据类型：</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>连续</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>离散</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>数据类型</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;连续：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>将所选数据解释为连续数据行。行与行之间将使用数值相关度量。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;离散：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>将所选数据解释为离散值的行。这些值可以是数值，但通常是文本。行与行之间将使用离散相关度量。</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>视为二值</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>启用此选项后，在相关算法中，空单元格或值为 &lt;i&gt;0&lt;/i&gt; 或 &lt;i&gt;false&lt;/i&gt; 的单元格视为假，其他所有值视为真。</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;警告：所选数据框似乎包含连续数据；将其解释为离散数据可能只会得到较低的相关性。&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相关</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;相关 - 调整参数&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>高级</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>在下方图表的横轴上，值 1 表示完全相关的行，而 0 表示不相关。低于所选最小相关阈值的所有绝对值都将被丢弃，不会用于创建边。默认情况下会添加一个变换，移除绝对值低于指定初始相关阈值的所有边。&lt;br&gt;&lt;br&gt;如需更好地控制所用参数，请勾选右上角的&lt;b&gt;高级&lt;/b&gt;选项。</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>算法：</translation>
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
        <translation>相关算法</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman 秩</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>欧几里得相似度</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>余弦相似度</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>极性：</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>正</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>负</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>两者</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>极性</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>默认情况下，创建图时只考虑正相关。几乎在所有情况下这都是正确的设置，但对某些数据源，考虑相关的极性可能更合理。请注意，并非所有相关算法都能发现负相关关系。</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>插补：</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>常数</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>行插值</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>列均值</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>插补</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;常数：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>用一个常数替换所有缺失值。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;行插值：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>使用行中最近的周围值及其相对位置对缺失值进行线性插值。如果只有一个周围值可用，则缺失值将设为与之相同。如果该行为空，则值设为零。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;列均值：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>用所在列的均值（不含缺失值）替换缺失值。</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>截断：</translation>
    </message>
    <message>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>缩尾处理</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>百分位</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>截断</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>将数据表的最大值限制为指定的常数。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;缩尾处理：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>逐行将最大数据值限制为指定百分位处的值。例如，如果选择第 95 百分位，则排在前 5% 的值将被设为第 95 百分位的值。</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>缩放：</translation>
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
        <translation>缩放类型</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε)：</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>计算 &lt;i&gt;x&lt;/i&gt; + ε 以 &lt;i&gt;b&lt;/i&gt; 为底的对数，其中 &lt;i&gt;x&lt;/i&gt; 为输入数据，ε 为一个极小的常数。</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;)：</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>求 &lt;i&gt;b&lt;/i&gt; 的 &lt;i&gt;x&lt;/i&gt; 次幂，其中 &lt;i&gt;x&lt;/i&gt; 为输入数据。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;)：</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>对 &lt;i&gt;x&lt;/i&gt; 求反正弦，其中 &lt;i&gt;x&lt;/i&gt; 为输入数据。当需要对数变换但数据集中含有负数或零时，这很有用。</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>归一化：</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>最小/最大</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>均值</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>标准化</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>单位缩放</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>分位数</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>归一化类型</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;最小/最大：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>重新缩放数据，使其值位于 [0, 1] 范围内：(&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;))。当数据集中各列的尺度或单位不同时，这很有用。注意：如果某列中所有元素的值都相同，这些值将被缩放为 0.0。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;均值：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>与最小/最大缩放类似，但结果值以所在列的均值为中心：(&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;))。注意：如果某列中所有元素的值都相同，这些值将被缩放为 0.0。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;标准化：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>也称为 Z 分数归一化，此方法将值以均值为中心，并按标准差缩放：(&lt;i&gt;x&lt;/i&gt;-µ)/σ。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;单位缩放：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>将值除以该列数据所构成向量的欧几里得长度：&lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;分位数：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>对数据进行归一化，使各列具有相同的分布。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Softmax 函数对每个元素取指数，再除以总和进行归一化，从而把每列数据转换为概率分布。最终会使每列中的最大值被放大。</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>过滤：</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>阈值</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>正在过滤</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;阈值：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>简单过滤，丢弃低于所选阈值的相关性。</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k 近邻：&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>跟踪行对之间的相关性，丢弃不在每行前 &lt;i&gt;k&lt;/i&gt; 个相关性之内的相关。这对相关性极高的数据集最为有用。</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>最小值：</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>最小相关值</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>高于此最小相关值时将在图中创建边。最小值越低，计算量和内存需求越大。</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>最大 k：</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>最大相关 &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>每个数据行保留的最大相关数，最终限制图中将创建的边数。最大值越大，计算量和内存需求越大。</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>初始值：</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>初始 k：</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>相关阈值</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>相关 &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>初始相关阈值决定所生成图的大小。值越低，过滤掉的边越少，得到的图越大。创建图之后仍可更改此值。</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>初始 k 值决定所生成图的大小。值越高，过滤掉的边越少，得到的图越大。创建图之后仍可更改此值。</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>正在处理…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>空图</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>模板</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;模板&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>模板是一组具名的变换和可视化，可在图创建后应用到图上。请选择要应用的模板。&lt;b&gt;注意：&lt;/b&gt;无论模板本身的应用方式如何，在此处应用的变换总是会无条件追加。</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>名称：</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>设为默认</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>摘要</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;摘要&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>将使用以下参数创建图：</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>已转置&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>数据框：[ 列：{0} 行：{1} 宽：{2} 高：{3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>最小相关值：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>初始相关阈值：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>最大 k 值：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>初始 k 值：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>连续相关度量：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>相关极性：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>插补：{0}（{1}）</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>插补：{0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>截断：{0}（{1}）</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>截断：{0}（第 {1} 百分位）</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>截断：{0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>缩放：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>归一化：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>离散相关度量：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>模板：{0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>预计变换前的图大小：{0}{3} 个节点{2}，{1}{4} 条边{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>提高最小相关值通常会减小图的大小。</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>减小最大 &lt;i&gt;k&lt;/i&gt; 通常会减小图的大小。</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}警告：这是一个非常大的图，可能耗尽系统资源，导致不稳定或卡死。{1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}警告：生成的图很可能为空。{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 列</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n 列</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…以及另外 %1 个</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>调整大小以显示列信息</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>调整大小以显示列名</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1（%2）</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n 个唯一值</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>%1 的均值：%2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>所选内容的均值</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>%1 的中位数：%2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>所选内容的中位数</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>%1 的均值直方图：%2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>所选内容的均值直方图</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>偏差</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 上界</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 下界</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>标准差</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>标准误</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1、%2 及另外 %3 个…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>%1 的扩展名表明它是“%2”，但无法确定其内容。</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>%1 的扩展名表明它是“%2”，但其内容看起来像“%3”文件。</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相关</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>创建一个图，其中节点代表数据行，边代表这些行之间的相关性。</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>数据值是一个带参数的属性，可用于引用相关数据中的特定列。</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>对于与节点关联的数据行，绝对值最大的数据列的名称。</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>平均数据值是与节点关联的值的平均值。</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>最小数据值是与节点关联的最小值。</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>最大数据值是与节点关联的最大值。</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 衡量与节点关联的值的离散程度。其定义为 ∑(&lt;i&gt;x&lt;/i&gt;-µ)²，其中 &lt;i&gt;x&lt;/i&gt; 为值，µ 为均值。</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>方差</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 衡量与节点关联的值的离散程度。其定义为 √∑(&lt;i&gt;x&lt;/i&gt;-µ)²，其中 &lt;i&gt;x&lt;/i&gt; 为值，µ 为均值。</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>标准差</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>%1 衡量与节点关联的值的离散程度。其定义为标准差除以均值。</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>变异系数</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>插件数据缺少“%1”键。</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>数据框：[ 列：%1 行：%2 宽：%3 高：%4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>已转置</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>连续相关度量：%1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>正</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>负</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>两者</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>相关极性：%1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>离散相关度量：%1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>最小相关值：%1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>最大 k 值：%1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>常数（%1）</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>列均值</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>行插值</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>插补：%1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>截断：常数（%1）</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>截断：缩尾处理（第 %1 百分位）</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>缩放：Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>缩放：Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>缩放：AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>缩放：AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>缩放：Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>归一化：最小/最大</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>归一化：分位数</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>归一化：均值</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>归一化：标准化</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>归一化：单位缩放</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>归一化：Softmax 缩放</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>导入列注释</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>没有合适的键</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>加载 {0} 失败。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>正在加载 {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>简介</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;导入注释&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>可以从外部来源导入注释数据，并叠加显示在图表上。可以使用任何受支持的表格数据格式。导入新注释后，可以按通常的方式对其排序和分组。</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>键选择</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;键选择&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>要导入注释数据，需要找出与现有数据列名对应的输入数据行，以便将表中的列与图表中的列对应起来。请在下方选择正确的行。也可以单击&lt;i&gt;{0}&lt;/i&gt;，自动找出最佳匹配。</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>所选行 &lt;b&gt;{0}&lt;/b&gt; 中有重复值，部分列将被忽略！</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>找到 {0}% 匹配</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>未找到匹配</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>注释选择</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;注释选择&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>请从右侧列表中选择所需的注释。按住 &lt;i&gt;Ctrl&lt;/i&gt; 可选择多个注释，按住 &lt;i&gt;Shift&lt;/i&gt; 再单击可选择一个范围。</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>替换现有注释</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>取消全选</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>摘要</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;摘要&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>正在检测键…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>以下注释将以行 &lt;b&gt;{0}{1}&lt;/b&gt; 作为键导入：</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;（含有重复值）&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;必须选择一个键才能导入注释。&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>正在检测键</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>并排显示界面(&amp;S)</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>显示列名(&amp;C)</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>查找感兴趣的行</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>选择可见的列注释</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>网格线</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>图例</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Y 轴包含零</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>显示离群值</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>重置缩放</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>显示所有列</translation>
    </message>
    <message>
        <source>None</source>
        <translation>无</translation>
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
        <translation>均值中心化缩放</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>单位方差缩放</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Pareto 缩放</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>误差线</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>区域</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>标准差</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>标准误</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>单独折线</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>均值线</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>中位数线</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>均值直方图</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>四分位距</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>按注释分组</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>从表格导入注释…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-相关图</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>另存为图像(&amp;I)…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>设置 X 轴标签…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>设置 Y 轴标签…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>X 轴标签</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>请输入 X 轴标签：</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Y 轴标签</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>请输入 Y 轴标签：</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>图表(&amp;P)</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>坐标轴标签</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>缩放</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>按属性</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>平均</translation>
    </message>
    <message>
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>离散程度</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>列排序依据</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>分组着色依据</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>按以下方式另存为图像…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>行</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>自然顺序</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>列名</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>数据值</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>层次聚类</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>单击列或列注释即可选择。按住 &lt;i&gt;Ctrl&lt;/i&gt; 并单击可选择多列。</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>固定数据值排序顺序</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>将图表另存为图像</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>PNG 图像 (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>JPEG 图像 (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>PDF 文档 (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>显示所有注释</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>隐藏所有注释</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>相对</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>绝对</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>选择权重</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>选择行的依据是所选列之和相对于未选列的加权。对所选列加权（记为&lt;i&gt;绝对&lt;/i&gt;）倾向于找出在这些列中有峰值的行，而不考虑未选列。相反，对未选列（负）加权（记为&lt;i&gt;相对&lt;/i&gt;）倾向于找出恰好在所选列中有显著值的行。</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>第 {0}</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>第 {0}</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>第 {0}</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>第 {0}</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>百分位</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>要选择并排序的行数。</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>重新选择</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>%1 的四分位距</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>四分位距</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>正在保存图表图像</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Pearson 相关系数表示两个变量之间的线性相关程度。</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Pearson 相关值</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>%1 表示两个变量之间的线性关系。</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Pearson 相关系数</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman 秩</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Spearman 秩相关系数是衡量两个变量的排序之间统计相关性的非参数度量。它评估两个变量之间的关系能在多大程度上用单调函数描述。</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Spearman 秩相关值</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>%1 表示两个变量之间的单调关系。</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Spearman 秩相关系数</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>欧几里得相似度</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>欧几里得相似度本质上是两个向量之间欧几里得距离的倒数。</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1 本质上是两个向量之间欧几里得距离的倒数。</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>余弦相似度</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>余弦相似度是内积空间中两个非零向量之间相似性的度量。它定义为两者夹角的余弦，也等于将这两个向量都归一化为长度 1 后的内积。</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1 是内积空间中两个非零向量之间相似性的度量。</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>双权中相关（也称 Bicor）是样本之间相似性的度量。它基于中位数而非均值，因此对离群值不太敏感。</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Bicor 相关值</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 是基于中位数的样本相似性度量。</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Jaccard 指数是用于衡量样本集相似性和多样性的统计量。</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Jaccard 相关值</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>%1 是用于衡量样本集相似性和多样性的统计量。</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Jaccard 指数</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>简单匹配系数</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>简单匹配系数是用于衡量样本集相似性和多样性的统计量。它与 Jaccard 相同，只是还计入了共同缺失。</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>简单匹配系数（SMC）</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>数据值</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>最强数据列</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>平均数据值</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>最小数据值</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>最大数据值</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>方差</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>标准差</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>变异系数</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>绝对 %1</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>数据</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>相关值</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>相关 CSV 文件</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>相关 CSV 文件</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>相关 TSV 文件</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>相关 TSV 文件</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>相关 SSV 文件</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>相关 SSV 文件</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>相关 Excel 文件</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>相关 Excel 文件</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>正在计算相关</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>聚类</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>导入列注释</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>导入列注释</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>正在导入列注释</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>正在导入列注释</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>已导入 %n 个注释</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>已导入注释 %1</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>属性</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>归一化</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相关</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>正在构建图</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>正在对列排序</translation>
    </message>
</context>
</TS>
