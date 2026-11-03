<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="el_GR" sourcelanguage="en_GB">
<context>
    <name>CorrelationParameters</name>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Αποτυχία φόρτωσης του {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Φόρτωση {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Κλείσιμο</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Ακύρωση</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Εισαγωγή</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation Graph Analysis&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Ανάλυση γράφου συσχέτισης&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>The correlation plugin creates graphs based on the similarity between variables in a dataset.&lt;br&gt;&lt;br&gt;A {0} provides this similarity metric. If specified, the input data can be scaled and normalised, after which correlation scores are used to determine whether or not an edge is created between the nodes representing rows of data.&lt;br&gt;&lt;br&gt;The edges may be filtered using transforms once the graph has been created.</source>
        <translation>Το πρόσθετο συσχέτισης δημιουργεί γράφους με βάση την ομοιότητα μεταξύ μεταβλητών σε ένα σύνολο δεδομένων.&lt;br&gt;&lt;br&gt;Ένας {0} παρέχει αυτό το μέτρο ομοιότητας. Αν καθοριστεί, τα δεδομένα εισόδου μπορούν να κλιμακωθούν και να κανονικοποιηθούν, και στη συνέχεια οι τιμές συσχέτισης χρησιμοποιούνται για να αποφασιστεί αν θα δημιουργηθεί ακμή μεταξύ των κόμβων που αντιπροσωπεύουν γραμμές δεδομένων.&lt;br&gt;&lt;br&gt;Οι ακμές μπορούν να φιλτραριστούν με μετασχηματισμούς αφού δημιουργηθεί ο γράφος.</translation>
    </message>
    <message>
        <source>correlation coefficient</source>
        <translation>συντελεστής συσχέτισης</translation>
    </message>
    <message>
        <source>Data Selection</source>
        <translation>Επιλογή δεδομένων</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Data Selection - Select and Adjust&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Επιλογή δεδομένων - Επιλογή και προσαρμογή&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A dataframe has been automatically selected for your dataset. If you would like to adjust it, select the new starting cell below. Note that the dataframe always extends to the bottom rightmost cell of the dataset.</source>
        <translation>Ένα πλαίσιο δεδομένων επιλέχθηκε αυτόματα για το σύνολο δεδομένων σας. Αν θέλετε να το προσαρμόσετε, επιλέξτε παρακάτω το νέο αρχικό κελί. Σημειώστε ότι το πλαίσιο δεδομένων εκτείνεται πάντα έως το κάτω δεξιό κελί του συνόλου δεδομένων.</translation>
    </message>
    <message>
        <source>Transpose</source>
        <translation>Αντιμετάθεση</translation>
    </message>
    <message>
        <source>Transpose will flip the data over its diagonal. Moving rows to columns and vice versa.</source>
        <translation>Η αντιμετάθεση αναστρέφει τα δεδομένα ως προς τη διαγώνιό τους, μετατρέποντας τις γραμμές σε στήλες και αντίστροφα.</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Τύπος δεδομένων:</translation>
    </message>
    <message>
        <source>Continuous</source>
        <translation>Συνεχή</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Διακριτά</translation>
    </message>
    <message>
        <source>Data Type</source>
        <translation>Τύπος δεδομένων</translation>
    </message>
    <message>
        <source>&lt;b&gt;Continuous:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Συνεχή:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of continuous data. A numerical correlation measure will be used between rows.</source>
        <translation>Ερμηνεία των επιλεγμένων δεδομένων ως γραμμών συνεχών δεδομένων. Μεταξύ των γραμμών θα χρησιμοποιηθεί αριθμητικό μέτρο συσχέτισης.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Discrete:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Διακριτά:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Interpret the selected data as rows of discrete values. The values may be numerical, but in the normal case they will be textual. A discrete correlation measure will be used between rows.</source>
        <translation>Ερμηνεία των επιλεγμένων δεδομένων ως γραμμών διακριτών τιμών. Οι τιμές μπορεί να είναι αριθμητικές, αλλά συνήθως είναι κείμενο. Μεταξύ των γραμμών θα χρησιμοποιηθεί διακριτό μέτρο συσχέτισης.</translation>
    </message>
    <message>
        <source>Treat As Binary</source>
        <translation>Χειρισμός ως δυαδικά</translation>
    </message>
    <message>
        <source>If this is enabled, for the purposes of the correlation algorithm empty cells or cells containing the values &lt;i&gt;0&lt;/i&gt; or &lt;i&gt;false&lt;/i&gt; are considered to be false, while all other values are considered to be true.</source>
        <translation>Αν αυτό είναι ενεργοποιημένο, για τους σκοπούς του αλγορίθμου συσχέτισης τα κενά κελιά ή τα κελιά με τιμές &lt;i&gt;0&lt;/i&gt; ή &lt;i&gt;false&lt;/i&gt; θεωρούνται ψευδή, ενώ όλες οι άλλες τιμές θεωρούνται αληθείς.</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;WARNING: the selected dataframe appears to contain continuous data; interpreting it as discrete may result in only a low level of correlation.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;ΠΡΟΕΙΔΟΠΟΙΗΣΗ: το επιλεγμένο πλαίσιο δεδομένων φαίνεται να περιέχει συνεχή δεδομένα· η ερμηνεία τους ως διακριτών μπορεί να οδηγήσει σε χαμηλό μόνο επίπεδο συσχέτισης.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Συσχέτιση</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Correlation - Adjust Parameters&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Συσχέτιση - Προσαρμογή παραμέτρων&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation>Για προχωρημένους</translation>
    </message>
    <message>
        <source>On the horizontal axis of the plot below, values of 1 represent perfectly correlated rows whereas 0 indicates no correlation. All absolute values below the selected minimum correlation threshold are discarded and will not be used to create edges. By default a transform is added which will remove edges for all absolute values below the specified initial correlation threshold.&lt;br&gt;&lt;br&gt;For more control over the parameters used, check the &lt;b&gt;Advanced&lt;/b&gt; option on the top right.</source>
        <translation>Στον οριζόντιο άξονα του παρακάτω γραφήματος, οι τιμές 1 αντιπροσωπεύουν τέλεια συσχετισμένες γραμμές, ενώ το 0 δηλώνει απουσία συσχέτισης. Όλες οι απόλυτες τιμές κάτω από το επιλεγμένο ελάχιστο κατώφλι συσχέτισης απορρίπτονται και δεν θα χρησιμοποιηθούν για τη δημιουργία ακμών. Από προεπιλογή προστίθεται ένας μετασχηματισμός που αφαιρεί τις ακμές για όλες τις απόλυτες τιμές κάτω από το καθορισμένο αρχικό κατώφλι συσχέτισης.&lt;br&gt;&lt;br&gt;Για περισσότερο έλεγχο στις παραμέτρους, επιλέξτε &lt;b&gt;Για προχωρημένους&lt;/b&gt; πάνω δεξιά.</translation>
    </message>
    <message>
        <source>Algorithm:</source>
        <translation>Αλγόριθμος:</translation>
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
        <translation>Αλγόριθμος συσχέτισης</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Κατάταξη Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Ευκλείδεια ομοιότητα</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Ομοιότητα συνημιτόνου</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Polarity:</source>
        <translation>Πολικότητα:</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Θετική</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Αρνητική</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Και οι δύο</translation>
    </message>
    <message>
        <source>Polarity</source>
        <translation>Πολικότητα</translation>
    </message>
    <message>
        <source>By default only positive correlations are considered when creating the graph. In almost all cases this is the correct setting, but for some data sources it may make more sense to take account of the polarity of the correlation. Note that not all correlation algorithms are capable of finding negative relationships.</source>
        <translation>Από προεπιλογή, κατά τη δημιουργία του γράφου λαμβάνονται υπόψη μόνο οι θετικές συσχετίσεις. Σχεδόν σε όλες τις περιπτώσεις αυτή είναι η σωστή ρύθμιση, αλλά για ορισμένες πηγές δεδομένων ίσως έχει περισσότερο νόημα να ληφθεί υπόψη η πολικότητα της συσχέτισης. Σημειώστε ότι δεν μπορούν όλοι οι αλγόριθμοι συσχέτισης να εντοπίσουν αρνητικές σχέσεις.</translation>
    </message>
    <message>
        <source>Imputation:</source>
        <translation>Συμπλήρωση:</translation>
    </message>
    <message>
        <source>Constant</source>
        <translation>Σταθερά</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Παρεμβολή γραμμής</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Μέσος όρος στήλης</translation>
    </message>
    <message>
        <source>Imputation</source>
        <translation>Συμπλήρωση</translation>
    </message>
    <message>
        <source>&lt;b&gt;Constant:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Σταθερά:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace all missing values with a constant.</source>
        <translation>Αντικατάσταση όλων των τιμών που λείπουν με μια σταθερά.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Row Interpolate:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Παρεμβολή γραμμής:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Linearly interpolate missing values using the nearest surrounding values in the row and their relative positions. If only one surrounding value is available the value will be set to match. The value will be set to zero if the row is empty.</source>
        <translation>Γραμμική παρεμβολή των τιμών που λείπουν με βάση τις πλησιέστερες γειτονικές τιμές της γραμμής και τις σχετικές τους θέσεις. Αν είναι διαθέσιμη μόνο μία γειτονική τιμή, η τιμή θα οριστεί ίση με αυτήν. Αν η γραμμή είναι κενή, η τιμή θα οριστεί σε μηδέν.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Column Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Μέσος όρος στήλης:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Replace missing values with the mean value of their column, excluding missing values.</source>
        <translation>Αντικατάσταση των τιμών που λείπουν με τη μέση τιμή της στήλης τους, εξαιρώντας τις τιμές που λείπουν.</translation>
    </message>
    <message>
        <source>Clipping:</source>
        <translation>Αποκοπή:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Κανένα</translation>
    </message>
    <message>
        <source>Winsorization</source>
        <translation>Winsorization</translation>
    </message>
    <message>
        <source>percentile</source>
        <translation>εκατοστημόριο</translation>
    </message>
    <message>
        <source>Clipping</source>
        <translation>Αποκοπή</translation>
    </message>
    <message>
        <source>Limit the maximum data table value to a specified constant.</source>
        <translation>Περιορισμός της μέγιστης τιμής του πίνακα δεδομένων σε μια καθορισμένη σταθερά.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Winsorization:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Winsorization:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Limit the maximum data value to that at a specified percentile, on a per-row basis. For example if the 95th percentile is chosen, the top 5th percentile of values will be set to that of the 95th percentile.</source>
        <translation>Περιορισμός της μέγιστης τιμής δεδομένων στην τιμή ενός καθορισμένου εκατοστημορίου, ανά γραμμή. Για παράδειγμα, αν επιλεγεί το 95ο εκατοστημόριο, το ανώτερο 5% των τιμών θα οριστεί στην τιμή του 95ου εκατοστημορίου.</translation>
    </message>
    <message>
        <source>Scaling:</source>
        <translation>Κλιμάκωση:</translation>
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
        <translation>Τύποι κλιμάκωσης</translation>
    </message>
    <message>
        <source>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</source>
        <translation>&lt;b&gt;Log&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt; + ε):</translation>
    </message>
    <message>
        <source>Perform a Log of &lt;i&gt;x&lt;/i&gt; + ε to base &lt;i&gt;b&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data and ε is a very small constant.</source>
        <translation>Υπολογισμός του λογαρίθμου του &lt;i&gt;x&lt;/i&gt; + ε με βάση &lt;i&gt;b&lt;/i&gt;, όπου &lt;i&gt;x&lt;/i&gt; είναι τα δεδομένα εισόδου και ε μια πολύ μικρή σταθερά.</translation>
    </message>
    <message>
        <source>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;AntiLog&lt;/b&gt;&lt;i&gt;b&lt;/i&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Raise &lt;i&gt;b&lt;/i&gt; to the power of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data.</source>
        <translation>Ύψωση του &lt;i&gt;b&lt;/i&gt; στη δύναμη &lt;i&gt;x&lt;/i&gt;, όπου &lt;i&gt;x&lt;/i&gt; είναι τα δεδομένα εισόδου.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</source>
        <translation>&lt;b&gt;Arcsin&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;):</translation>
    </message>
    <message>
        <source>Perform an inverse sine function of &lt;i&gt;x&lt;/i&gt;, where &lt;i&gt;x&lt;/i&gt; is the input data. This is useful when you require a log transform but the dataset contains negative numbers or zeros.</source>
        <translation>Υπολογισμός του τόξου ημιτόνου του &lt;i&gt;x&lt;/i&gt;, όπου &lt;i&gt;x&lt;/i&gt; είναι τα δεδομένα εισόδου. Χρήσιμο όταν απαιτείται λογαριθμικός μετασχηματισμός αλλά το σύνολο δεδομένων περιέχει αρνητικούς αριθμούς ή μηδενικά.</translation>
    </message>
    <message>
        <source>Normalisation:</source>
        <translation>Κανονικοποίηση:</translation>
    </message>
    <message>
        <source>Min/Max</source>
        <translation>Ελάχ./Μέγ.</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Μέσος όρος</translation>
    </message>
    <message>
        <source>Standardisation</source>
        <translation>Τυποποίηση</translation>
    </message>
    <message>
        <source>Unit Scaling</source>
        <translation>Μοναδιαία κλιμάκωση</translation>
    </message>
    <message>
        <source>Quantile</source>
        <translation>Ποσοστημόριο</translation>
    </message>
    <message>
        <source>Softmax</source>
        <translation>Softmax</translation>
    </message>
    <message>
        <source>Normalisation Types</source>
        <translation>Τύποι κανονικοποίησης</translation>
    </message>
    <message>
        <source>&lt;b&gt;Min/Max:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ελάχ./Μέγ.:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Rescale the data so that its values lie within the [0, 1] range: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). This is useful if the columns in the dataset have differing scales or units. Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Επανακλιμάκωση των δεδομένων ώστε οι τιμές τους να βρίσκονται στο εύρος [0, 1]: (&lt;i&gt;x&lt;/i&gt;-min(&lt;i&gt;x&lt;/i&gt;))/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Χρήσιμο αν οι στήλες του συνόλου δεδομένων έχουν διαφορετικές κλίμακες ή μονάδες. Σημείωση: αν όλα τα στοιχεία μιας στήλης έχουν την ίδια τιμή, οι τιμές θα επανακλιμακωθούν σε 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Mean:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Μέσος όρος:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Similar to Min/Max scaling except the resultant values are centred around their column mean: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Note: If all elements in a column have the same value this will rescale the values to 0.0.</source>
        <translation>Παρόμοιο με την κλιμάκωση Ελάχ./Μέγ., με τη διαφορά ότι οι τιμές που προκύπτουν κεντράρονται γύρω από τον μέσο όρο της στήλης τους: (&lt;i&gt;x&lt;/i&gt;-µ)/(max(&lt;i&gt;x&lt;/i&gt;)-min(&lt;i&gt;x&lt;/i&gt;)). Σημείωση: αν όλα τα στοιχεία μιας στήλης έχουν την ίδια τιμή, οι τιμές θα επανακλιμακωθούν σε 0.0.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Standardisation:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Τυποποίηση:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Also known as Z-score normalisation, this method centres the value around the mean and scales by the standard deviation: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</source>
        <translation>Γνωστή και ως κανονικοποίηση Z-score, αυτή η μέθοδος κεντράρει την τιμή γύρω από τον μέσο όρο και την κλιμακώνει με την τυπική απόκλιση: (&lt;i&gt;x&lt;/i&gt;-µ)/σ.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Unit Scaling:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Μοναδιαία κλιμάκωση:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Values are divided by the euclidean length of the vector formed by the column of data: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</source>
        <translation>Οι τιμές διαιρούνται με το ευκλείδειο μήκος του διανύσματος που σχηματίζει η στήλη δεδομένων: &lt;i&gt;x&lt;/i&gt;/‖&lt;i&gt;x&lt;/i&gt;‖.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Quantile:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Ποσοστημόριο:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Normalise the data so that the columns have equal distributions.</source>
        <translation>Κανονικοποίηση των δεδομένων ώστε οι στήλες να έχουν ίσες κατανομές.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Softmax:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Softmax:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The softmax function converts each column of data into a probability distribution by exponentiating each element, and normalising by dividing by the sum. Ultimately this causes the largest values in each column to be exaggerated.</source>
        <translation>Η συνάρτηση softmax μετατρέπει κάθε στήλη δεδομένων σε κατανομή πιθανότητας, υψώνοντας εκθετικά κάθε στοιχείο και κανονικοποιώντας με διαίρεση διά του αθροίσματος. Τελικά αυτό μεγεθύνει τις μεγαλύτερες τιμές κάθε στήλης.</translation>
    </message>
    <message>
        <source>Filtering:</source>
        <translation>Φιλτράρισμα:</translation>
    </message>
    <message>
        <source>Threshold</source>
        <translation>Κατώφλι</translation>
    </message>
    <message>
        <source>k-NN</source>
        <translation>k-NN</translation>
    </message>
    <message>
        <source>Filtering</source>
        <translation>Φιλτράρισμα</translation>
    </message>
    <message>
        <source>&lt;b&gt;Threshold:&lt;/b&gt;</source>
        <translation>&lt;b&gt;Κατώφλι:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Simple filter where correlations below a selected threshold are discarded.</source>
        <translation>Απλό φίλτρο που απορρίπτει τις συσχετίσεις κάτω από ένα επιλεγμένο κατώφλι.</translation>
    </message>
    <message>
        <source>&lt;b&gt;k-Nearest Neighbours:&lt;/b&gt;</source>
        <translation>&lt;b&gt;k πλησιέστεροι γείτονες:&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Track correlations found between row pairs, discarding those that don&apos;t lie in the top &lt;i&gt;k&lt;/i&gt; correlations for each row. This is most useful for datasets that are extremely highly correlated.</source>
        <translation>Καταγραφή των συσχετίσεων μεταξύ ζευγών γραμμών, απορρίπτοντας όσες δεν ανήκουν στις &lt;i&gt;k&lt;/i&gt; ισχυρότερες συσχετίσεις κάθε γραμμής. Είναι πιο χρήσιμο για σύνολα δεδομένων με εξαιρετικά υψηλή συσχέτιση.</translation>
    </message>
    <message>
        <source>Minimum:</source>
        <translation>Ελάχιστο:</translation>
    </message>
    <message>
        <source>Minimum Correlation Value</source>
        <translation>Ελάχιστη τιμή συσχέτισης</translation>
    </message>
    <message>
        <source>The minimum correlation value above which an edge will be created in the graph. Using a lower minimum value will increase the compute and memory requirements.</source>
        <translation>Η ελάχιστη τιμή συσχέτισης πάνω από την οποία θα δημιουργείται ακμή στον γράφο. Μια χαμηλότερη ελάχιστη τιμή αυξάνει τις απαιτήσεις σε υπολογισμούς και μνήμη.</translation>
    </message>
    <message>
        <source>Maximum k:</source>
        <translation>Μέγιστο k:</translation>
    </message>
    <message>
        <source>Maximum Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>Μέγιστο &lt;i&gt;k&lt;/i&gt; συσχέτισης</translation>
    </message>
    <message>
        <source>The maximum number of correlations to retain per data row, ultimately limiting the number of edges that will be created in the graph. Using a larger maximum value will increase the compute and memory requirements.</source>
        <translation>Ο μέγιστος αριθμός συσχετίσεων που διατηρούνται ανά γραμμή δεδομένων, περιορίζοντας τελικά τον αριθμό των ακμών που θα δημιουργηθούν στον γράφο. Μια μεγαλύτερη μέγιστη τιμή αυξάνει τις απαιτήσεις σε υπολογισμούς και μνήμη.</translation>
    </message>
    <message>
        <source>Initial:</source>
        <translation>Αρχικό:</translation>
    </message>
    <message>
        <source>Initial k:</source>
        <translation>Αρχικό k:</translation>
    </message>
    <message>
        <source>Correlation Threshold</source>
        <translation>Κατώφλι συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation &lt;i&gt;k&lt;/i&gt;</source>
        <translation>&lt;i&gt;k&lt;/i&gt; συσχέτισης</translation>
    </message>
    <message>
        <source>The initial correlation threshold determines the size of the resultant graph. A lower value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Το αρχικό κατώφλι συσχέτισης καθορίζει το μέγεθος του γράφου που προκύπτει. Μια χαμηλότερη τιμή φιλτράρει λιγότερες ακμές και οδηγεί σε μεγαλύτερο γράφο. Η τιμή αυτή μπορεί να αλλάξει αργότερα, αφού δημιουργηθεί ο γράφος.</translation>
    </message>
    <message>
        <source>The initial k value determines the size of the resultant graph. A higher value filters fewer edges, and results in a larger graph. This value can be changed later, after the graph has been created.</source>
        <translation>Η αρχική τιμή k καθορίζει το μέγεθος του γράφου που προκύπτει. Μια υψηλότερη τιμή φιλτράρει λιγότερες ακμές και οδηγεί σε μεγαλύτερο γράφο. Η τιμή αυτή μπορεί να αλλάξει αργότερα, αφού δημιουργηθεί ο γράφος.</translation>
    </message>
    <message>
        <source>Working…</source>
        <translation>Επεξεργασία…</translation>
    </message>
    <message>
        <source>Empty Graph</source>
        <translation>Κενός γράφος</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Πρότυπο</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Template&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Πρότυπο&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A template is a named set of transforms and visualisations that can be applied to the graph after it is created. Please select a template to apply. &lt;b&gt;Note:&lt;/b&gt; transforms applied here will always be appended unconditionally, regardless of the template&apos;s specific application method.</source>
        <translation>Ένα πρότυπο είναι ένα επώνυμο σύνολο μετασχηματισμών και οπτικοποιήσεων που μπορεί να εφαρμοστεί στον γράφο μετά τη δημιουργία του. Επιλέξτε ένα πρότυπο για εφαρμογή. &lt;b&gt;Σημείωση:&lt;/b&gt; οι μετασχηματισμοί που εφαρμόζονται εδώ θα προσαρτώνται πάντα χωρίς όρους, ανεξάρτητα από τη μέθοδο εφαρμογής του συγκεκριμένου προτύπου.</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Όνομα:</translation>
    </message>
    <message>
        <source>Set As Default</source>
        <translation>Ορισμός ως προεπιλογή</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Σύνοψη</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Σύνοψη&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>A graph will be created using the following parameters:</source>
        <translation>Θα δημιουργηθεί ένας γράφος με τις ακόλουθες παραμέτρους:</translation>
    </message>
    <message>
        <source>Transposed&lt;br&gt;</source>
        <translation>Με αντιμετάθεση&lt;br&gt;</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: {0} Row: {1} Width: {2} Height: {3} ]&lt;br&gt;</source>
        <translation>Πλαίσιο δεδομένων: [ Στήλη: {0} Γραμμή: {1} Πλάτος: {2} Ύψος: {3} ]&lt;br&gt;</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: {0}&lt;br&gt;</source>
        <translation>Ελάχιστη τιμή συσχέτισης: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial Correlation Threshold: {0}&lt;br&gt;</source>
        <translation>Αρχικό κατώφλι συσχέτισης: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Maximum k Value: {0}&lt;br&gt;</source>
        <translation>Μέγιστη τιμή k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Initial k Value: {0}&lt;br&gt;</source>
        <translation>Αρχική τιμή k: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Μέτρο συσχέτισης συνεχών δεδομένων: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Correlation Polarity: {0}&lt;br&gt;</source>
        <translation>Πολικότητα συσχέτισης: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Imputation: {0} ({1})</source>
        <translation>Συμπλήρωση: {0} ({1})</translation>
    </message>
    <message>
        <source>Imputation: {0}</source>
        <translation>Συμπλήρωση: {0}</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1})</source>
        <translation>Αποκοπή: {0} ({1})</translation>
    </message>
    <message>
        <source>Clipping: {0} ({1} percentile)</source>
        <translation>Αποκοπή: {0} ({1}ο εκατοστημόριο)</translation>
    </message>
    <message>
        <source>Clipping: {0}</source>
        <translation>Αποκοπή: {0}</translation>
    </message>
    <message>
        <source>Scaling: {0}&lt;br&gt;</source>
        <translation>Κλιμάκωση: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Normalisation: {0}&lt;br&gt;</source>
        <translation>Κανονικοποίηση: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: {0}&lt;br&gt;</source>
        <translation>Μέτρο συσχέτισης διακριτών δεδομένων: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Template: {0}&lt;br&gt;</source>
        <translation>Πρότυπο: {0}&lt;br&gt;</translation>
    </message>
    <message>
        <source>Estimated Pre-Transform Graph Size: {0}{3} Nodes{2}, {1}{4} Edges{2}</source>
        <translation>Εκτιμώμενο μέγεθος γράφου πριν από τους μετασχηματισμούς: {0}{3} κόμβοι{2}, {1}{4} ακμές{2}</translation>
    </message>
    <message>
        <source>Increasing the Minimum Correlation Value will usually reduce the graph size.</source>
        <translation>Η αύξηση της ελάχιστης τιμής συσχέτισης συνήθως μειώνει το μέγεθος του γράφου.</translation>
    </message>
    <message>
        <source>Decreasing the Maximum &lt;i&gt;k&lt;/i&gt; will usually reduce the graph size.</source>
        <translation>Η μείωση του μέγιστου &lt;i&gt;k&lt;/i&gt; συνήθως μειώνει το μέγεθος του γράφου.</translation>
    </message>
    <message>
        <source>&lt;br&gt;&lt;br&gt;{0}WARNING: This is a very large graph which has the potential to exhaust system resources and lead to instability or freezes. {1}{2}</source>
        <translation>&lt;br&gt;&lt;br&gt;{0}ΠΡΟΕΙΔΟΠΟΙΗΣΗ: αυτός είναι ένας πολύ μεγάλος γράφος που ενδέχεται να εξαντλήσει τους πόρους του συστήματος και να οδηγήσει σε αστάθεια ή παγώματα. {1}{2}</translation>
    </message>
    <message>
        <source>&lt;br&gt;{0}WARNING: It is likely that the generated graph will be empty.{1}</source>
        <translation>&lt;br&gt;{0}ΠΡΟΕΙΔΟΠΟΙΗΣΗ: ο γράφος που θα δημιουργηθεί πιθανότατα θα είναι κενός.{1}</translation>
    </message>
