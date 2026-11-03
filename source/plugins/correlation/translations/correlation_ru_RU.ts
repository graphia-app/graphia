<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Не удалось загрузить {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Загрузка {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Введение</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Корреляционный анализ графа&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>Плагин корреляции строит графы на основе сходства между переменными в наборе данных.&lt;br&gt;&lt;br&gt;Мерой сходства служит {0}. При необходимости входные данные можно масштабировать и нормализовать, после чего по значениям корреляции определяется, создавать ли ребро между узлами, представляющими строки данных.&lt;br&gt;&lt;br&gt;После создания графа рёбра можно отфильтровать с помощью преобразований.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>коэффициент корреляции</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Выбор данных</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Выбор данных — выбор и настройка&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Для вашего набора данных автоматически выбран фрейм данных. Если вы хотите его изменить, выберите ниже новую начальную ячейку. Обратите внимание, что фрейм данных всегда продолжается до правой нижней ячейки набора данных.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Транспонировать</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Транспонирование отражает данные относительно диагонали, превращая строки в столбцы и наоборот.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Тип данных:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Непрерывные</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Дискретные</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Тип данных</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Непрерывные:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Интерпретировать выбранные данные как строки непрерывных данных. Между строками будет использоваться числовая мера корреляции.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Дискретные:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Интерпретировать выбранные данные как строки дискретных значений. Значения могут быть числовыми, но обычно они текстовые. Между строками будет использоваться дискретная мера корреляции.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Считать двоичными</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Если этот параметр включён, алгоритм корреляции считает пустые ячейки и ячейки со значениями &lt;i&gt;0&lt;/i&gt; или &lt;i&gt;false&lt;/i&gt; ложными, а все остальные значения — истинными.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;ВНИМАНИЕ: выбранный фрейм данных, по-видимому, содержит непрерывные данные; если интерпретировать их как дискретные, корреляция может оказаться низкой.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Корреляция</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Корреляция — настройка параметров&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Дополнительно</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>На горизонтальной оси графика ниже значение 1 соответствует идеально коррелирующим строкам, а 0 — отсутствию корреляции. Все абсолютные значения ниже выбранного минимального порога корреляции отбрасываются и не используются для создания рёбер. По умолчанию добавляется преобразование, которое удаляет рёбра для всех абсолютных значений ниже заданного начального порога корреляции.&lt;br&gt;&lt;br&gt;Для более тонкой настройки параметров отметьте вариант &lt;b&gt;Дополнительно&lt;/b&gt; справа вверху.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Алгоритм:</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Жаккар</translation>
    </message>
    <message>
        <source>SMC</source>
        <translation>SMC</translation>
    </message>
    <message>
        <source>Correlation Algorithm</source>
        <translation>Алгоритм корреляции</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Пирсон</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Ранговая Спирмена</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Евклидово сходство</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Косинусное сходство</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Знак:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Положительная</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Отрицательная</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Обе</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Знак</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>По умолчанию при построении графа учитываются только положительные корреляции. Почти всегда это правильная настройка, но для некоторых источников данных имеет смысл учитывать знак корреляции. Обратите внимание, что не все алгоритмы корреляции способны находить отрицательные связи.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Импутация:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Константа</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Интерполяция по строке</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Среднее по столбцу</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Импутация</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Константа:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Заменить все пропущенные значения константой.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Интерполяция по строке:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Линейно интерполировать пропущенные значения по ближайшим окружающим значениям в строке и их относительным позициям. Если доступно только одно окружающее значение, будет использовано оно. Если строка пуста, значение будет равно нулю.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Среднее по столбцу:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Заменить пропущенные значения средним значением их столбца без учёта пропущенных значений.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Ограничение:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Нет</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Винзоризация</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>процентиль</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Ограничение</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Ограничить максимальное значение таблицы данных заданной константой.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Винзоризация:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Ограничить максимальное значение данных значением заданного процентиля, отдельно для каждой строки. Например, если выбран 95-й процентиль, верхние 5% значений будут заменены значением 95-го процентиля.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Масштабирование:</translation>
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
        <translation>Типы масштабирования</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Вычислить логарифм &lt;i&gt;x&lt;/i&gt; + ε по основанию &lt;i&gt;b&lt;/i&gt;, где &lt;i&gt;x&lt;/i&gt; — входные данные, а ε — очень малая константа.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Возвести &lt;i&gt;b&lt;/i&gt; в степень &lt;i&gt;x&lt;/i&gt;, где &lt;i&gt;x&lt;/i&gt; — входные данные.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Вычислить арксинус &lt;i&gt;x&lt;/i&gt;, где &lt;i&gt;x&lt;/i&gt; — входные данные. Это полезно, когда требуется логарифмическое преобразование, но в наборе данных есть отрицательные числа или нули.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Нормализация:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Мин./макс.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Стандартизация</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Единичное масштабирование</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Квантильная</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Типы нормализации</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Мин./макс.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Масштабировать данные так, чтобы их значения лежали в диапазоне [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Это полезно, если столбцы набора данных имеют разные масштабы или единицы измерения. Примечание: если все элементы столбца имеют одинаковое значение, они будут масштабированы до 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Среднее:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Аналогично масштабированию мин./макс., но полученные значения центрируются относительно среднего по столбцу: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Примечание: если все элементы столбца имеют одинаковое значение, они будут масштабированы до 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Стандартизация:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Этот метод, также известный как нормализация по Z-оценке, центрирует значения относительно среднего и масштабирует их на стандартное отклонение: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Единичное масштабирование:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Значения делятся на евклидову длину вектора, образованного столбцом данных: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Квантильная:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Нормализовать данные так, чтобы столбцы имели одинаковые распределения.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Функция softmax превращает каждый столбец данных в распределение вероятностей: каждый элемент возводится в экспоненту, а затем делится на сумму. В итоге наибольшие значения в каждом столбце усиливаются.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Фильтрация:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Порог</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Фильтрация</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Порог:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Простой фильтр, отбрасывающий корреляции ниже выбранного порога.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k ближайших соседей:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Отслеживать корреляции между парами строк, отбрасывая те, что не входят в &lt;i&gt;k&lt;/i&gt; наибольших корреляций для каждой строки. Это наиболее полезно для наборов данных с очень высокой корреляцией.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Минимум:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Минимальное значение корреляции</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Минимальное значение корреляции, выше которого в графе создаётся ребро. Чем ниже минимальное значение, тем выше требования к вычислениям и памяти.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>Максимум k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Максимальное &lt;i&gt;k&lt;/i&gt; корреляции</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Максимальное число корреляций, сохраняемых для каждой строки данных; в итоге оно ограничивает число рёбер, создаваемых в графе. Чем больше максимальное значение, тем выше требования к вычислениям и памяти.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Начальный:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>Начальное k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Порог корреляции</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; корреляции</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Начальный порог корреляции определяет размер получаемого графа. Чем ниже значение, тем меньше рёбер отфильтровывается и тем больше граф. Это значение можно изменить позже, после создания графа.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Начальное значение k определяет размер получаемого графа. Чем выше значение, тем меньше рёбер отфильтровывается и тем больше граф. Это значение можно изменить позже, после создания графа.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Обработка…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Пустой граф</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Шаблон&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Шаблон — это именованный набор преобразований и визуализаций, который можно применить к графу после его создания. Выберите шаблон для применения. &lt;b&gt;Примечание:&lt;/b&gt; заданные здесь преобразования всегда добавляются безусловно, независимо от способа применения, заданного в шаблоне.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Сделать по умолчанию</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Сводка</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Сводка&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Граф будет создан со следующими параметрами:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Транспонировано&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Фрейм данных: [ Столбец: {0} Строка: {1} Ширина: {2} Высота: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Минимальное значение корреляции: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Начальный порог корреляции: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Максимальное значение k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Начальное значение k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Мера корреляции непрерывных данных: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Знак корреляции: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Импутация: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Импутация: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Ограничение: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Ограничение: {0} ({1}-й процентиль)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Ограничение: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Масштабирование: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Нормализация: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Мера корреляции дискретных данных: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Шаблон: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Оценка размера графа до преобразований: {0}{3} узлов{2}, {1}{4} рёбер{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Увеличение минимального значения корреляции обычно уменьшает размер графа.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Уменьшение максимального &lt;i&gt;k&lt;/i&gt; обычно уменьшает размер графа.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ВНИМАНИЕ: это очень большой граф, который может исчерпать системные ресурсы и привести к нестабильной работе или зависаниям. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}ВНИМАНИЕ: скорее всего, созданный граф будет пустым.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 столбец</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n столбец</numerusform>
            <numerusform>%n столбца</numerusform>
            <numerusform>%n столбцов</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…и ещё %1</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Увеличьте, чтобы увидеть сведения о столбцах</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Увеличьте, чтобы увидеть названия столбцов</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n уникальное значение</numerusform>
            <numerusform>%n уникальных значения</numerusform>
            <numerusform>%n уникальных значений</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Среднее значение %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Среднее значение выделения</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Медиана %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Медиана выделения</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Средняя гистограмма %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Средняя гистограмма выделения</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Отклонение</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 сверху</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 снизу</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Ст. откл.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Ст. ошибка</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 и ещё %3…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>Расширение файла %1 указывает на тип «%2», однако определить его содержимое не удалось.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>Расширение файла %1 указывает на тип «%2», однако его содержимое похоже на файл «%3».</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Корреляция</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Создаёт граф, в котором узлы представляют строки данных, а рёбра — корреляции между этими строками.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>Значение данных — параметризованный атрибут, позволяющий ссылаться на определённый столбец коррелируемых данных.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Название столбца данных с наибольшим абсолютным значением в строке данных, связанной с узлом.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>Среднее значение данных — среднее значений, связанных с узлом.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>Минимальное значение данных — наименьшее значение, связанное с узлом.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>Максимальное значение данных — наибольшее значение, связанное с узлом.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 — мера разброса значений, связанных с узлом. Определяется как ∑(&lt;i&gt;x&lt;/i&gt;-µ)², где &lt;i&gt;x&lt;/i&gt; — значение, а µ — среднее.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Дисперсия</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>%1 — мера разброса значений, связанных с узлом. Определяется как √∑(&lt;i&gt;x&lt;/i&gt;-µ)², где &lt;i&gt;x&lt;/i&gt; — значение, а µ — среднее.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>%1 — мера разброса значений, связанных с узлом. Определяется как стандартное отклонение, делённое на среднее.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Коэффициент вариации</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>В данных плагина отсутствует ключ «%1».</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Фрейм данных: [ Столбец: %1 Строка: %2 Ширина: %3 Высота: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Транспонировано</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Мера корреляции непрерывных данных: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Положительная</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Отрицательная</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Обе</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Знак корреляции: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Мера корреляции дискретных данных: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Минимальное значение корреляции: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Максимальное значение k: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Константа (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Среднее по столбцу</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Интерполяция по строке</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Импутация: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Ограничение: константа (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Ограничение: винзоризация (%1-й процентиль)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Масштабирование: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Масштабирование: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Масштабирование: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Масштабирование: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Масштабирование: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Нормализация: мин./макс.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Нормализация: квантильная</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Нормализация: среднее</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Нормализация: стандартизация</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Нормализация: единичное масштабирование</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Нормализация: масштабирование softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Импорт аннотаций столбцов</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Нет подходящих ключей</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Не удалось загрузить {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Загрузка {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Введение</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Импорт аннотаций&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Данные аннотаций можно импортировать из внешнего источника и наложить на график. Можно использовать любой из поддерживаемых табличных форматов. После импорта новые аннотации можно сортировать и группировать обычным образом.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Выбор ключей</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Выбор ключей&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Чтобы импортировать данные аннотаций, нужно указать строку входных данных, соответствующую названиям столбцов имеющихся данных. По ней столбцы таблицы сопоставляются со столбцами графика. Выберите нужную строку ниже. Или нажмите &lt;i&gt;{0}&lt;/i&gt;, и наилучшее совпадение будет найдено автоматически.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Автоопределение</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>В выбранной строке &lt;b&gt;{0}&lt;/b&gt; есть повторяющиеся значения, некоторые столбцы будут пропущены!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Найдено совпадение {0}%</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Совпадений не найдено</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Выбор аннотаций</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Выбор аннотаций&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Выберите нужные аннотации в списке справа. Чтобы выбрать несколько аннотаций, удерживайте &lt;i&gt;Ctrl&lt;/i&gt;; чтобы выбрать диапазон, удерживайте &lt;i&gt;Shift&lt;/i&gt; и щёлкайте по нужным пунктам.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Заменить существующие аннотации</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Выделить всё</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Снять выделение</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Сводка</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Сводка&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Определение ключа…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Следующие аннотации будут импортированы с использованием строки &lt;b&gt;{0}{1}&lt;/b&gt; в качестве ключа:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(содержит повторяющиеся значения)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Аннотации нельзя импортировать, пока не выбран ключ.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Определение ключа</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>&amp;Интерфейс рядом с графиком</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Показывать &amp;названия столбцов</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Найти интересующие строки</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Выбрать видимые аннотации столбцов</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Линии сетки</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Включать ноль в ось Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Показывать выбросы</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Сбросить масштаб</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Показать все столбцы</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Нет</translation>
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
        <translation>Центрирование по среднему</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Масштабирование к единичной дисперсии</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Масштабирование Парето</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Планки погрешностей</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Область</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Стандартная ошибка</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Отдельные линии</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Линия среднего</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Линия медианы</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Средняя гистограмма</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Межквартильный размах</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Группировать по аннотации</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Импортировать аннотации из таблицы…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-график-корреляции</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Сохранить как &amp;изображение…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Задать подпись оси X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Задать подпись оси Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Подпись оси X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Введите подпись оси X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Подпись оси Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Введите подпись оси Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;График</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Подписи осей</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Масштабирование</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>По атрибуту</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Усреднение</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Разброс</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Сортировать столбцы по</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Раскрашивать группы по</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Сохранить как изображения по…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Строка</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Естественный порядок</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Название столбца</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Значение данных</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Иерархическая кластеризация</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Щёлкните по столбцу или аннотации столбца, чтобы выделить его. Чтобы выделить несколько столбцов, щёлкайте, удерживая &lt;i&gt;Ctrl&lt;/i&gt;.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Зафиксировать порядок сортировки по значениям данных</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Сохранить график как изображение</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Изображение PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Изображение JPEG (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>Документ PDF (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Показать все аннотации</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Скрыть все аннотации</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Относительный</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Абсолютный</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Взвешивание выделения</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Строки выбираются по взвешенному сравнению суммы выделенных столбцов с невыделенными. Взвешивание выделенных столбцов (режим &lt;i&gt;Абсолютный&lt;/i&gt;) находит строки с пиками в этих столбцах независимо от невыделенных. Напротив, (отрицательное) взвешивание невыделенных столбцов (режим &lt;i&gt;Относительный&lt;/i&gt;) находит строки, имеющие значимые значения именно в выделенных столбцах.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}-й</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}-й</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}-й</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}-й</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Процентиль</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Число строк для выбора и ранжирования.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Выбрать заново</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Межквартильный размах %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Межквартильный размах</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Сохранение изображений графика</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Пирсон</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Коэффициент корреляции Пирсона показывает степень линейной корреляции между двумя переменными.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Значение корреляции Пирсона</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>%1 показывает степень линейной связи между двумя переменными.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Коэффициент корреляции Пирсона</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Ранговая Спирмена</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Коэффициент ранговой корреляции Спирмена — непараметрическая мера статистической зависимости между рангами двух переменных. Он показывает, насколько хорошо связь между двумя переменными описывается монотонной функцией.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Значение ранговой корреляции Спирмена</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>%1 показывает степень монотонной связи между двумя переменными.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Коэффициент ранговой корреляции Спирмена</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Евклидово сходство</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Евклидово сходство — по сути величина, обратная евклидову расстоянию между двумя векторами.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>%1 — по сути величина, обратная евклидову расстоянию между двумя векторами.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Косинусное сходство</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Косинусное сходство — мера сходства между двумя ненулевыми векторами пространства со скалярным произведением. Оно определяется как косинус угла между ними, что равно скалярному произведению этих векторов, нормализованных к длине 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>%1 — мера сходства между двумя ненулевыми векторами пространства со скалярным произведением.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Бивесовая срединная корреляция (Bicor) — мера сходства между выборками. Она основана на медиане, а не на среднем, поэтому менее чувствительна к выбросам.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Значение корреляции Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>%1 — основанная на медиане мера сходства между выборками.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Жаккар</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Индекс Жаккара — статистика для оценки сходства и разнообразия наборов выборок.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Значение корреляции Жаккара</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>%1 — статистика для оценки сходства и разнообразия наборов выборок.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Индекс Жаккара</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Простой коэффициент совпадения</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Простой коэффициент совпадения — статистика для оценки сходства и разнообразия наборов выборок. Он совпадает с индексом Жаккара, но учитывает и совместное отсутствие.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Простой коэффициент совпадения (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Значение данных</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Сильнейший столбец данных</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Среднее значение данных</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Минимальное значение данных</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Максимальное значение данных</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Дисперсия</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Коэффициент вариации</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>Абсолютное %1</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Данные</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Значения корреляции</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Файл CSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Файлы CSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Файл TSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Файлы TSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Файл SSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Файлы SSV для корреляции</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Файл Excel для корреляции</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Файлы Excel для корреляции</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Вычисление корреляции</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Кластеризация</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Импорт аннотаций столбцов</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Импорт аннотации столбца</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Импорт аннотаций столбцов</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Импорт аннотации столбца</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>Импортирована %n аннотация</numerusform>
            <numerusform>Импортированы %n аннотации</numerusform>
            <numerusform>Импортировано %n аннотаций</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Аннотация %1 импортирована</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Нормализация</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Корреляция</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Построение графа</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Сортировка столбцов</translation>
    </message>
</context>
</TS>
