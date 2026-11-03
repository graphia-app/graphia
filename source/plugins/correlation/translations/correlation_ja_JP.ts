<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>{0} を読み込めませんでした。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} を読み込み中…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>はじめに</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;相関グラフ解析&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>相関プラグインは、データセット内の変数間の類似度に基づいてグラフを作成します。&lt;br&gt;&lt;br&gt;この類似度の指標となるのが{0}です。指定すれば入力データをスケーリングおよび正規化でき、その後、相関スコアを使ってデータの行を表すノード間にエッジを作成するかどうかを決定します。&lt;br&gt;&lt;br&gt;グラフの作成後は、変換を使ってエッジをフィルタリングできます。</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>相関係数</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>データの選択</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;データの選択 - 選択と調整&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>データセットのデータフレームが自動的に選択されています。調整したい場合は、以下で新しい開始セルを選択してください。データフレームは常にデータセットの右下端のセルまで広がります。</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>転置</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>転置すると、データが対角線を軸に反転し、行が列に、列が行になります。</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>データの種類:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>連続</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>離散</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>データの種類</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;連続:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>選択したデータを連続データの行として解釈します。行間には数値の相関尺度が使われます。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;離散:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>選択したデータを離散値の行として解釈します。値は数値でもかまいませんが、通常はテキストです。行間には離散値の相関尺度が使われます。</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>2 値として扱う</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>有効にすると、相関アルゴリズムでは空のセルと値が &lt;i&gt;0&lt;/i&gt; または &lt;i&gt;false&lt;/i&gt; のセルを偽、それ以外のすべての値を真とみなします。</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;警告: 選択したデータフレームには連続データが含まれているようです。離散データとして解釈すると、相関が低くなる可能性があります。&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相関</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;相関 - パラメーターの調整&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>詳細設定</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>下のプロットの横軸では、1 は完全に相関した行を、0 は相関がないことを表します。選択した最小相関しきい値を下回る絶対値はすべて破棄され、エッジの作成には使われません。デフォルトでは、指定した初期相関しきい値を下回るすべての絶対値のエッジを削除する変換が追加されます。&lt;br&gt;&lt;br&gt;使用するパラメーターをより細かく制御するには、右上の&lt;b&gt;詳細設定&lt;/b&gt;オプションをオンにしてください。</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>アルゴリズム:</translation>
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
        <translation>相関アルゴリズム</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman 順位</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>ユークリッド類似度</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>コサイン類似度</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>極性:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>正</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>負</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>両方</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>極性</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>デフォルトでは、グラフの作成時に正の相関のみが考慮されます。ほとんどの場合はこれが適切な設定ですが、データソースによっては相関の極性を考慮したほうがよい場合もあります。なお、すべての相関アルゴリズムが負の関係を検出できるわけではありません。</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>補完:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>定数</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>行内補間</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>列の平均</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>補完</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;定数:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>すべての欠損値を定数で置き換えます。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;行内補間:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>行内で最も近い周囲の値とその相対位置を使って、欠損値を線形補間します。周囲の値が 1 つしかない場合は、その値と同じになります。行が空の場合、値はゼロになります。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;列の平均:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>欠損値を、欠損値を除いたその列の平均値で置き換えます。</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>クリッピング:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>なし</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>ウィンソライズ</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>パーセンタイル</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>クリッピング</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>データテーブルの最大値を指定した定数に制限します。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;ウィンソライズ:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>行ごとに、データの最大値を指定したパーセンタイルの値に制限します。たとえば 95 パーセンタイルを選んだ場合、上位 5% の値は 95 パーセンタイルの値に置き換えられます。</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>スケーリング:</translation>
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
        <translation>スケーリングの種類</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>&lt;i&gt;b&lt;/i&gt; を底とする &lt;i&gt;x&lt;/i&gt; + ε の対数を計算します。&lt;i&gt;x&lt;/i&gt; は入力データ、ε はごく小さな定数です。</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>&lt;i&gt;b&lt;/i&gt; の &lt;i&gt;x&lt;/i&gt; 乗を計算します。&lt;i&gt;x&lt;/i&gt; は入力データです。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>&lt;i&gt;x&lt;/i&gt; の逆正弦を計算します。&lt;i&gt;x&lt;/i&gt; は入力データです。対数変換が必要なのにデータセットに負の数やゼロが含まれている場合に便利です。</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>正規化:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>最小/最大</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>平均</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>標準化</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>単位スケーリング</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>分位</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>正規化の種類</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;最小/最大:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>値が [0, 1] の範囲に収まるようにデータを再スケーリングします: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;))。データセットの列ごとにスケールや単位が異なる場合に便利です。注意: 列内のすべての要素が同じ値の場合、値は 0.0 に再スケーリングされます。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;平均:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>最小/最大スケーリングと似ていますが、結果の値は列の平均を中心とします: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;))。注意: 列内のすべての要素が同じ値の場合、値は 0.0 に再スケーリングされます。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;標準化:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Z スコア正規化とも呼ばれ、値を平均を中心に配置し、標準偏差でスケーリングします: (&lt;i&gt;x&lt;/i&gt;-µ)/σ。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;単位スケーリング:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>値を、データの列が形成するベクトルのユークリッド長で割ります: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;分位:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>列の分布が等しくなるようにデータを正規化します。</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Softmax 関数は、各要素の指数を取り、その合計で割って正規化することで、データの各列を確率分布に変換します。結果として、各列の大きな値が強調されます。</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>フィルタリング:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>しきい値</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>フィルタリング中</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;しきい値:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>選択したしきい値を下回る相関を破棄するシンプルなフィルターです。</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k 近傍:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>行のペア間の相関を追跡し、各行の上位 &lt;i&gt;k&lt;/i&gt; 個に入らない相関を破棄します。非常に強く相関したデータセットで特に役立ちます。</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>最小:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>最小相関値</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>グラフにエッジを作成する最小の相関値です。最小値を低くすると、必要な計算量とメモリが増えます。</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>最大 k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>最大相関 &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>データ行ごとに保持する相関の最大数で、最終的にグラフに作成されるエッジの数を制限します。最大値を大きくすると、必要な計算量とメモリが増えます。</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>初期値:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>初期 k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>相関しきい値</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>相関 &lt;i&gt;k&lt;/i&gt;</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>初期相関しきい値は、作成されるグラフのサイズを決めます。値が低いほど除外されるエッジが少なくなり、グラフは大きくなります。この値はグラフの作成後に変更できます。</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>初期 k 値は、作成されるグラフのサイズを決めます。値が高いほど除外されるエッジが少なくなり、グラフは大きくなります。この値はグラフの作成後に変更できます。</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>処理中…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>空のグラフ</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>テンプレート</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;テンプレート&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>テンプレートは、名前を付けた変換と可視化のセットで、グラフの作成後に適用できます。適用するテンプレートを選択してください。&lt;b&gt;注意:&lt;/b&gt; ここで適用する変換は、テンプレート固有の適用方法にかかわらず、常に無条件で追加されます。</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>名前:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>デフォルトに設定</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;概要&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>次のパラメーターでグラフを作成します:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>転置済み&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>データフレーム: [ 列: {0} 行: {1} 幅: {2} 高さ: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>最小相関値: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>初期相関しきい値: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>最大 k 値: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>初期 k 値: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>連続データの相関尺度: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>相関の極性: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>補完: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>補完: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>クリッピング: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>クリッピング: {0} ({1} パーセンタイル)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>クリッピング: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>スケーリング: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>正規化: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>離散データの相関尺度: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>テンプレート: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>変換前の推定グラフサイズ: ノード {0}{3}{2}、エッジ {1}{4}{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>最小相関値を上げると、通常はグラフのサイズが小さくなります。</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>最大 &lt;i&gt;k&lt;/i&gt; を下げると、通常はグラフのサイズが小さくなります。</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}警告: これは非常に大きなグラフで、システムリソースを使い果たし、動作が不安定になったりフリーズしたりするおそれがあります。{1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}警告: 生成されるグラフはおそらく空になります。{1}</translation>
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
        <translation>…ほか %1 件</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>サイズを変更すると列情報が表示されます</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>サイズを変更すると列名が表示されます</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>一意な値 %n 個</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>%1 の平均: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>選択範囲の平均</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>%1 の中央値: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>選択範囲の中央値</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>%1 の平均ヒストグラム: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>選択範囲の平均ヒストグラム</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>ばらつき</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 上</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 下</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>標準偏差</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>標準誤差</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1、%2 ほか %3 件…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>%1 の拡張子は「%2」であることを示していますが、内容を特定できませんでした。</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>%1 の拡張子は「%2」であることを示していますが、内容は「%3」ファイルに似ています。</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相関</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>ノードがデータの行を、エッジがそれらの行の間の相関を表すグラフを作成します。</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>データ値は、相関データ内の特定の列を参照できるパラメーター付きの属性です。</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>ノードに関連付けられたデータ行で、絶対値が最大のデータ列の名前。</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>平均データ値は、ノードに関連付けられた値の平均です。</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>最小データ値は、ノードに関連付けられた値の最小値です。</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>最大データ値は、ノードに関連付けられた値の最大値です。</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1は、ノードに関連付けられた値のばらつきの尺度です。∑(&lt;i&gt;x&lt;/i&gt;-µ)² で定義されます。&lt;i&gt;x&lt;/i&gt; は値、µ は平均です。</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>分散</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1は、ノードに関連付けられた値のばらつきの尺度です。√∑(&lt;i&gt;x&lt;/i&gt;-µ)² で定義されます。&lt;i&gt;x&lt;/i&gt; は値、µ は平均です。</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>標準偏差</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>%1は、ノードに関連付けられた値のばらつきの尺度です。標準偏差を平均で割った値として定義されます。</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>変動係数</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>プラグインデータにキー「%1」がありません。</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>データフレーム: [ 列: %1 行: %2 幅: %3 高さ: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>転置済み</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>連続データの相関尺度: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>正</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>負</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>両方</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>相関の極性: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>離散データの相関尺度: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>最小相関値: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>最大 k 値: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>定数 (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>列の平均</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>行内補間</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>補完: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>クリッピング: 定数 (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>クリッピング: ウィンソライズ (%1 パーセンタイル)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>スケーリング: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>スケーリング: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>スケーリング: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>スケーリング: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>スケーリング: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>正規化: 最小/最大</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>正規化: 分位</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>正規化: 平均</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>正規化: 標準化</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>正規化: 単位スケーリング</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>正規化: Softmax スケーリング</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>列の注釈をインポート</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>適切なキーがありません</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>{0} を読み込めませんでした。</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>{0} を読み込み中…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>閉じる</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>キャンセル</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>はじめに</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;注釈のインポート&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>注釈データは外部ソースからインポートして、プロットに重ねて表示できます。サポートされている表形式データのどの形式でも使用できます。インポートした新しい注釈は、通常の方法で並べ替えたりグループ化したりできます。</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>キーの選択</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;キーの選択&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>注釈データをインポートするには、既存データの列名に対応する入力データの行を指定する必要があります。これを使ってテーブルの列とプロットの列を対応付けます。以下で正しい行を選択してください。または、&lt;i&gt;{0}&lt;/i&gt; をクリックすると最適な一致が自動的に特定されます。</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>自動検出</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>選択した行 &lt;b&gt;{0}&lt;/b&gt; には重複する値があるため、一部の列は無視されます！</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>{0}% 一致しました</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>一致するものが見つかりません</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>注釈の選択</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;注釈の選択&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>右側のリストから必要な注釈を選択してください。&lt;i&gt;Ctrl&lt;/i&gt; キーを押しながらクリックすると複数の注釈を、&lt;i&gt;Shift&lt;/i&gt; キーを押しながらクリックすると範囲を選択できます。</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>既存の注釈を置き換える</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>すべて選択</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>選択を解除</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>概要</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;概要&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>キーを検出中…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>次の注釈を、行 &lt;b&gt;{0}{1}&lt;/b&gt; をキーとしてインポートします:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(重複する値があります)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;キーを選択しないと注釈をインポートできません。&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>キーを検出中</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>UI を横に並べて表示(&amp;S)</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>列名を表示(&amp;C)</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>注目する行を検索</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>表示する列の注釈を選択</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>グリッド線</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>凡例</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Y 軸にゼロを含める</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>外れ値を表示</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>ズームをリセット</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>すべての列を表示</translation>
    </message>
    <message>
        <source>None</source>
        <translation>なし</translation>
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
        <translation>平均センタリング</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>単位分散スケーリング</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>パレートスケーリング</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>エラーバー</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>領域</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>標準偏差</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>標準誤差</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>個別の線</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>平均線</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>中央値線</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>平均ヒストグラム</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>四分位範囲</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>注釈でグループ化</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>テーブルから注釈をインポート…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-相関プロット</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>画像として保存(&amp;I)…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>X 軸ラベルを設定…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Y 軸ラベルを設定…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>X 軸ラベル</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>X 軸ラベルを入力してください:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Y 軸ラベル</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Y 軸ラベルを入力してください:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>プロット(&amp;P)</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>軸ラベル</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>スケーリング</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>属性別</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>平均化</translation>
    </message>
    <message>
        <source>All</source>
        <translation>すべて</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>ばらつき</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>列の並べ替え基準</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>グループの色分け基準</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>画像として保存 (分割基準)…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>行</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>本来の順序</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>列名</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>データ値</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>階層クラスタリング</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>列または列の注釈をクリックすると選択できます。&lt;i&gt;Ctrl&lt;/i&gt; キーを押しながらクリックすると複数の列を選択できます。</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>データ値による並べ替え順を固定</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>プロットを画像として保存</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>PNG 画像 (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>JPEG 画像 (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>PDF ドキュメント (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>すべての注釈を表示</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>すべての注釈を非表示</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>相対</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>絶対</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>選択の重み付け</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>行は、選択した列の合計を選択していない列と比較して重み付けすることで選ばれます。選択した列を重み付けする方法 (&lt;i&gt;絶対&lt;/i&gt;) では、選択していない列に関係なく、選択した列にピークがある行が見つかりやすくなります。一方、選択していない列を (負に) 重み付けする方法 (&lt;i&gt;相対&lt;/i&gt;) では、まさに選択した列で有意な値を持つ行が見つかりやすくなります。</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>パーセンタイル</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>選択して順位付けする行の数。</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>再選択</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>%1 の四分位範囲</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>四分位範囲</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>プロット画像を保存中</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Pearson の相関係数は、2 つの変数間の線形相関を示します。</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Pearson 相関値</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>%1は、2 つの変数間の線形関係を示します。</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Pearson の相関係数</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Spearman 順位</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Spearman の順位相関係数は、2 つの変数の順位間の統計的な依存関係を表すノンパラメトリックな尺度です。2 つの変数間の関係が単調関数でどの程度うまく表せるかを評価します。</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Spearman 順位相関値</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>%1は、2 つの変数間の単調な関係を示します。</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Spearman の順位相関係数</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>ユークリッド類似度</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>ユークリッド類似度は、本質的には 2 つのベクトル間のユークリッド距離の逆数です。</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1は、本質的には 2 つのベクトル間のユークリッド距離の逆数です。</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>コサイン類似度</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>コサイン類似度は、内積空間における 2 つの非ゼロベクトル間の類似度の尺度です。2 つのベクトルのなす角の余弦として定義され、これは長さ 1 に正規化した同じベクトル同士の内積と等しくなります。</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1は、内積空間における 2 つの非ゼロベクトル間の類似度の尺度です。</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Biweight Midcorrelation (Bicor とも呼ばれます) は、サンプル間の類似度の尺度です。平均ではなく中央値に基づくため、外れ値の影響を受けにくくなっています。</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Bicor 相関値</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1は、中央値に基づくサンプル間の類似度の尺度です。</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Jaccard 指数は、サンプル集合の類似性と多様性を測るための統計量です。</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Jaccard 相関値</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>%1は、サンプル集合の類似性と多様性を測るための統計量です。</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Jaccard 指数</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>単純一致係数</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>単純一致係数は、サンプル集合の類似性と多様性を測るための統計量です。Jaccard と同じですが、両方に存在しない場合も数に入れる点が異なります。</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>単純一致係数 (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>データ値</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>最も強いデータ列</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>平均データ値</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>最小データ値</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>最大データ値</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>分散</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>標準偏差</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>変動係数</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>%1 の絶対値</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>データ</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>相関値</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>相関 CSV ファイル</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>相関 CSV ファイル</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>相関 TSV ファイル</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>相関 TSV ファイル</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>相関 SSV ファイル</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>相関 SSV ファイル</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>相関 Excel ファイル</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>相関 Excel ファイル</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>相関を計算中</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>クラスタリング</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>列の注釈をインポート</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>列の注釈をインポート</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>列の注釈をインポート中</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>列の注釈をインポート中</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>%n 個の注釈をインポートしました</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>注釈 %1 をインポートしました</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>属性</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>正規化</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>相関</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>グラフを構築中</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>列を並べ替え中</translation>
    </message>
</context>
</TS>
