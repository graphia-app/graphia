/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

pragma ComponentBehavior: Bound

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
    flags: Constants.defaultWindowFlags
    color: palette.window
    width: 900
    height: 475
    minimumWidth: 900
    minimumHeight: 475

    component DefaultVisualisation: ColumnLayout
    {
        property string attributeName
        property string value
    }

    property Document document
    property string transformExpression
    property var defaultVisualisations

    property var _transform: undefined
    property int _numParameters: root._transform?.parameterNames.length ?? 0
    property int _numAttributeParameters: root._transform?.attributeParameterNames.length ?? 0
    property int _numDefaultVisualisations: Object.keys(root._transform?.defaultVisualisations ?? {}).length

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
            return favouriteTransformsAsArray().includes(transform);
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
                        let value = model.displayName;

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
                        {
                            return model.data(left, modelRole("displayName")) <
                                model.data(right, modelRole("displayName"));
                        }

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

                            root._transform = root.document.transform(selectedValue);
                            lhsAttributeList.model = root.document.availableAttributesModel(
                                root._transform.elementType, ValueType.All,
                                AttributeFlag.DisableDuringTransform);
                            valueRadioButton.checked = true;
                            rhsAttributeList.model = undefined;

                            if(root._transform.parameterNames !== undefined)
                                parametersRepeater.model = root._transform.parameterNames;

                            if(root._transform.attributeParameterNames !== undefined)
                                attributeParametersRepeater.model = root._transform.attributeParameterNames;

                            if(root._transform.defaultVisualisations !== undefined)
                                visualisationsRepeater.model = Object.keys(root._transform.defaultVisualisations);
                        }

                        attributeDescription.clear();
                        root.updateTransformExpression();
                    }

                    onAccepted:
                    {
                        if(root.document.graphTransformIsValid(root.transformExpression))
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

                    onClicked: function()
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
                    visible: root._transform !== undefined
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

                        text: root._transform !== undefined ?
                            "<h3>" + root._transform.displayName + "</h3>" + root._transform.description : ""
                    }

                    Image
                    {
                        Layout.preferredHeight: 64
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 64
                        source: root._transform?.image ?? ""
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

                    text: root._transform !== undefined && root._numParameters === 0 && root._numAttributeParameters === 0 ?
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
                                    enabled: root._transform !== undefined && root._transform.requiresCondition
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
                                            prettifyFunction: (name) => root.document.attributeDisplayName(name)

                                            onSelectedValueChanged:
                                            {
                                                let attribute = root.document.attribute(selectedValue);
                                                if(currentIndexIsSelectable && attribute.isValid)
                                                {
                                                    opList.updateModel(attribute.ops);

                                                    let parameterData = root.document.attribute(lhsAttributeList.selectedValue);
                                                    rhs.configure(parameterData);
                                                }
                                                else
                                                {
                                                    opList.updateModel();
                                                    valueParameter.reset();
                                                }

                                                attributeDescription.updateFor(lhsAttributeList.selectedValue);
                                                root.updateTransformExpression();
                                            }

                                            AttributeListSortMenu { attributeList: lhsAttributeList }
                                        }

                                        ListBox
                                        {
                                            id: opList
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            Layout.preferredWidth: 150

                                            onSelectedValueChanged: { root.updateTransformExpression(); }

                                            Component { id: modelComponent; ListModel {} }

                                            function updateModel(ops)
                                            {
                                                if(ops === undefined)
                                                {
                                                    opList.model = undefined;
                                                    return;
                                                }

                                                let newModel = modelComponent.createObject() as ListModel;

                                                for(const op of ops)
                                                {
                                                    let item =
                                                    {
                                                        display: root.document.opDisplayText(op).name,
                                                        value: op,
                                                        unary: root.document.opIsUnary(op)
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
                                                onCheckedChanged: { root.updateTransformExpression(); }
                                            }

                                            TransformParameter
                                            {
                                                id: valueParameter
                                                Layout.fillWidth: true

                                                document: root.document

                                                enabled: valueRadioButton.checked
                                                updateValueImmediately: true
                                                direction: Qt.Vertical
                                                fillWidth: true
                                                onValueChanged: { root.updateTransformExpression(); }
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
                                                prettifyFunction: (name) => root.document.attributeDisplayName(name)

                                                enabled: attributeRadioButton.checked
                                                onSelectedValueChanged:
                                                {
                                                    attributeDescription.updateFor(rhsAttributeList.selectedValue);
                                                    root.updateTransformExpression();
                                                }

                                                AttributeListSortMenu { attributeList: rhsAttributeList }
                                            }

                                            function configure(parameterData)
                                            {
                                                valueParameter.configure(parameterData);

                                                if(parameterData.valueType !== undefined)
                                                {
                                                    rhsAttributeList.model = root.document.availableAttributesModel(
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
                                                    if(!root.document.attribute(rhsAttributeList.selectedValue).isValid)
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
                                            let parameterData = root.document.attribute(attributeName);

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
                                    enabled: root._transform !== undefined && root._numAttributeParameters > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _attributeNames

                                    Repeater
                                    {
                                        id: attributeParametersRepeater

                                        RowLayout
                                        {
                                            id: attributeParametersLayout

                                            required property string modelData

                                            property var parameterData:
                                            {
                                                return root._transform.attributeParameters.find((element) =>
                                                {
                                                    return element.name === attributeParametersLayout.modelData;
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
                                                    text: attributeParametersLayout.modelData
                                                }

                                                Text
                                                {
                                                    width: parent.width
                                                    text: attributeParametersLayout.parameterData.description
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
                                                prettifyFunction: (name) => root.document.attributeDisplayName(name)

                                                onSelectedValueChanged:
                                                {
                                                    if(selectedValue !== undefined)
                                                        attributeParameters._attributeNames[attributeParametersLayout.modelData] = selectedValue;

                                                    root.updateTransformExpression();
                                                }

                                                AttributeListSortMenu { attributeList: attributeParameterAttributeList }
                                            }

                                            Component.onCompleted:
                                            {
                                                attributeParameterAttributeList.model = root.document.availableAttributesModel(
                                                    attributeParametersLayout.parameterData.elementType,
                                                    attributeParametersLayout.parameterData.valueType,
                                                    AttributeFlag.DisableDuringTransform);
                                            }
                                        }
                                    }
                                }

                                ColumnLayout
                                {
                                    id: parameters
                                    enabled: root._transform !== undefined && root._numParameters > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _values

                                    Repeater
                                    {
                                        id: parametersRepeater

                                        RowLayout
                                        {
                                            id: parametersInfoLayout

                                            required property string modelData
                                            property var parameterData: root._transform.parameters[parametersInfoLayout.modelData]

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
                                                    text: parametersInfoLayout.parameterData.displayName
                                                }

                                                Text
                                                {
                                                    width: parent.width
                                                    text: parametersInfoLayout.parameterData.description
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
                                                let transformParameter = TransformConfig.createTransformParameter(root.document,
                                                    controlPlaceholder, parametersInfoLayout.parameterData, root.updateTransformExpression);
                                                transformParameter.updateValueImmediately = true;
                                                transformParameter.direction = Qt.Vertical;
                                                parameters._values[parametersInfoLayout.modelData] = transformParameter;
                                            }
                                        }
                                    }

                                    function updateValues()
                                    {
                                        for(const transformParameter of Object.values(parameters._values ?? {}))
                                            transformParameter.updateValue();

                                        visualisations.update();
                                    }

                                    function valueOf(parameterName)
                                    {
                                        if(parameters._values === undefined)
                                            return "\"\"";

                                        let transformParameter = parameters._values[parameterName];

                                        if(transformParameter === undefined)
                                            return "\"\"";

                                        return transformParameter.value;
                                    }
                                }

                                ColumnLayout
                                {
                                    id: visualisations
                                    enabled: root._transform !== undefined && root._numDefaultVisualisations > 0
                                    visible: enabled

                                    Layout.fillWidth: visible
                                    spacing: 20

                                    property var _visualisations

                                    function update()
                                    {
                                        visualisations._visualisations = {};

                                        for(let i = 0; i < visualisationsRepeater.count; i++)
                                        {
                                            let item = visualisationsRepeater.itemAt(i) as DefaultVisualisation;
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

                                        DefaultVisualisation
                                        {
                                            id: visualisationLayout

                                            required property string modelData

                                            attributeName:
                                            {
                                                return visualisations.resolvedAttributeName(visualisationLayout.modelData);
                                            }

                                            property var visualisation: root._transform?.defaultVisualisations[visualisationLayout.modelData]

                                            RowLayout
                                            {
                                                Layout.fillWidth: true

                                                Label
                                                {
                                                    font.italic: visualisationLayout.attributeName.length === 0
                                                    color: palette.buttonText
                                                    text:
                                                    {
                                                        if(visualisationLayout.attributeName.length === 0)
                                                            return qsTr("Invalid Attribute Name");

                                                        // Not named by a parameter, so it's the transform's own attribute
                                                        if(visualisationLayout.attributeName === visualisationLayout.modelData &&
                                                            visualisationLayout.visualisation !== undefined)
                                                        {
                                                            return visualisationLayout.visualisation.displayName;
                                                        }

                                                        return root.document.attributeDisplayName(visualisationLayout.attributeName);
                                                    }
                                                }

                                                ComboBox
                                                {
                                                    id: visualisationsComboBox
                                                    enabled: visualisationLayout.attributeName.length > 0
                                                    editable: false
                                                    textRole: "displayName"
                                                    valueRole: "name"
                                                }

                                                Item { Layout.fillWidth: true }
                                            }

                                            // Empty for no visualisation
                                            value: visualisationsComboBox.currentValue ?? ""

                                            onValueChanged: { visualisations.update(); }

                                            Component.onCompleted:
                                            {
                                                let visualisationChannelNames = root.document.availableVisualisationChannelNames(
                                                    visualisationLayout.visualisation.elementType, visualisationLayout.visualisation.valueType);
                                                visualisationsComboBox.model = [{name: "", displayName: qsTr("None")}].concat(
                                                    visualisationChannelNames.map(channelName => ({name: channelName,
                                                    displayName: root.document.visualisationChannelDisplayName(channelName)})));
                                                visualisationsComboBox.currentIndex = visualisationsComboBox.indexOfValue(
                                                    visualisationLayout.visualisation.channelName);
                                            }
                                        }
                                    }

                                    function resolvedAttributeName(attributeName)
                                    {
                                        let noQuotesAttributeName = attributeName.replace(/"/g, "");
                                        if(root._transform !== undefined &&
                                            root._transform.parameters.hasOwnProperty(noQuotesAttributeName) &&
                                            root._transform.parameters[noQuotesAttributeName].valueType === ValueType.String)
                                        {
                                            // If the attribute name matches a parameter name, the parameter
                                            // is (probably) the name of a new attribute, so use its value
                                            return root.document.writtenAttributeName(parameters.valueOf(noQuotesAttributeName));
                                        }

                                        return attributeName;
                                    }

                                    function selectedVisualisation(attributeName)
                                    {
                                        if(visualisations._visualisations === undefined)
                                            return "";

                                        attributeName = visualisations.resolvedAttributeName(attributeName);
                                        let visualisationChannelName = visualisations._visualisations[attributeName];

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
                        enabled: { return root.document.graphTransformIsValid(root.transformExpression); }
                        onClicked: function() { root.accept(); }
                    }

                    Button
                    {
                        Layout.alignment: Qt.AlignBottom
                        text: qsTr("Cancel")
                        onClicked: function() { root.reject(); }
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
                root.reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                root.accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function accept()
    {
        if(!root.document.graphTransformIsValid(root.transformExpression))
        {
            console.log("CreateTransformDialog: trying to accept invalid expression");
            return;
        }

        root.accepted();
        root.close();
    }

    function reject()
    {
        root.rejected();
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
            expression += root.document.writtenString(transformsList.selectedValue);

            if(root._numAttributeParameters > 0)
            {
                expression += " using";

                for(let attributeParameterName of root._transform.attributeParameterNames)
                {
                    if(attributeParameterName in attributeParameters._attributeNames)
                        expression += " $" + attributeParameters._attributeNames[attributeParameterName];
                }
            }

            if(root._numParameters > 0)
            {
                expression += " with";

                for(let parameterName of root._transform.parameterNames)
                    expression += " " + root.document.writtenString(parameterName) + " = " + parameters.valueOf(parameterName);
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

        root.transformExpression = expression;
    }

    function updateDefaultVisualisations()
    {
        root.defaultVisualisations = [];

        let resolvedDefaultVisualisations = {};
        for(const [attributeName, defaultVisualisation] of Object.entries(root._transform.defaultVisualisations))
        {
            let resolvedAttributeName = visualisations.resolvedAttributeName(attributeName);
            resolvedDefaultVisualisations[resolvedAttributeName] = defaultVisualisation;
        }

        for(const [attributeName, defaultVisualisation] of Object.entries(resolvedDefaultVisualisations))
        {
            let channelName = visualisations.selectedVisualisation(attributeName);

            if(channelName.length > 0)
            {
                let expression = VisualisationUtils.expressionFor(root.document,
                    attributeName, defaultVisualisation.flags,
                    defaultVisualisation.valueType, channelName);

                root.defaultVisualisations.push(expression);
            }
        }
    }

    onAccepted:
    {
        root.updateTransformExpression();
        root.updateDefaultVisualisations();

        root.document.update([root.transformExpression], root.defaultVisualisations);
    }

    onVisibleChanged:
    {
        root.transformExpression = "";
        root.defaultVisualisations = [];
        transformsList.model = root.document.availableTransforms();

        parametersRepeater.model = undefined;
        parameters._values = {};

        attributeParametersRepeater.model = undefined;
        attributeParameters._attributeNames = {};

        lhsAttributeList.model = rhsAttributeList.model = undefined;
        opList.model = undefined;

        root._transform = undefined;
    }
}