</context>
<context>
    <name>CorrelationPlot</name>
    <message>
        <source>1 column</source>
        <translation>1 στήλη</translation>
    </message>
    <message numerus="yes">
        <source>%n columns</source>
        <translation>
            <numerusform>%n στήλη</numerusform>
            <numerusform>%n στήλες</numerusform>
        </translation>
    </message>
    <message>
        <source>%1, %2: %3</source>
        <translation>%1, %2: %3</translation>
    </message>
    <message>
        <source>…and %1 more</source>
        <translation>…και %1 ακόμη</translation>
    </message>
    <message>
        <source>Resize To Expose Column Information</source>
        <translation>Αλλάξτε το μέγεθος για να εμφανιστούν πληροφορίες στηλών</translation>
    </message>
    <message>
        <source>Resize To Expose Column Names</source>
        <translation>Αλλάξτε το μέγεθος για να εμφανιστούν τα ονόματα στηλών</translation>
    </message>
    <message>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message numerus="yes">
        <source>%n unique values</source>
        <translation>
            <numerusform>%n μοναδική τιμή</numerusform>
            <numerusform>%n μοναδικές τιμές</numerusform>
        </translation>
    </message>
    <message>
        <source>Mean average of %1: %2</source>
        <translation>Μέσος όρος του %1: %2</translation>
    </message>
    <message>
        <source>Mean average of selection</source>
        <translation>Μέσος όρος της επιλογής</translation>
    </message>
    <message>
        <source>Median average of %1: %2</source>
        <translation>Διάμεσος του %1: %2</translation>
    </message>
    <message>
        <source>Median average of selection</source>
        <translation>Διάμεσος της επιλογής</translation>
    </message>
    <message>
        <source>Mean histogram of %1: %2</source>
        <translation>Μέσο ιστόγραμμα του %1: %2</translation>
    </message>
    <message>
        <source>Mean histogram of selection</source>
        <translation>Μέσο ιστόγραμμα της επιλογής</translation>
    </message>
    <message>
        <source>Deviation</source>
        <translation>Απόκλιση</translation>
    </message>
    <message>
        <source>%1 Top</source>
        <translation>%1 πάνω</translation>
    </message>
    <message>
        <source>%1 Bottom</source>
        <translation>%1 κάτω</translation>
    </message>
    <message>
        <source>Std Dev</source>
        <translation>Τυπ. απόκλ.</translation>
    </message>
    <message>
        <source>Std Err</source>
        <translation>Τυπ. σφάλμα</translation>
    </message>
    <message>
        <source>%1, %2 and %3 more…</source>
        <translation>%1, %2 και %3 ακόμη…</translation>
    </message>
