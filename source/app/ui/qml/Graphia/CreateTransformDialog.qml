/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Graphia.Controls
import Graphia.SharedTypes
import Graphia.Utils

import "TransformConfig.js" as TransformConfig

Window
{
    id: root

    title: qsTr("Add Transform")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    color: palette.window
    width: 900
    height: 475
    minimumWidth: 900
    minimumHeight: 475

    property var document
    property string transformExpression
    property var defaultVisualisations

    property var _transform: undefined
    property int _numParameters: _transform !== undefined ? _transform.parameterNames.length : 0
    property int _numAttributeParameters: _transform !== undefined ? _transform.attributeParameterNames.length : 0
    property int _numDefaultVisualisations: _transform !== undefined ? Object.keys(_transform.defaultVisualisations).length : 0

    Preferences
    {
        id: misc
        section: "misc"

        property alias transformSortOrder: transformsList.ascendingSortOrder
        property alias transformSortBy: transformsList.sortRoleName
        property alias transformAttributeSortOrder: lhsAttributeList.ascendingSortOrder
        property alias transformAttributeSortBy: lhsAttributeList.sortRoleName

        property string favouriteTransforms

        function favouriteTransformsAsArray()
        {
            let a = [];

            if(favouriteTransforms.length > 0)
            {
                try { a = JSON.parse(favouriteTransforms); }
                catch(e) { a = []; }
            }

            return a;
        }

        function transformIsFavourite(transform)
        {
            return favouriteTransformsAsArray().indexOf(transform) >= 0;
        }

        function toggleFavouriteTransform(transform)
        {
            if(typeof transform !== "string")
                return;

            let a = favouriteTransformsAsArray();

            if(!transformIsFavourite(transform))
                a.push(transform);
            else
                a.splice(a.indexOf(transform), 1);

            a = a.filter(i => typeof i === "string");

            favouriteTransforms = JSON.stringify(a);
        }
    }

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        RowLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Constants.spacing

            ColumnLayout
            {
                // Using preferredWidth here doesn't seem to constrain sufficiently, for some reason
                Layout.minimumWidth: 192
                Layout.maximumWidth: 192
                Layout.fillHeight: true

                TreeBox
                {
                    id: transformsList

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    itemTextDelegateFunction: function(model)
                    {
                        let value = model.display;

                        if(model.isFavourite)
                            value = "<font color=\"#F1C40F\">★</font> " + value;

                        return value;
                    }

                    showSections: sortRoleName === "category"
                    sectionRoleName: "category"
                    sortRoleName: "category"

                    sortExpression: function(left, right)
                    {
                        let leftIsFavourite = model.data(left, modelRole("isFavourite"));
                        let rightIsFavourite = model.data(right, modelRole("isFavourite"));

                        // Always sort favourites first
                        if(leftIsFavourite !== rightIsFavourite)
                        {
                            return transformsList.ascendingSortOrder ?
                                leftIsFavourite : rightIsFavourite;
                        }

                        let leftCategory = model.data(left, modelRole("category"));
                        let rightCategory = model.data(right, modelRole("category"));

                        if(transformsList.sortRoleName !== "category" || leftCategory === rightCategory)
                            return model.data(left) < model.data(right);

                        return leftCategory < rightCategory;
                    }

                    onSelectedValueChanged:
                    {
                        if(selectedValue !== undefined)
                        {
                            parametersRepeater.model = [];
                            parameters._values = {};

                            attributeParametersRepeater.model = [];
                            attributeParameters._attributeNames = {};

                            visualisationsRepeater.model = [];
                            visualisations._visualisations = {};

                            root._transform = document.transform(selectedValue);
                            lhsAttributeList.model = document.availableAttributesModel(
                                root._transform.elementType, ValueType.All,
                                AttributeFlag.DisableDuringTransform);
                            valueRadioButton.checked = true;
                            rhsAttributeList.model = undefined;

                            if(_transform.parameterNames !== undefined)
                                parametersRepeater.model = _transform.parameterNames;

                            if(_transform.attributeParameterNames !== undefined)
                                attributeParametersRepeater.model = _transform.attributeParameterNames;

                            if(_transform.defaultVisualisations !== undefined)
                                visualisationsRepeater.model = Object.keys(_transform.defaultVisualisations);
                        }

                        attributeDescription.clear();
                        updateTransformExpression();
                    }

                    onAccepted:
                    {
                        if(document.graphTransformIsValid(transformExpression))
                            root.accept();
                    }

                    TransformListSortMenu { transformsList: transformsList }
                }

                Button
                {
                    Layout.fillWidth: true

                    enabled: transformsList.currentIndex !== null

                    text: misc.transformIsFavourite(transformsList.selectedValue) ?
                        qsTr("Remove Favourite") : qsTr("Add Favourite")

                    onClicked: function(mouse)
                    {
                        let index = transformsList.currentIndex;
                        if(index !== null)
                        {
                            misc.toggleFavouriteTransform(transformsList.selectedValue);
                            transformsList.select(index);
                        }
                    }
                }
            }

            ColumnLayout
            {
                Layout.fillWidth: true
                Layout.fillHeight: true

                RowLayout
                {
                    visible: _transform !== undefined
                    Layout.fillWidth: true
                    Layout.bottomMargin: Constants.spacing

                    Text
                    {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64

                        textFormat: Text.StyledText
                        wrapMode: Text.WordWrap
                        color: palette.buttonText

                        PointingCursorOnHoverLink {}
                        onLinkActivated: function(link) { Qt.openUrlExternally(link); }

                        text: _transform !== undefined ?
                            "<h3>" + _transform.name + "</h3>" + _transform.description : ""
                    }

                    Image
                    {
                        Layout.preferredHeight: 64
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 64
                        source: _transform !== undefined ? _transform.image : ""
                    }
                }

                Label
                {
                    visible: !scrollView.visible
                    Layout.fillWidth: visible
                    Layout.fillHeight: visible

                    horizontalAlignment: Qt.AlignCenter
                    verticalAlignment: Qt.AlignVCenter
                    font.pixelSize: 16
                    font.italic: true
                    color: palette.buttonText

                    text: _transform !== undefined && _numParameters === 0 && _numAttributeParameters === 0 ?
                              qsTr("No Parameters Required") : qsTr("Select A Transform")
                }

                Item
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: parameters.enabled || attributeParameters.enabled ||
                        visualisations.enabled || condition.enabled

                    ScrollView
                    {
                        id: scrollView

                        anchors.fill: parent

                        property bool needsFrame: contentHeight > availableHeight
                        readonly property real frameMargin: needsFrame ? Constants.margin : 0
                        readonly property real scrollBarWidth: ScrollBar.vertical.size < 1 ? ScrollBar.vertical.width : 0

                        Component.onCompleted:
                        {
                            // contentItem is the Flickable; only clip when required
                            contentItem.clip = Qt.binding(() => scrollView.needsFrame);

                            // Make the scrolling behaviour more desktop-y
                            contentItem.boundsBehavior = Flickable.StopAtBounds;
                            contentItem.flickableDirection = Flickable.VerticalFlick;
                        }

                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded
                        ScrollBar.vertical.visible: ScrollBar.vertical.size < 1.0 // So that it's invisible to mouse clicks

                        contentHeight: parametersLayout.implicitHeight

                        RowLayout
                        {
                            id: parametersLayout
                            width: scrollView.width

                            ColumnLayout
                            {
                                Layout.fillWidth: true
                                Layout.leftMargin: scrollView.frameMargin
                                Layout.rightMargin: scrollView.frameMargin + scrollView.scrollBarWidth
                                Layout.topMargin: scrollView.frameMargin
                                Layout.bottomMargin: scrollView.frameMargin

                                spacing: 20

                                ColumnLayout
                                {
                                    id: condition
                                    enabled: _transform !== undefined && _transform.requiresCondition
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    Layout.minimumHeight:
                                    {
                                        let conditionHeight = scrollView.height;

                                        if(parameters.enabled || visualisations.enabled)
                                            conditionHeight *= 0.5;

                                        return Math.max(conditionHeight, 128);
                                    }

                                    RowLayout
                                    {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true

                                        Label
                                        {
                                            Layout.topMargin: Constants.margin
                                            Layout.alignment: Qt.AlignTop
                                            color: palette.buttonText
                                            text: qsTr("where")
                                        }

                                        TreeBox
                                        {
                                            id: lhsAttributeList
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true

                                            showSections: sortRoleName !== "display"
                                            showSearch: true
                                            showParentGuide: true
                                            sortRoleName: "userDefined"
                                            prettifyFunction: AttributeUtils.prettify

                                            onSelectedValueChanged:
                                            {
                                                let attribute = document.attribute(selectedValue);
                                                if(currentIndexIsSelectable && attribute.isValid)
                                                {
                                                    opList.updateModel(attribute.ops);

                                                    let parameterData = document.attribute(lhsAttributeList.selectedValue);
                                                    rhs.configure(parameterData);
                                                }
                                                else
                                                {
                                                    opList.updateModel();
                                                    valueParameter.reset();
                                                }

                                                attributeDescription.updateFor(lhsAttributeList.selectedValue);
                                                updateTransformExpression();
                                            }

                                            AttributeListSortMenu { attributeList: lhsAttributeList }
                                        }

                                        ListBox
                                        {
                                            id: opList
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredWidth: 150

                                            onSelectedValueChanged: { updateTransformExpression(); }

                                            Component { id: modelComponent; ListModel {} }

                                            function updateModel(ops)
                                            {
                                                if(ops === undefined)
                                                {
                                                    opList.model = undefined;
                                                    return;
                                                }

                                                let newModel = modelComponent.createObject();

                                                for(let i = 0; i < ops.length; i++)
                                                {
                                                    let item =
                                                    {
                                                        display: TransformConfig.sanitiseOp(ops[i]),
                                                        value: ops[i],
                                                        unary: document.opIsUnary(ops[i])
                                                    };

                                                    newModel.append(item);
                                                }

                                                opList.model = newModel;
                                            }
                                        }

                                        GridLayout
                                        {
                                            id: rhs

                                            Layout.fillWidth: true
                                            Layout.fillHeight: true

                                            enabled: opList.selectedValue !== undefined &&
                                                     !opList.selectedValue.unary

                                            columns: 2
                                            ButtonGroup { buttons: [valueRadioButton, attributeRadioButton] }

                                            RadioButton
                                            {
                                                id: valueRadioButton
                                                Layout.alignment: Qt.AlignTop

                                                checked: true

                                                // This effectively handles
                                                // attributeRadioButton::onCheckedChanged too
                                                onCheckedChanged: { updateTransformExpression(); }
                                            }

                                            TransformParameter
                                            {
                                                id: valueParameter
                                                Layout.fillWidth: true

                                                enabled: valueRadioButton.checked
                                                updateValueImmediately: true
                                                direction: Qt.Vertical
                                                fillWidth: true
                                                onValueChanged: { updateTransformExpression(); }
                                            }

                                            RadioButton
                                            {
                                                id: attributeRadioButton
                                                Layout.alignment: Qt.AlignTop
                                            }

                                            TreeBox
                                            {
                                                id: rhsAttributeList
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true

                                                showSections: lhsAttributeList.showSections
                                                showSearch: true
                                                showParentGuide: true
                                                sortRoleName: lhsAttributeList.sortRoleName
                                                prettifyFunction: AttributeUtils.prettify

                                                enabled: attributeRadioButton.checked
                                                onSelectedValueChanged:
                                                {
                                                    attributeDescription.updateFor(rhsAttributeList.selectedValue);
                                                    updateTransformExpression();
                                                }

                                                AttributeListSortMenu { attributeList: rhsAttributeList }
                                            }

                                            function configure(parameterData)
                                            {
                                                valueParameter.configure(parameterData);

                                                if(parameterData.valueType !== undefined)
                                                {
                                                    rhsAttributeList.model = document.availableAttributesModel(
                                                        root._transform.elementType, parameterData.valueType,
                                                        AttributeFlag.DisableDuringTransform);
                                                }
                                                else
                                                    rhsAttributeList.model = undefined;
                                            }

                                            function value()
                                            {
                                                if(valueRadioButton.checked)
                                                    return valueParameter.value;

                                                if(attributeRadioButton.checked)
                                                {
                                                    if(!document.attribute(rhsAttributeList.selectedValue).isValid)
                                                        return "";

                                                    return "$" + rhsAttributeList.selectedValue;
                                                }

                                                return "";
                                            }
                                        }
                                    }

                                    Text
                                    {
                                        id: attributeDescription
                                        visible: text.length > 0

                                        Layout.fillWidth: true

                                        textFormat: Text.StyledText
                                        wrapMode: Text.WordWrap
                                        color: palette.buttonText

                                        PointingCursorOnHoverLink {}
                                        onLinkActivated: function(link) { Qt.openUrlExternally(link); }

                                        function clear()
                                        {
                                            attributeDescription.text = "";
                                        }

                                        function updateFor(attributeName)
                                        {
                                            let parameterData = document.attribute(attributeName);

                                            if(parameterData.isValid && parameterData.description !== undefined)
                                                attributeDescription.text = parameterData.description;
                                            else
                                                attributeDescription.clear();
                                        }
                                    }
                                }

                                ColumnLayout
                                {
                                    id: attributeParameters
                                    enabled: _transform !== undefined && _numAttributeParameters > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _attributeNames

                                    Repeater
                                    {
                                        id: attributeParametersRepeater

                                        RowLayout
                                        {
                                            property var parameterData:
                                            {
                                                return _transform.attributeParameters.find(function(element)
                                                {
                                                    return element.name === modelData;
                                                });
                                            }

                                            Column
                                            {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                spacing: Constants.spacing

                                                Label
                                                {
                                                    font.italic: true
                                                    font.bold: true
                                                    color: palette.buttonText
                                                    text: modelData
                                                }

                                                Text
                                                {
                                                    width: parent.width
                                                    text: parameterData.description
                                                    textFormat: Text.StyledText
                                                    wrapMode: Text.Wrap
                                                    elide: Text.ElideRight
                                                    color: palette.buttonText

                                                    PointingCursorOnHoverLink {}
                                                    onLinkActivated: function(link) { Qt.openUrlExternally(link); }
                                                }
                                            }

                                            TreeBox
                                            {
                                                id: attributeParameterAttributeList
                                                Layout.fillHeight: true
                                                Layout.alignment: Qt.AlignTop
                                                Layout.preferredHeight: 110
                                                Layout.preferredWidth: 250

                                                showSections: sortRoleName !== "display"
                                                showSearch: true
                                                showParentGuide: true
                                                sortRoleName: "userDefined"
                                                prettifyFunction: AttributeUtils.prettify

                                                onSelectedValueChanged:
                                                {
                                                    if(selectedValue !== undefined)
                                                        attributeParameters._attributeNames[modelData] = selectedValue;

                                                    updateTransformExpression();
                                                }

                                                AttributeListSortMenu { attributeList: attributeParameterAttributeList }
                                            }

                                            Component.onCompleted:
                                            {
                                                attributeParameterAttributeList.model = document.availableAttributesModel(
                                                    parameterData.elementType, parameterData.valueType,
                                                    AttributeFlag.DisableDuringTransform);
                                            }
                                        }
                                    }
                                }

                                ColumnLayout
                                {
                                    id: parameters
                                    enabled: _transform !== undefined && _numParameters > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _values

                                    Repeater
                                    {
                                        id: parametersRepeater

                                        RowLayout
                                        {
                                            property var parameterData: _transform.parameters[modelData]

                                            Column
                                            {
                                                Layout.fillWidth: true
                                                Layout.fillHeight: true
                                                spacing: Constants.spacing

                                                Label
                                                {
                                                    font.italic: true
                                                    font.bold: true
                                                    color: palette.buttonText
                                                    text: modelData
                                                }

                                                Text
                                                {
                                                    width: parent.width
                                                    text: parameterData.description
                                                    textFormat: Text.StyledText
                                                    wrapMode: Text.Wrap
                                                    elide: Text.ElideRight
                                                    color: palette.buttonText

                                                    PointingCursorOnHoverLink {}
                                                    onLinkActivated: function(link) { Qt.openUrlExternally(link); }
                                                }
                                            }

                                            Item
                                            {
                                                id: controlPlaceholder

                                                implicitWidth: childrenRect.width
                                                implicitHeight: childrenRect.height
                                            }

                                            Component.onCompleted:
                                            {
                                                let transformParameter = TransformConfig.createTransformParameter(document,
                                                    controlPlaceholder, parameterData, updateTransformExpression);
                                                transformParameter.updateValueImmediately = true;
                                                transformParameter.direction = Qt.Vertical;
                                                parameters._values[modelData] = transformParameter;
                                            }
                                        }
                                    }

                                    function updateValues()
                                    {
                                        for(let parameterName in _values)
                                            _values[parameterName].updateValue();

                                        visualisations.update();
                                    }

                                    function valueOf(parameterName)
                                    {
                                        if(_values === undefined)
                                            return "\"\"";

                                        let transformParameter = _values[parameterName];

                                        if(transformParameter === undefined)
                                            return "\"\"";

                                        return transformParameter.value;
                                    }
                                }

                                ColumnLayout
                                {
                                    id: visualisations
                                    enabled: _transform !== undefined && _numDefaultVisualisations > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _visualisations

                                    function update()
                                    {
                                        visualisations._visualisations = {};

                                        for(let i = 0; i < visualisationsRepeater.count; i++)
                                        {
                                            let item = visualisationsRepeater.itemAt(i);
                                            if(item === null)
                                            {
                                                // Not sure why this happens, but it
                                                // does, so avoid the inevitable warning
                                                continue;
                                            }

                                            visualisations._visualisations[item.attributeName] = item.value;
                                        }
                                    }

                                    Label
                                    {
                                        Layout.alignment: Qt.AlignTop
                                        font.italic: true
                                        font.bold: true
                                        color: palette.buttonText
                                        text: qsTr("Visualisations")
                                    }

                                    Repeater
                                    {
                                        id: visualisationsRepeater

                                        ColumnLayout
                                        {
                                            property string attributeName:
                                            {
                                                return visualisations.resolvedAttributeName(modelData);
                                            }

                                            property var visualisation: _transform !== undefined ?
                                                _transform.defaultVisualisations[modelData] : undefined

                                            RowLayout
                                            {
                                                Layout.fillWidth: true

                                                Label
                                                {
                                                    font.italic: attributeName.length === 0
                                                    color: palette.buttonText
                                                    text: attributeName.length > 0 ?
                                                        AttributeUtils.prettify(attributeName) :
                                                        qsTr("Invalid Attribute Name")
                                                }

                                                ComboBox
                                                {
                                                    id: visualisationsComboBox
                                                    enabled: attributeName.length > 0
                                                    editable: false
                                                }

                                                Item { Layout.fillWidth: true }
                                            }

                                            property string value:
                                            {
                                                if(visualisationsComboBox.currentText === "None")
                                                    return "";

                                                return visualisationsComboBox.currentText;
                                            }

                                            onValueChanged: { visualisations.update(); }

                                            Component.onCompleted:
                                            {
                                                let visualisationChannelNames = document.availableVisualisationChannelNames(
                                                    visualisation.elementType, visualisation.valueType);
                                                visualisationsComboBox.model = ["None"].concat(visualisationChannelNames);
                                                visualisationsComboBox.currentIndex = visualisationsComboBox.model.indexOf(visualisation.channelName);
                                            }
                                        }
                                    }

                                    function resolvedAttributeName(attributeName)
                                    {
                                        let noQuotesAttributeName = attributeName.replace(/"/g, "");
                                        if(_transform !== undefined &&
                                            _transform.parameters.hasOwnProperty(noQuotesAttributeName) &&
                                            _transform.parameters[noQuotesAttributeName].valueType === ValueType.String)
                                        {
                                            // If the attribute name matches a parameter name, the parameter
                                            // is (probably) the name of a new attribute, so use its value
                                            return parameters.valueOf(noQuotesAttributeName);
                                        }

                                        return attributeName;
                                    }

                                    function selectedVisualisation(attributeName)
                                    {
                                        if(_visualisations === undefined)
                                            return "";

                                        attributeName = resolvedAttributeName(attributeName);
                                        let visualisationChannelName = _visualisations[attributeName];

                                        if(visualisationChannelName === undefined)
                                            return "";

                                        return visualisationChannelName;
                                    }
                                }
                            }
                        }
                    }

                    Outline
                    {
                        anchors.fill: parent
                        outlineVisible: scrollView.needsFrame
                    }
                }

                RowLayout
                {
                    Layout.fillWidth: true

                    Item { Layout.fillWidth: true }

                    Button
                    {
                        Layout.alignment: Qt.AlignBottom
                        text: qsTr("OK")
                        enabled: { return document.graphTransformIsValid(transformExpression); }
                        onClicked: function(mouse) { root.accept(); }
                    }

                    Button
                    {
                        Layout.alignment: Qt.AlignBottom
                        text: qsTr("Cancel")
                        onClicked: function(mouse) { root.reject(); }
                    }
                }
            }
        }

        Keys.onPressed: function(event)
        {
            event.accepted = true;
            switch(event.key)
            {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function accept()
    {
        if(!document.graphTransformIsValid(transformExpression))
        {
            console.log("CreateTransformDialog: trying to accept invalid expression");
            return;
        }

        accepted();
        root.close();
    }

    function reject()
    {
        rejected();
        root.close();
    }

    signal accepted()
    signal rejected()

    function updateTransformExpression()
    {
        parameters.updateValues();

        let expression = "";

        if(transformsList.selectedValue !== undefined)
        {
            expression += "\"" + transformsList.selectedValue + "\"";

            if(_numAttributeParameters > 0)
            {
                expression += " using";

                for(let attributeParameterName of _transform.attributeParameterNames)
                {
                    if(attributeParameterName in attributeParameters._attributeNames)
                        expression += " $" + attributeParameters._attributeNames[attributeParameterName];
                }
            }

            if(_numParameters > 0)
            {
                expression += " with";

                for(let parameterName of _transform.parameterNames)
                    expression += " \"" + parameterName + "\" = " + parameters.valueOf(parameterName);
            }

            if(lhsAttributeList.selectedValue !== undefined)
            {
                expression += " where $" + lhsAttributeList.selectedValue;

                if(opList.selectedValue !== undefined)
                {
                    expression += " " + opList.selectedValue.value;
                    let rhsValue = rhs.value();

                    if(!opList.selectedValue.unary && rhsValue.length > 0)
                        expression += " " + rhsValue;
                }
            }
        }

        transformExpression = expression;
    }

    function updateDefaultVisualisations()
    {
        defaultVisualisations = [];

        let resolvedDefaultVisualisations = {};
        Object.keys(_transform.defaultVisualisations).forEach(function(attributeName)
        {
            let resolvedAttributeName = visualisations.resolvedAttributeName(attributeName);
            resolvedDefaultVisualisations[resolvedAttributeName] =
                _transform.defaultVisualisations[attributeName];
        });

        Object.keys(resolvedDefaultVisualisations).forEach(function(attributeName)
        {
            let defaultVisualisation = resolvedDefaultVisualisations[attributeName];
            let channelName = visualisations.selectedVisualisation(attributeName);

            if(channelName.length > 0)
            {
                let expression = VisualisationUtils.expressionFor(document,
                    attributeName, defaultVisualisation.flags,
                    defaultVisualisation.valueType, channelName);

                defaultVisualisations.push(expression);
            }
        });
    }

    onAccepted:
    {
        updateTransformExpression();
        updateDefaultVisualisations();

        document.update([transformExpression], defaultVisualisations);
    }

    onVisibleChanged:
    {
        transformExpression.text = "";
        defaultVisualisations = [];
        transformsList.model = document.availableTransforms();

        parametersRepeater.model = undefined;
        parameters._values = {};

        attributeParametersRepeater.model = undefined;
        attributeParameters._attributeNames = {};

        lhsAttributeList.model = rhsAttributeList.model = undefined;
        opList.model = undefined;

        _transform = undefined;
    }
}
