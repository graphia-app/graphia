function create(visualisation)
{
    this.metaAttributes = visualisation.metaAttributes;
    this.dataField = visualisation.dataField;
    this.channel = visualisation.channel;

    this._elements = [];

    function appendToElements(elements, value)
    {
        var last = elements.length - 1;
        if(last >= 0 && typeof value === "string" && typeof elements[last] === typeof value)
            elements[last] += value;
        else
            elements.push(value);
    }

    appendToElements(this._elements, visualisation.dataField);
    appendToElements(this._elements, ": ");
    appendToElements(this._elements, visualisation.channel);

    this.toComponents =
    function(document, parent)
    {
        var labelText = "";

        function addLabel()
        {
            labelText = labelText.trim();
            Qt.createQmlObject(qsTr("import QtQuick 2.7;" +
                                    "import QtQuick.Controls 1.5;" +
                                    "Label { text: \"%1\"; color: root.textColor }")
                                    .arg(Utils.normaliseWhitespace(labelText)), parent);

            labelText = "";
        }

        this.parameters = [];

        for(var i = 0; i < this._elements.length; i++)
        {
            if(typeof this._elements[i] === "string")
                labelText += this._elements[i];
            else if(typeof this._elements[i] === 'object')
            {}
        }

        if(labelText.length > 0)
            addLabel();
    }
}