</context>
<context>
    <name>CorrelationPlugin</name>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content could not be determined.</source>
        <translation>Η επέκταση του %1 υποδεικνύει ότι είναι «%2», ωστόσο δεν ήταν δυνατό να προσδιοριστεί το περιεχόμενό του.</translation>
    </message>
    <message>
        <source>%1 has an extension that indicates it is a &apos;%2&apos;, however its content resembles a &apos;%3&apos; file.</source>
        <translation>Η επέκταση του %1 υποδεικνύει ότι είναι «%2», ωστόσο το περιεχόμενό του μοιάζει με αρχείο «%3».</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Συσχέτιση</translation>
    </message>
    <message>
        <source>Creates a graph where nodes represent rows of data, and edges represent correlations between said rows.</source>
        <translation>Δημιουργεί έναν γράφο όπου οι κόμβοι αντιπροσωπεύουν γραμμές δεδομένων και οι ακμές συσχετίσεις μεταξύ αυτών των γραμμών.</translation>
    </message>
</context>
<context>
    <name>CorrelationPluginInstance</name>
    <message>
        <source>The Data Value is a parameterised attribute that permits referencing a specific column in the correlated data.</source>
        <translation>Η τιμή δεδομένων είναι ένα παραμετροποιημένο χαρακτηριστικό που επιτρέπει την αναφορά σε μια συγκεκριμένη στήλη των συσχετισμένων δεδομένων.</translation>
    </message>
    <message>
        <source>The name of the data column which has the largest absolute value for the data row associated with the node.</source>
        <translation>Το όνομα της στήλης δεδομένων με τη μεγαλύτερη απόλυτη τιμή στη γραμμή δεδομένων που σχετίζεται με τον κόμβο.</translation>
    </message>
    <message>
        <source>The Mean Data Value is the mean of the values associated with the node.</source>
        <translation>Η μέση τιμή δεδομένων είναι ο μέσος όρος των τιμών που σχετίζονται με τον κόμβο.</translation>
    </message>
    <message>
        <source>The Minimum Data Value is the minimum value associated with the node.</source>
        <translation>Η ελάχιστη τιμή δεδομένων είναι η μικρότερη τιμή που σχετίζεται με τον κόμβο.</translation>
    </message>
    <message>
        <source>The Maximum Data Value is the maximum value associated with the node.</source>
        <translation>Η μέγιστη τιμή δεδομένων είναι η μεγαλύτερη τιμή που σχετίζεται με τον κόμβο.</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as ∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>Η %1 είναι ένα μέτρο της διασποράς των τιμών που σχετίζονται με τον κόμβο. Ορίζεται ως ∑(&lt;i&gt;x&lt;/i&gt;-µ)², όπου &lt;i&gt;x&lt;/i&gt; η τιμή και µ ο μέσος όρος.</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Διακύμανση</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as √∑(&lt;i&gt;x&lt;/i&gt;-µ)², where &lt;i&gt;x&lt;/i&gt; is the value and µ is the mean.</source>
        <translation>Η %1 είναι ένα μέτρο της διασποράς των τιμών που σχετίζονται με τον κόμβο. Ορίζεται ως √∑(&lt;i&gt;x&lt;/i&gt;-µ)², όπου &lt;i&gt;x&lt;/i&gt; η τιμή και µ ο μέσος όρος.</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Τυπική απόκλιση</translation>
    </message>
    <message>
        <source>The %1 is a measure of the spread of the values associated with the node. It is defined as the standard deviation divided by the mean.</source>
        <translation>Ο %1 είναι ένα μέτρο της διασποράς των τιμών που σχετίζονται με τον κόμβο. Ορίζεται ως η τυπική απόκλιση διαιρεμένη με τον μέσο όρο.</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Συντελεστής μεταβλητότητας</translation>
    </message>
    <message>
        <source>Plugin data is missing &apos;%1&apos; key.</source>
        <translation>Από τα δεδομένα του πρόσθετου λείπει το κλειδί «%1».</translation>
    </message>
    <message>
        <source>Data Frame: [ Column: %1 Row: %2 Width: %3 Height: %4 ]</source>
        <translation>Πλαίσιο δεδομένων: [ Στήλη: %1 Γραμμή: %2 Πλάτος: %3 Ύψος: %4 ]</translation>
    </message>
    <message>
        <source>Transposed</source>
        <translation>Με αντιμετάθεση</translation>
    </message>
    <message>
        <source>Continuous Correlation Metric: %1</source>
        <translation>Μέτρο συσχέτισης συνεχών δεδομένων: %1</translation>
    </message>
    <message>
        <source>Positive</source>
        <translation>Θετική</translation>
    </message>
    <message>
        <source>Negative</source>
        <translation>Αρνητική</translation>
    </message>
    <message>
        <source>Both</source>
        <translation>Και οι δύο</translation>
    </message>
    <message>
        <source>Correlation Polarity: %1</source>
        <translation>Πολικότητα συσχέτισης: %1</translation>
    </message>
    <message>
        <source>Discrete Correlation Metric: %1</source>
        <translation>Μέτρο συσχέτισης διακριτών δεδομένων: %1</translation>
    </message>
    <message>
        <source>Minimum Correlation Value: %1</source>
        <translation>Ελάχιστη τιμή συσχέτισης: %1</translation>
    </message>
    <message>
        <source>Maximum k Value: %1</source>
        <translation>Μέγιστη τιμή k: %1</translation>
    </message>
    <message>
        <source>Constant (%1)</source>
        <translation>Σταθερά (%1)</translation>
    </message>
    <message>
        <source>Column Mean</source>
        <translation>Μέσος όρος στήλης</translation>
    </message>
    <message>
        <source>Row Interpolate</source>
        <translation>Παρεμβολή γραμμής</translation>
    </message>
    <message>
        <source>Imputation: %1</source>
        <translation>Συμπλήρωση: %1</translation>
    </message>
    <message>
        <source>Clipping: Constant (%1)</source>
        <translation>Αποκοπή: Σταθερά (%1)</translation>
    </message>
    <message>
        <source>Clipping: Winsorization (%1 percentile)</source>
        <translation>Αποκοπή: Winsorization (%1ο εκατοστημόριο)</translation>
    </message>
    <message>
        <source>Scaling: Log2(x + ε)</source>
        <translation>Κλιμάκωση: Log2(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: Log10(x + ε)</source>
        <translation>Κλιμάκωση: Log10(x + ε)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog2(x)</source>
        <translation>Κλιμάκωση: AntiLog2(x)</translation>
    </message>
    <message>
        <source>Scaling: AntiLog10(x)</source>
        <translation>Κλιμάκωση: AntiLog10(x)</translation>
    </message>
    <message>
        <source>Scaling: Arcsin(x)</source>
        <translation>Κλιμάκωση: Arcsin(x)</translation>
    </message>
    <message>
        <source>Normalisation: Min/Max</source>
        <translation>Κανονικοποίηση: Ελάχ./Μέγ.</translation>
    </message>
    <message>
        <source>Normalisation: Quantile</source>
        <translation>Κανονικοποίηση: Ποσοστημόριο</translation>
    </message>
    <message>
        <source>Normalisation: Mean</source>
        <translation>Κανονικοποίηση: Μέσος όρος</translation>
    </message>
    <message>
        <source>Normalisation: Standardisation</source>
        <translation>Κανονικοποίηση: Τυποποίηση</translation>
    </message>
    <message>
        <source>Normalisation: Unit Scaling</source>
        <translation>Κανονικοποίηση: Μοναδιαία κλιμάκωση</translation>
    </message>
    <message>
        <source>Normalisation: Softmax Scaling</source>
        <translation>Κανονικοποίηση: Κλιμάκωση softmax</translation>
    </message>
</context>
<context>
    <name>ImportAnnotationsDialog</name>
    <message>
        <source>Import Column Annotations</source>
        <translation>Εισαγωγή σχολιασμών στηλών</translation>
    </message>
    <message>
        <source>No Suitable Keys</source>
        <translation>Δεν υπάρχουν κατάλληλα κλειδιά</translation>
    </message>
    <message>
        <source>Failed to Load {0}.</source>
        <translation>Αποτυχία φόρτωσης του {0}.</translation>
    </message>
    <message>
        <source>Loading {0}…</source>
        <translation>Φόρτωση {0}…</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Κλείσιμο</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Ακύρωση</translation>
    </message>
    <message>
        <source>Introduction</source>
        <translation>Εισαγωγή</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Import Annotations&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Εισαγωγή σχολιασμών&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Annotation data may be imported from an external source, and superimposed on the plot. Any of the supported tabular data formats can be used. Following the import of new annotations, they can be sorted and grouped in the usual way.</source>
        <translation>Τα δεδομένα σχολιασμών μπορούν να εισαχθούν από εξωτερική πηγή και να επιτεθούν στο διάγραμμα. Μπορεί να χρησιμοποιηθεί οποιαδήποτε από τις υποστηριζόμενες μορφές δεδομένων πίνακα. Μετά την εισαγωγή, οι νέοι σχολιασμοί μπορούν να ταξινομηθούν και να ομαδοποιηθούν με τον συνήθη τρόπο.</translation>
    </message>
    <message>
        <source>Key Selection</source>
        <translation>Επιλογή κλειδιών</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Key Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Επιλογή κλειδιών&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>In order to import annotation data, it is necessary to identify the input data row which corresponds to the column names of the existing data. This is used to correlate columns in the table with columns in the plot. Please select the correct one below. Alternatively, click &lt;i&gt;{0}&lt;/i&gt; and the best match will be identified automatically.</source>
        <translation>Για την εισαγωγή δεδομένων σχολιασμών, πρέπει να προσδιοριστεί η γραμμή των δεδομένων εισόδου που αντιστοιχεί στα ονόματα στηλών των υπαρχόντων δεδομένων. Χρησιμοποιείται για την αντιστοίχιση των στηλών του πίνακα με τις στήλες του διαγράμματος. Επιλέξτε παρακάτω τη σωστή γραμμή. Εναλλακτικά, κάντε κλικ στο &lt;i&gt;{0}&lt;/i&gt; και η καλύτερη αντιστοιχία θα εντοπιστεί αυτόματα.</translation>
    </message>
    <message>
        <source>Auto Detect</source>
        <translation>Αυτόματος εντοπισμός</translation>
    </message>
    <message>
        <source>Selected row &lt;b&gt;{0}&lt;/b&gt; has duplicate values, some columns will be ignored!</source>
        <translation>Η επιλεγμένη γραμμή &lt;b&gt;{0}&lt;/b&gt; έχει διπλότυπες τιμές, ορισμένες στήλες θα αγνοηθούν!</translation>
    </message>
    <message>
        <source>Found {0}% Match</source>
        <translation>Βρέθηκε αντιστοίχιση {0}%</translation>
    </message>
    <message>
        <source>No Match Found</source>
        <translation>Δεν βρέθηκε αντιστοίχιση</translation>
    </message>
    <message>
        <source>Annotation Selection</source>
        <translation>Επιλογή σχολιασμών</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Annotation Selection&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Επιλογή σχολιασμών&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Please select the required annotations from the list on the right. Multiple annotations may be selected by holding down &lt;i&gt;Ctrl&lt;/i&gt;, or a range may be selected by holding &lt;i&gt;Shift&lt;/i&gt;, then clicking as desired.</source>
        <translation>Επιλέξτε τους σχολιασμούς που χρειάζεστε από τη λίστα στα δεξιά. Μπορείτε να επιλέξετε πολλούς σχολιασμούς κρατώντας πατημένο το &lt;i&gt;Ctrl&lt;/i&gt;, ή ένα εύρος κρατώντας πατημένο το &lt;i&gt;Shift&lt;/i&gt; και κάνοντας κλικ όπου θέλετε.</translation>
    </message>
    <message>
        <source>Replace Existing Annotations</source>
        <translation>Αντικατάσταση υπαρχόντων σχολιασμών</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation>Επιλογή όλων</translation>
    </message>
    <message>
        <source>Select None</source>
        <translation>Καμία επιλογή</translation>
    </message>
    <message>
        <source>Summary</source>
        <translation>Σύνοψη</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Summary&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Σύνοψη&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Detecting key…</source>
        <translation>Ανίχνευση κλειδιού…</translation>
    </message>
    <message>
        <source>The following annotation(s) will be imported using the row &lt;b&gt;{0}{1}&lt;/b&gt; as the key:</source>
        <translation>Οι ακόλουθοι σχολιασμοί θα εισαχθούν με χρήση της γραμμής &lt;b&gt;{0}{1}&lt;/b&gt; ως κλειδιού:</translation>
    </message>
    <message>
        <source> &lt;font color=&quot;red&quot;&gt;(has duplicate values)&lt;/font&gt;</source>
        <translation> &lt;font color=&quot;red&quot;&gt;(περιέχει διπλότυπες τιμές)&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&quot;red&quot;&gt;Annotations cannot be imported unless a key is selected.&lt;/font&gt;</source>
        <translation>&lt;font color=&quot;red&quot;&gt;Οι σχολιασμοί δεν μπορούν να εισαχθούν αν δεν επιλεγεί κλειδί.&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Detecting Key</source>
        <translation>Ανίχνευση κλειδιού</translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Display UI &amp;Side By Side</source>
        <translation>Εμφάνιση διεπαφής &amp;δίπλα δίπλα</translation>
    </message>
    <message>
        <source>Show &amp;Column Names</source>
        <translation>Εμφάνιση ονομάτων &amp;στηλών</translation>
    </message>
    <message>
        <source>Find Rows Of Interest</source>
        <translation>Εύρεση γραμμών ενδιαφέροντος</translation>
    </message>
    <message>
        <source>Select Visible Column Annotations</source>
        <translation>Επιλογή ορατών σχολιασμών στηλών</translation>
    </message>
    <message>
        <source>Grid Lines</source>
        <translation>Γραμμές πλέγματος</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Υπόμνημα</translation>
    </message>
    <message>
        <source>Include Zero In Y Axis</source>
        <translation>Συμπερίληψη του μηδενός στον άξονα Y</translation>
    </message>
    <message>
        <source>Show Outliers</source>
        <translation>Εμφάνιση ακραίων τιμών</translation>
    </message>
    <message>
        <source>Reset Zoom</source>
        <translation>Επαναφορά ζουμ</translation>
    </message>
    <message>
        <source>Show All Columns</source>
        <translation>Εμφάνιση όλων των στηλών</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Κανένα</translation>
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
        <translation>Κεντράρισμα στον μέσο όρο</translation>
    </message>
    <message>
        <source>Unit Variance Scaling</source>
        <translation>Κλιμάκωση σε μοναδιαία διακύμανση</translation>
    </message>
    <message>
        <source>Pareto Scaling</source>
        <translation>Κλιμάκωση Pareto</translation>
    </message>
    <message>
        <source>Error Bars</source>
        <translation>Ράβδοι σφάλματος</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Περιοχή</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Τυπική απόκλιση</translation>
    </message>
    <message>
        <source>Standard Error</source>
        <translation>Τυπικό σφάλμα</translation>
    </message>
    <message>
        <source>Individual Line</source>
        <translation>Μεμονωμένη γραμμή</translation>
    </message>
    <message>
        <source>Mean Line</source>
        <translation>Γραμμή μέσου όρου</translation>
    </message>
    <message>
        <source>Median Line</source>
        <translation>Γραμμή διαμέσου</translation>
    </message>
    <message>
        <source>Mean Histogram</source>
        <translation>Μέσο ιστόγραμμα</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Ενδοτεταρτημοριακό εύρος</translation>
    </message>
    <message>
        <source>Group By Annotation</source>
        <translation>Ομαδοποίηση κατά σχολιασμό</translation>
    </message>
    <message>
        <source>Import Annotations From Table…</source>
        <translation>Εισαγωγή σχολιασμών από πίνακα…</translation>
    </message>
    <message>
        <source>{0}/{1}-correlation-plot</source>
        <translation>{0}/{1}-διάγραμμα-συσχέτισης</translation>
    </message>
    <message>
        <source>Save As &amp;Image…</source>
        <translation>Αποθήκευση ως &amp;εικόνα…</translation>
    </message>
    <message>
        <source>Set X Axis Label…</source>
        <translation>Ορισμός ετικέτας άξονα X…</translation>
    </message>
    <message>
        <source>Set Y Axis Label…</source>
        <translation>Ορισμός ετικέτας άξονα Y…</translation>
    </message>
    <message>
        <source>X Axis Label</source>
        <translation>Ετικέτα άξονα X</translation>
    </message>
    <message>
        <source>Please enter the X Axis label:</source>
        <translation>Εισαγάγετε την ετικέτα του άξονα X:</translation>
    </message>
    <message>
        <source>Y Axis Label</source>
        <translation>Ετικέτα άξονα Y</translation>
    </message>
    <message>
        <source>Please enter the Y Axis label:</source>
        <translation>Εισαγάγετε την ετικέτα του άξονα Y:</translation>
    </message>
    <message>
        <source>&amp;Plot</source>
        <translation>&amp;Διάγραμμα</translation>
    </message>
    <message>
        <source>Axis Labels</source>
        <translation>Ετικέτες αξόνων</translation>
    </message>
    <message>
        <source>Scaling</source>
        <translation>Κλιμάκωση</translation>
    </message>
    <message>
        <source>By Attribute</source>
        <translation>Κατά χαρακτηριστικό</translation>
    </message>
    <message>
        <source>Averaging</source>
        <translation>Υπολογισμός μέσου</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Όλα</translation>
    </message>
    <message>
        <source>Dispersion</source>
        <translation>Διασπορά</translation>
    </message>
    <message>
        <source>Sort Columns By</source>
        <translation>Ταξινόμηση στηλών κατά</translation>
    </message>
    <message>
        <source>Colour Groups By</source>
        <translation>Χρωματισμός ομάδων κατά</translation>
    </message>
    <message>
        <source>Save As Images By…</source>
        <translation>Αποθήκευση ως εικόνες κατά…</translation>
    </message>
    <message>
        <source>Row</source>
        <translation>Γραμμή</translation>
    </message>
    <message>
        <source>Natural Order</source>
        <translation>Φυσική σειρά</translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation>Όνομα στήλης</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Hierarchical Clustering</source>
        <translation>Ιεραρχική ομαδοποίηση</translation>
    </message>
    <message>
        <source>Click on a column or a column annotation to select. Hold &lt;i&gt;Ctrl&lt;/i&gt; and click to select multiple columns.</source>
        <translation>Κάντε κλικ σε μια στήλη ή σε έναν σχολιασμό στήλης για επιλογή. Κρατήστε πατημένο το &lt;i&gt;Ctrl&lt;/i&gt; και κάντε κλικ για να επιλέξετε πολλές στήλες.</translation>
    </message>
    <message>
        <source>Fix Data Value Sort Order</source>
        <translation>Σταθεροποίηση σειράς ταξινόμησης κατά τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Save Plot As Image</source>
        <translation>Αποθήκευση γραφήματος ως εικόνα</translation>
    </message>
    <message>
        <source>PNG Image (*.png)</source>
        <translation>Εικόνα PNG (*.png)</translation>
    </message>
    <message>
        <source>JPEG Image (*.jpg *.jpeg)</source>
        <translation>Εικόνα JPEG (*.jpg *.jpeg)</translation>
    </message>
    <message>
        <source>PDF Document (*.pdf)</source>
        <translation>Έγγραφο PDF (*.pdf)</translation>
    </message>
</context>
<context>
    <name>PlotModalControls</name>
    <message>
        <source>Show All Annotations</source>
        <translation>Εμφάνιση όλων των σχολιασμών</translation>
    </message>
    <message>
        <source>Hide All Annotations</source>
        <translation>Απόκρυψη όλων των σχολιασμών</translation>
    </message>
    <message>
        <source>Relative</source>
        <translation>Σχετική</translation>
    </message>
    <message>
        <source>Absolute</source>
        <translation>Απόλυτη</translation>
    </message>
    <message>
        <source>Selection Weighting</source>
        <translation>Στάθμιση επιλογής</translation>
    </message>
    <message>
        <source>Rows are selected on the basis of weighting the sum of the selected columns against the non-selected columns. Weighting for the selected columns (denoted &lt;i&gt;Absolute&lt;/i&gt;) tends to find rows which have peaks in those columns, regardless of the non-selected columns. In contrast, (negatively) weighting for the non-selected columns (denoted &lt;i&gt;Relative&lt;/i&gt;) tends to find rows which have significant values specifically in the selected columns.</source>
        <translation>Οι γραμμές επιλέγονται με βάση τη στάθμιση του αθροίσματος των επιλεγμένων στηλών έναντι των μη επιλεγμένων. Η στάθμιση υπέρ των επιλεγμένων στηλών (&lt;i&gt;Απόλυτη&lt;/i&gt;) τείνει να βρίσκει γραμμές με κορυφές σε αυτές τις στήλες, ανεξάρτητα από τις μη επιλεγμένες. Αντίθετα, η (αρνητική) στάθμιση των μη επιλεγμένων στηλών (&lt;i&gt;Σχετική&lt;/i&gt;) τείνει να βρίσκει γραμμές με σημαντικές τιμές ειδικά στις επιλεγμένες στήλες.</translation>
    </message>
    <message>
        <source>{0}st</source>
        <translation>{0}ο</translation>
    </message>
    <message>
        <source>{0}nd</source>
        <translation>{0}ο</translation>
    </message>
    <message>
        <source>{0}rd</source>
        <translation>{0}ο</translation>
    </message>
    <message>
        <source>{0}th</source>
        <translation>{0}ο</translation>
    </message>
    <message>
        <source>Percentile</source>
        <translation>Εκατοστημόριο</translation>
    </message>
    <message>
        <source>The quantity of rows to select and rank.</source>
        <translation>Το πλήθος των γραμμών προς επιλογή και κατάταξη.</translation>
    </message>
    <message>
        <source>Reselect</source>
        <translation>Επανεπιλογή</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>IQR of %1</source>
        <translation>Ενδοτεταρτημοριακό εύρος του %1</translation>
    </message>
    <message>
        <source>IQR</source>
        <translation>Ενδοτεταρτημοριακό εύρος</translation>
    </message>
    <message>
        <source>Saving Plot Image(s)</source>
        <translation>Αποθήκευση εικόνων διαγράμματος</translation>
    </message>
    <message>
        <source>Pearson</source>
        <translation>Pearson</translation>
    </message>
    <message>
        <source>The Pearson Correlation Coefficient is an indication of the linear correlation between two variables.</source>
        <translation>Ο συντελεστής συσχέτισης Pearson δείχνει τη γραμμική συσχέτιση μεταξύ δύο μεταβλητών.</translation>
    </message>
    <message>
        <source>Pearson Correlation Value</source>
        <translation>Τιμή συσχέτισης Pearson</translation>
    </message>
    <message>
        <source>The %1 is an indication of the linear relationship between two variables.</source>
        <translation>Ο %1 δείχνει τη γραμμική σχέση μεταξύ δύο μεταβλητών.</translation>
    </message>
    <message>
        <source>Pearson Correlation Coefficient</source>
        <translation>Συντελεστής συσχέτισης Pearson</translation>
    </message>
    <message>
        <source>Spearman Rank</source>
        <translation>Κατάταξη Spearman</translation>
    </message>
    <message>
        <source>Spearman&apos;s rank correlation coefficient is a nonparametric measure of the statistical dependence between the rankings of two variables. It assesses how well the relationship between two variables can be described using a monotonic function.</source>
        <translation>Ο συντελεστής συσχέτισης τάξης Spearman είναι ένα μη παραμετρικό μέτρο της στατιστικής εξάρτησης μεταξύ των κατατάξεων δύο μεταβλητών. Αξιολογεί πόσο καλά μπορεί να περιγραφεί η σχέση μεταξύ δύο μεταβλητών με μια μονότονη συνάρτηση.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Value</source>
        <translation>Τιμή συσχέτισης τάξης Spearman</translation>
    </message>
    <message>
        <source>The %1 is an indication of the monotonic relationship between two variables.</source>
        <translation>Ο %1 δείχνει τη μονότονη σχέση μεταξύ δύο μεταβλητών.</translation>
    </message>
    <message>
        <source>Spearman Rank Correlation Coefficient</source>
        <translation>Συντελεστής συσχέτισης τάξης Spearman</translation>
    </message>
    <message>
        <source>Euclidean Similarity</source>
        <translation>Ευκλείδεια ομοιότητα</translation>
    </message>
    <message>
        <source>Euclidean Similarity is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Η ευκλείδεια ομοιότητα είναι ουσιαστικά το αντίστροφο της ευκλείδειας απόστασης μεταξύ δύο διανυσμάτων.</translation>
    </message>
    <message>
        <source>%1 is essentially the inverse of the Euclidean distance between two vectors.</source>
        <translation>Η %1 είναι ουσιαστικά το αντίστροφο της ευκλείδειας απόστασης μεταξύ δύο διανυσμάτων.</translation>
    </message>
    <message>
        <source>Cosine Similarity</source>
        <translation>Ομοιότητα συνημιτόνου</translation>
    </message>
    <message>
        <source>Cosine Similarity is a measure of similarity between two non-zero vectors of an inner product space. It is defined to equal the cosine of the angle between them, which is also the same as the inner product of the same vectors normalized to both have length 1.</source>
        <translation>Η ομοιότητα συνημιτόνου είναι ένα μέτρο ομοιότητας μεταξύ δύο μη μηδενικών διανυσμάτων ενός χώρου εσωτερικού γινομένου. Ορίζεται ως το συνημίτονο της γωνίας μεταξύ τους, που ισούται επίσης με το εσωτερικό γινόμενο των ίδιων διανυσμάτων κανονικοποιημένων ώστε να έχουν και τα δύο μήκος 1.</translation>
    </message>
    <message>
        <source>%1 is a measure of similarity between two non-zero vectors of an inner product space.</source>
        <translation>Η %1 είναι ένα μέτρο ομοιότητας μεταξύ δύο μη μηδενικών διανυσμάτων ενός χώρου εσωτερικού γινομένου.</translation>
    </message>
    <message>
        <source>Bicor</source>
        <translation>Bicor</translation>
    </message>
    <message>
        <source>Biweight Midcorrelation (also called Bicor) is a measure of similarity between samples. It is median-based, rather than mean-based, thus is less sensitive to outliers.</source>
        <translation>Η Biweight Midcorrelation (γνωστή και ως Bicor) είναι ένα μέτρο ομοιότητας μεταξύ δειγμάτων. Βασίζεται στη διάμεσο αντί στον μέσο όρο και είναι επομένως λιγότερο ευαίσθητη σε ακραίες τιμές.</translation>
    </message>
    <message>
        <source>Bicor Correlation Value</source>
        <translation>Τιμή συσχέτισης Bicor</translation>
    </message>
    <message>
        <source>%1 is a median-based measure of similarity between samples.</source>
        <translation>Η %1 είναι ένα μέτρο ομοιότητας μεταξύ δειγμάτων που βασίζεται στη διάμεσο.</translation>
    </message>
    <message>
        <source>Jaccard</source>
        <translation>Jaccard</translation>
    </message>
    <message>
        <source>The Jaccard Index is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Ο δείκτης Jaccard είναι ένα στατιστικό μέγεθος για την εκτίμηση της ομοιότητας και της ποικιλομορφίας συνόλων δειγμάτων.</translation>
    </message>
    <message>
        <source>Jaccard Correlation Value</source>
        <translation>Τιμή συσχέτισης Jaccard</translation>
    </message>
    <message>
        <source>The %1 is a statistic used for gauging the similarity and diversity of sample sets.</source>
        <translation>Ο %1 είναι ένα στατιστικό μέγεθος για την εκτίμηση της ομοιότητας και της ποικιλομορφίας συνόλων δειγμάτων.</translation>
    </message>
    <message>
        <source>Jaccard Index</source>
        <translation>Δείκτης Jaccard</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient</source>
        <translation>Απλός συντελεστής ταύτισης</translation>
    </message>
    <message>
        <source>The Simple Matching Coefficient is a statistic used for gauging the similarity and diversity of sample sets. It is identical to Jaccard, except that it counts mutual absence.</source>
        <translation>Ο απλός συντελεστής ταύτισης είναι ένα στατιστικό μέγεθος για την εκτίμηση της ομοιότητας και της ποικιλομορφίας συνόλων δειγμάτων. Είναι ίδιος με τον δείκτη Jaccard, με τη διαφορά ότι μετρά και την αμοιβαία απουσία.</translation>
    </message>
    <message>
        <source>Simple Matching Coefficient (SMC)</source>
        <translation>Απλός συντελεστής ταύτισης (SMC)</translation>
    </message>
    <message>
        <source>Data Value</source>
        <translation>Τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Strongest Data Column</source>
        <translation>Ισχυρότερη στήλη δεδομένων</translation>
    </message>
    <message>
        <source>Mean Data Value</source>
        <translation>Μέση τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Minimum Data Value</source>
        <translation>Ελάχιστη τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Maximum Data Value</source>
        <translation>Μέγιστη τιμή δεδομένων</translation>
    </message>
    <message>
        <source>Variance</source>
        <translation>Διακύμανση</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Τυπική απόκλιση</translation>
    </message>
    <message>
        <source>Coefficient of Variation</source>
        <translation>Συντελεστής μεταβλητότητας</translation>
    </message>
    <message>
        <source>Absolute %1</source>
        <translation>Απόλυτη %1</translation>
    </message>
    <message>
        <source>Data</source>
        <translation>Δεδομένα</translation>
    </message>
    <message>
        <source>Correlation Values</source>
        <translation>Τιμές συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation CSV File</source>
        <translation>Αρχείο CSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation CSV Files</source>
        <translation>Αρχεία CSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation TSV File</source>
        <translation>Αρχείο TSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation TSV Files</source>
        <translation>Αρχεία TSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation SSV File</source>
        <translation>Αρχείο SSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation SSV Files</source>
        <translation>Αρχεία SSV συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation Excel File</source>
        <translation>Αρχείο Excel συσχέτισης</translation>
    </message>
    <message>
        <source>Correlation Excel Files</source>
        <translation>Αρχεία Excel συσχέτισης</translation>
    </message>
    <message>
        <source>Correlating</source>
        <translation>Υπολογισμός συσχέτισης</translation>
    </message>
    <message>
        <source>Clustering</source>
        <translation>Ομαδοποίηση</translation>
    </message>
    <message>
        <source>Import Column Annotations</source>
        <translation>Εισαγωγή σχολιασμών στηλών</translation>
    </message>
    <message>
        <source>Import Column Annotation</source>
        <translation>Εισαγωγή σχολιασμού στήλης</translation>
    </message>
    <message>
        <source>Importing Column Annotations</source>
        <translation>Εισαγωγή σχολιασμών στηλών</translation>
    </message>
    <message>
        <source>Importing Column Annotation</source>
        <translation>Εισαγωγή σχολιασμού στήλης</translation>
    </message>
    <message numerus="yes">
        <source>%n Annotations Imported</source>
        <translation>
            <numerusform>Εισήχθη %n σχολιασμός</numerusform>
            <numerusform>Εισήχθησαν %n σχολιασμοί</numerusform>
        </translation>
    </message>
    <message>
        <source>Annotation %1 Imported</source>
        <translation>Εισήχθη ο σχολιασμός %1</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Χαρακτηριστικά</translation>
    </message>
    <message>
        <source>Normalisation</source>
        <translation>Κανονικοποίηση</translation>
    </message>
    <message>
        <source>Correlation</source>
        <translation>Συσχέτιση</translation>
    </message>
    <message>
        <source>Building Graph</source>
        <translation>Κατασκευή γράφου</translation>
    </message>
    <message>
        <source>Sorting Columns</source>
        <translation>Ταξινόμηση στηλών</translation>
    </message>
</context>
</TS>
