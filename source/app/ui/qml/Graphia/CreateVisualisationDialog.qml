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

Window
{
    id: root

    title: qsTr("Add Visualisation")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    color: palette.window
    width: 640
    height: 350
    minimumWidth: 640
    minimumHeight: 350

    property var document
    property var visualisationExpressions: []

    property bool visualisationExpressionsValid:
    {
        if(visualisationExpressions.length === 0)
            return false;

        for(let visualisationExpression of visualisationExpressions)
        {
            if(!document.visualisationIsValid(visualisationExpression))
                return false;
        }

        return true;
    }

    Preferences
    {
        section: "misc"
        property alias visualisationAttributeSortOrder: attributeList.ascendingSortOrder
        property alias visualisationAttributeSortBy: attributeList.sortRoleName
    }

    Preferences
    {
        id: visuals
        section: "visuals"
        property string defaultGradient
        property string defaultPalette
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        RowLayout
        {
            TreeBox
            {
                id: attributeList
                Layout.fillWidth: true
                Layout.fillHeight: true

                showSections: sortRoleName !== "display"
                showSearch: true
                showParentGuide: true
                sortRoleName: "elementType"
                prettifyFunction: AttributeUtils.prettify

                onSelectedValueChanged:
                {
                    let attribute = document.attribute(selectedValue);

                    if(currentIndexIsSelectable && attribute.isValid)
                    {
                        channelList.model = document.availableVisualisationChannelNames(
                            attribute.elementType, attribute.valueType);
                    }
                    else if(channelList.model)
                        channelList.model = null;

                    description.update();
                    channelIndicator.update();
                    updateVisualisationExpressions();
                }

                AttributeListSortMenu { attributeList: attributeList }
            }

            ListBox
            {
                id: channelList
                Layout.fillWidth: true
                Layout.fillHeight: true

                allowMultipleSelection: true

                onSelectedValuesChanged:
                {
                    description.update();
                    channelIndicator.update();
                    updateVisualisationExpressions();
                }

                onAccepted: { root.accept(); }
            }

            Label
            {
                visible: !descriptionItem.visible
                Layout.minimumWidth: descriptionItem.Layout.minimumWidth
                Layout.maximumWidth: descriptionItem.Layout.maximumWidth
                Layout.fillHeight: true

                horizontalAlignment: Qt.AlignCenter
                verticalAlignment: Qt.AlignVCenter
                font.pixelSize: 16
                font.italic: true
                wrapMode: Text.WordWrap

                text: qsTr("Select an Attribute and Channel")
            }

            Item
            {
                id: descriptionItem

                visible: description.text.length > 0

                Layout.minimumWidth: 250
                Layout.maximumWidth: 250
                Layout.fillHeight: true

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

                    contentHeight: descriptionLayout.implicitHeight

                    Column
                    {
                        id: descriptionLayout

                        width: scrollView.width
                        spacing: Constants.spacing

                        leftPadding: scrollView.frameMargin
                        rightPadding: scrollView.frameMargin + scrollView.scrollBarWidth
                        topPadding: scrollView.frameMargin
                        bottomPadding: scrollView.frameMargin

                        property real _contentWidth: width - (leftPadding + rightPadding)

                        Text
                        {
                            id: description

                            width: descriptionLayout._contentWidth

                            textFormat: Text.StyledText
                            wrapMode: Text.WordWrap
                            color: palette.buttonText

                            PointingCursorOnHoverLink {}
                            onLinkActivated: function(link) { Qt.openUrlExternally(link); }

                            function update()
                            {
                                text = "";

                                if(attributeList.selectedValue === undefined || !attributeList.currentIndexIsSelectable)
                                    return;

                                let attribute = document.attribute(attributeList.selectedValue);

                                if(attribute.description === undefined)
                                    return;

                                text += attribute.description;

                                if(channelList.selectedValues === undefined || channelList.selectedValues.length === 0)
                                    return;

                                let visualisationDescriptions = document.visualisationDescription(
                                    attributeList.selectedValue, channelList.selectedValues);

                                visualisationDescriptions.forEach(function(visualisationDescription)
                                {
                                    text += "<br><br>" + visualisationDescription;
                                });
                            }
                        }

                        RowLayout
                        {
                            id: channelIndicator

                            width: descriptionLayout._contentWidth

                            readonly property int _elementSize: 24

                            Layout.fillWidth: true
                            Layout.leftMargin: scrollView.frameMargin
                            Layout.rightMargin: scrollView.frameMargin + scrollView.scrollBarWidth
                            Layout.bottomMargin: scrollView.frameMargin

                            GradientKey
                            {
                                id: gradientKey

                                Layout.fillWidth: true
                                keyHeight: channelIndicator._elementSize

                                configuration: visuals.defaultGradient
                                showLabels: false
                                hoverEnabled: false
                            }

                            PaletteKey
                            {
                                id: paletteKey

                                Layout.fillWidth: true
                                keyHeight: channelIndicator._elementSize

                                configuration: visuals.defaultPalette
                                separateKeys: false
                                hoverEnabled: false
                            }

                            NamedIcon
                            {
                                id: nodeSizeIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "node-size"
                            }

                            NamedIcon
                            {
                                id: edgeSizeIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "edge-size"
                            }

                            NamedIcon
                            {
                                id: textSizeIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "text-size"
                            }

                            NamedIcon
                            {
                                id: textColorIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "text-color"
                            }

                            NamedIcon
                            {
                                id: textIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "format-text-bold"
                            }

                            NamedIcon
                            {
                                id: sharedTextIcon
                                width: channelIndicator._elementSize
                                height: channelIndicator._elementSize

                                iconName: "shared-text"
                            }

                            Item { Layout.fillWidth: !gradientKey.visible && !paletteKey.visible }

                            function update()
                            {
                                gradientKey.visible =
                                    paletteKey.visible =
                                    nodeSizeIcon.visible =
                                    edgeSizeIcon.visible =
                                    textIcon.visible =
                                    false;

                                if(attributeList.selectedValue === undefined || !attributeList.currentIndexIsSelectable)
                                    return;

                                let attribute = document.attribute(attributeList.selectedValue);

                                let colourSelected = channelList.selectedValues.indexOf(qsTr("Colour")) >= 0;
                                let sizeSelected = channelList.selectedValues.indexOf(qsTr("Size")) >= 0;
                                let sharedTextSelected = channelList.selectedValues.indexOf(qsTr("Shared Text")) >= 0;
                                let textSelected = channelList.selectedValues.indexOf(qsTr("Text")) >= 0;
                                let textColourSelected = channelList.selectedValues.indexOf(qsTr("Text Colour")) >= 0;
                                let textSizeSelected = channelList.selectedValues.indexOf(qsTr("Text Size")) >= 0;

                                if(colourSelected)
                                {
                                    if(attribute.valueType === ValueType.Float || attribute.valueType === ValueType.Int)
                                        gradientKey.visible = true;
                                    else
                                        paletteKey.visible = true;
                                }

                                if(sizeSelected)
                                {
                                    nodeSizeIcon.visible = (attribute.elementType === ElementType.Node);
                                    edgeSizeIcon.visible = (attribute.elementType === ElementType.Edge);
                                }

                                sharedTextIcon.visible = sharedTextSelected;
                                textIcon.visible = textSelected;
                                textColorIcon.visible = textColourSelected;
                                textSizeIcon.visible = textSizeSelected;
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
        }

        RowLayout
        {
            Item { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                enabled: root.visualisationExpressionsValid
                onClicked: function(mouse) { root.accept(); }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: function(mouse) { root.reject(); }
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
        if(!root.visualisationExpressionsValid)
            return;

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

    function updateVisualisationExpressions()
    {
        let newVisualsiationExpressions = [];

        channelList.selectedValues.forEach(function(channelName)
        {
            let attribute = document.attribute(attributeList.selectedValue);

            let expression = VisualisationUtils.expressionFor(
                document, attributeList.selectedValue, attribute.flags,
                attribute.valueType, channelName);

            newVisualsiationExpressions.push(expression);
        });

        visualisationExpressions = newVisualsiationExpressions;
    }

    onAccepted:
    {
        updateVisualisationExpressions();
        document.update([], visualisationExpressions);
    }

    onVisibleChanged:
    {
        if(visible)
            attributeList.model = document.availableAttributesModel(ElementType.Node|ElementType.Edge);
        else
            attributeList.model = null;

        channelList.model = null;
    }
}
