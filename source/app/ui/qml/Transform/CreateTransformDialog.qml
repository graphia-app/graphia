/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import app.graphia 1.0

import ".."
import "TransformConfig.js" as TransformConfig
import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "../Visualisation/VisualisationUtils.js" as VisualisationUtils
import "../AttributeUtils.js" as AttributeUtils

import "../Controls"

Window
{
    id: root

    title: qsTr("Add Transform")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: 900
    height: 350
    minimumWidth: 900
    minimumHeight: 400

    property var document
    property string transformExpression
    property var defaultVisualisations

    property var _transform: undefined
    property int _numParameters: _transform !== undefined ? _transform.parameterNames.length : 0
    property int _numAttributeParameters: _transform !== undefined ? _transform.attributeParameterNames.length : 0
    property int _numDefaultVisualisations: _transform !== undefined ? Object.keys(_transform.defaultVisualisations).length : 0

    Preferences
    {
        section: "misc"
        property alias transformSortOrder: transformsList.ascendingSortOrder
        property alias transformSortBy: transformsList.sortRoleName
        property alias transformAttributeSortOrder: lhsAttributeList.ascendingSortOrder
        property alias transformAttributeSortBy: lhsAttributeList.sortRoleName
    }

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TreeBox
            {
                id: transformsList
                Layout.preferredWidth: 192
                Layout.fillHeight: true

                showSections: sortRoleName !== "display"
                sortRoleName: "category"

                Component.onCompleted:
                {
                    // The role name used to be called "type", so if the user still
                    // has that in their preferences, just hack it to "category"
                    if(sortRoleName === "type")
                        sortRoleName = "category";
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

                        // This is a hack that forces the scrollView to reconsider the size of its
                        // content and avoid cutting the bottom off after some combination of clicking
                        // on transforms in the list. It appears to be related to the Layout.margins
                        // that are enabled when the scrollbar is visible.
                        // See LAYOUT_MARGINS_HACK comment for the Layout.margins in question.
                        scrollView.verticalScrollBarPolicy = Qt.ScrollBarAlwaysOff;
                        scrollView.verticalScrollBarPolicy = Qt.ScrollBarAsNeeded;
                    }

                    description.update();
                    updateTransformExpression();
                }

                onAccepted:
                {
                    if(document.graphTransformIsValid(transformExpression))
                        root.accept();
                }

                TransformListSortMenu { transformsList: transformsList }
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

                text: _transform !== undefined && _numParameters === 0 && _numAttributeParameters === 0 ?
                          qsTr("No Parameters Required") : qsTr("Select A Transform")
            }

            ScrollView
            {
                id: scrollView

                frameVisible: (parameters.enabled || attributeParameters.enabled ||
                               visualisations.enabled) && scrollView.__verticalScrollBar.visible

                visible: parameters.enabled || attributeParameters.enabled || visualisations.enabled ||
                         condition.enabled

                Layout.fillWidth: visible
                Layout.fillHeight: visible

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded

                contentItem: RowLayout
                {
                    width: scrollView.viewport.width

                    ColumnLayout
                    {
                        Layout.fillWidth: true

                        // LAYOUT_MARGINS_HACK
                        Layout.margins: scrollView.frameVisible ? Constants.margin : 0

                        spacing: 20

                        RowLayout
                        {
                            id: condition
                            enabled: _transform !== undefined && _transform.requiresCondition
                            visible: enabled

                            Layout.fillWidth: visible
                            Layout.minimumHeight:
                            {
                                let conditionHeight = scrollView.viewport.height;

                                if(parameters.enabled || visualisations.enabled)
                                    conditionHeight *= 0.5;

                                return Math.max(conditionHeight, 128);
                            }

                            Label
                            {
                                Layout.topMargin: Constants.margin
                                Layout.alignment: Qt.AlignTop
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

                                    description.update();
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
                                ExclusiveGroup { id: rhsGroup }

                                RadioButton
                                {
                                    id: valueRadioButton
                                    Layout.alignment: Qt.AlignTop

                                    checked: true
                                    exclusiveGroup: rhsGroup

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

                                    exclusiveGroup: rhsGroup
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
                                    onSelectedValueChanged: { updateTransformExpression(); }

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

                                delegate: Component
                                {
                                    RowLayout
                                    {
                                        property var parameterData:
                                        {
                                            return _transform.attributeParameters.find(function(element)
                                            {
                                                return element.name === modelData;
                                            });
                                        }

                                        ColumnLayout
                                        {
                                            id: attributeParameterRowLayout
                                            Layout.fillWidth: true

                                            Label
                                            {
                                                Layout.alignment: Qt.AlignTop
                                                font.italic: true
                                                font.bold: true
                                                text: modelData
                                            }

                                            Text
                                            {
                                                Layout.fillWidth: true
                                                text: parameterData.description
                                                textFormat: Text.StyledText
                                                wrapMode: Text.Wrap
                                                elide: Text.ElideRight

                                                PointingCursorOnHoverLink {}
                                                onLinkActivated: Qt.openUrlExternally(link);
                                            }

                                            Item { Layout.fillHeight: true }
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

                                delegate: Component
                                {
                                    RowLayout
                                    {
                                        property var parameterData: _transform.parameters[modelData]

                                        ColumnLayout
                                        {
                                            Layout.fillWidth: true

                                            Label
                                            {
                                                Layout.alignment: Qt.AlignTop
                                                font.italic: true
                                                font.bold: true
                                                text: modelData
                                            }

                                            Text
                                            {
                                                Layout.fillWidth: true
                                                text: parameterData.description
                                                textFormat: Text.StyledText
                                                wrapMode: Text.Wrap
                                                elide: Text.ElideRight

                                                PointingCursorOnHoverLink {}
                                                onLinkActivated: Qt.openUrlExternally(link);
                                            }

                                            Item { Layout.fillHeight: true }
                                        }

                                        Item
                                        {
                                            id: controlPlaceholder
                                            Layout.fillHeight: true

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
                                text: qsTr("Visualisations")
                            }

                            Repeater
                            {
                                id: visualisationsRepeater

                                delegate: Component
                                {
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
                                            let visualisationChannelNames = document.availableVisualisationChannelNames(visualisation.valueType);
                                            visualisationsComboBox.model = ["None"].concat(visualisationChannelNames);
                                            visualisationsComboBox.currentIndex = visualisationsComboBox.model.indexOf(visualisation.channelName);
                                        }
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
        }

        RowLayout
        {
            Layout.fillHeight: true
            Layout.preferredHeight: 64

            Text
            {
                id: description
                Layout.fillWidth: true

                textFormat: Text.StyledText
                wrapMode: Text.WordWrap

                PointingCursorOnHoverLink {}
                onLinkActivated: Qt.openUrlExternally(link);

                function update()
                {
                    text = "";

                    if(_transform !== undefined)
                    {
                        text += _transform.description;

                        if(lhsAttributeList.selectedValue !== undefined)
                        {
                            let parameterData = document.attribute(lhsAttributeList.selectedValue);

                            if(parameterData.description !== undefined)
                                text += "<br><br>" + parameterData.description;
                        }
                    }
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignBottom
                text: qsTr("OK")
                enabled: { return document.graphTransformIsValid(transformExpression); }
                onClicked: { root.accept(); }
            }

            Button
            {
                Layout.alignment: Qt.AlignBottom
                text: qsTr("Cancel")
                onClicked: { root.reject(); }
            }
        }

        Keys.onPressed:
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

                for(let attributeName in attributeParameters._attributeNames)
                    expression += " $" + attributeParameters._attributeNames[attributeName];
            }

            if(_numParameters > 0)
            {
                expression += " with";

                for(let index in _transform.parameterNames)
                {
                    let parameterName = _transform.parameterNames[index];
                    expression += " \"" + parameterName + "\" = " + parameters.valueOf(parameterName);
                }
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
