/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.4.3

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2017 - ROLI Ltd.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../../CommonFunctions.h"
#include "../../PluginState.h"


class MidiRemapTableModel : public TableListBoxModel
{
    int numRows = 3;

    Array<int>* inputMap;
    Array<int>* outputMap;

	Point<int> selectedCell;

public:

    MidiRemapTableModel(Array<int>* inputMapIn, Array<int>* outputMapIn);
	~MidiRemapTableModel();

    int getNumRows() override;

	void cellClicked(int rowNumber, int columnId, const MouseEvent& e) override;

	void backgroundClicked(const MouseEvent& e) override;

	Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

    void paintRowBackground (Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;

    void paintCell (Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;



    enum RowType
    {
        keyNumber = 1,
        inputNote,
        outputNote
    };
};
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MidiRemapperDialog  : public Component,
                            public TextEditor::Listener,
                            public ComboBox::Listener,
                            public Slider::Listener,
                            public Button::Listener
{
public:
    //==============================================================================
    MidiRemapperDialog (SuperVirtualKeyboardPluginState* pluginStateIn);
    ~MidiRemapperDialog();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	void visibilityChanged() override;
	void textEditorTextChanged(TextEditor& t) override;
	
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

    std::unique_ptr<TableHeaderComponent> remapTableHeader;
    std::unique_ptr<MidiRemapTableModel> remapTableModel;
    std::unique_ptr<TableListBox> remapTable;

	int boxW = 10;
	int boxH = 8;

	SuperVirtualKeyboardPluginState* pluginState;
	MidiRemapper* inputRemapper;
	MidiRemapper* outputRemapper;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<ComboBox> comboBox;
    std::unique_ptr<Label> midiInuptLable;
    std::unique_ptr<Slider> slider;
    std::unique_ptr<Label> noteRangeLabel;
    std::unique_ptr<ToggleButton> manualMapToggle;
    std::unique_ptr<ToggleButton> modeMapToggle;
    std::unique_ptr<ComboBox> modeLibraryBoxOG;
    std::unique_ptr<Label> modeBoxOriginal;
    std::unique_ptr<ComboBox> modeLibraryBoxOG2;
    std::unique_ptr<Label> modeBoxRemap;
    std::unique_ptr<Slider> modeOriginalRootSld;
    std::unique_ptr<Slider> modeRemapRootSld;
    std::unique_ptr<Label> rootNoteLabel;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiRemapperDialog)
};

//[EndFile] You can add extra defines here...

//[/EndFile]

