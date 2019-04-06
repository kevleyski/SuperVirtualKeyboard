/*
  ==============================================================================

	ViewPianoComponent.h
	Created: 14 Mar 2019 4:50:31pm
	Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "ViewPianoComponent.h"


//==============================================================================

ViewPianoComponent::PianoKeyComponent::PianoKeyComponent(String nameIn, int keyNumIn)
	: Button(nameIn)
{
	keyNumber = keyNumIn;
	mappedMIDInote = keyNumber;

	setOpaque(true);
}

void ViewPianoComponent::PianoKeyComponent::paintButton(Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    // idk why getBounds() doesn't work properly. maybe it's set in the parent bounds
    Rectangle<int> fillBounds = Rectangle<int>(0, 0, getWidth() - 1, getHeight());
	color = findColour(activeColor);
    
    if (activeColor == 2)
    {
        color = findColour(0).interpolatedWith(findColour(2), 0.5);
        fillBounds.reduce(1, 1);
    }

	if (externalMidiState > 0)
		color = color.interpolatedWith(Colours::lightblue, 0.75);

    
    g.setColour(color);
    g.fillRect(fillBounds);
}

//===============================================================================================

ViewPianoComponent::PianoKeyGrid::PianoKeyGrid()
{
	order = 1;
	modeSize = 7;
}

int ViewPianoComponent::PianoKeyGrid::set_order(int orderIn, float* orderWidthRatio, float* orderHeightRatio)
{
	order = orderIn;
	return order;
}

int ViewPianoComponent::PianoKeyGrid::set_mode_keys(int modeSizeIn)
{
	modeSize = modeSizeIn;
	set_columns(modeSizeIn);
	return modeSize;
}

void ViewPianoComponent::PianoKeyGrid::set_mode_keys(int modeSizeIn, int highestOrder)
{
	order = highestOrder;
	set_mode_keys(modeSizeIn);
}

void ViewPianoComponent::PianoKeyGrid::set_order_layout(std::vector<int> layoutIn)
{
	orderLayout = layoutIn;
}

void ViewPianoComponent::PianoKeyGrid::place_key(PianoKeyComponent* key)
{
	if (needs_to_update())
		update_grid();

	Point<int> pt;

	float colToPlace = ceil(key->modeDegree);
	int offset = (key->order > 0) * (int)(key->getWidth() / 2.0) * (key->orderWidthRatio + 1);

	if (key->order > 0)
	{
		colToPlace = ceil(colToPlace);
	}

	pt = Point<int>((int)(colToPlace * (column_size() + columnGap) - offset), 0 + rowGap);

	key->setTopLeftPosition(pt);
}

void ViewPianoComponent::PianoKeyGrid::place_key_layout(OwnedArray<PianoKeyComponent>* keys)
{
	if (needs_to_update())
		update_grid();

	for (int i = 0; i < keys->size(); i++)
	{
		place_key(keys->getUnchecked(i));
	}
}

//===============================================================================================

ViewPianoComponent::PianoMenuBar::PianoMenuBar(ApplicationCommandManager* cmdMgrIn)
{
	menu.reset(new MenuBarComponent(this));
	addAndMakeVisible(menu.get());
	setApplicationCommandManagerToWatch(cmdMgrIn);

	setSize(1000, 80);
}

ViewPianoComponent::PianoMenuBar::~PianoMenuBar()
{

}


StringArray ViewPianoComponent::PianoMenuBar::getMenuBarNames()
{
	return {"File", "Mode", "Orientation"};
}

void ViewPianoComponent::PianoMenuBar::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

}

PopupMenu ViewPianoComponent::PianoMenuBar::getMenuForIndex(int topLevelMenuIndex, const String &menuName)
{
	return PopupMenu();
}

void ViewPianoComponent::PianoMenuBar::resized()
{
	setSize(getParentWidth(), 80);
}

//===============================================================================================

ViewPianoComponent::ViewPianoComponent(ApplicationCommandManager& cmdMgrIn)
{
	// Default values
	tuningSize = 12;
	modeSize = 7;
	notesToShow = 128;
	rows = 1;

	removeMouseListener(this);

	appCmdMgr = &cmdMgrIn;
	menu.reset(new PianoMenuBar(appCmdMgr));
	menu.get()->setName("Piano Menu");
	//addAndMakeVisible(menu.get());
	

	// Create children (piano keys)        
	for (int i = 0; i < notesToShow; i++)
	{
		String keyName = "Key " + String(i);
		PianoKeyComponent* key = new PianoKeyComponent(keyName, i);
		addChildComponent(keys.add(key));
	}

	addMouseListener(this, true);
	setRepaintsOnMouseActivity(true);
	toBack();
	
	setSize(1000, 250);
	setOpaque(true);

	// Apply default layout
	apply_steps_layout(defaultMOS);
}

//===============================================================================================

MidiKeyboardState* ViewPianoComponent::get_keyboard_state()
{
	return &keyboardState;
}

Point<int> ViewPianoComponent::get_position_of_key(int midiNoteIn)
{
	PianoKeyComponent* key = keys.getUnchecked(midiNoteIn % keys.size());
	return key->getPosition();
}

ViewPianoComponent::PianoKeyComponent* ViewPianoComponent::get_key_from_position(Point<int> posIn)
{
	PianoKeyComponent* keyOut = nullptr;
	
	if (reallyContains(posIn, true))
	{
		Component* c = getComponentAt(posIn);
		
		if (c->getName().startsWith("Key"))
			keyOut = dynamic_cast<PianoKeyComponent*>(c);	
	}

	return keyOut;
}

ViewPianoComponent::PianoKeyComponent* ViewPianoComponent::get_key_from_position(const MouseEvent& e)
{
	PianoKeyComponent* keyOut = nullptr;
	Point<int> mousePosition = e.getScreenPosition() - getScreenBounds().getPosition();

	if (e.eventComponent->getName().startsWith("Key") &&
		reallyContains(mousePosition, true))
	{
		keyOut = dynamic_cast<PianoKeyComponent*>(getComponentAt(mousePosition));
	}

	return keyOut;
}

float ViewPianoComponent::get_velocity(PianoKeyComponent* keyIn, const MouseEvent& e)
{
	Point<int> mousePosition = e.getScreenPosition() - getScreenBounds().getPosition();
	Point<int> mouseInKey = mousePosition.withX(mousePosition.x - e.eventComponent->getX());

	float velocity = (float)mouseInKey.y / (float)keyIn->getHeight();

	return velocity;
}

//===============================================================================================

void ViewPianoComponent::apply_layout(ModeLayout layoutIn)
{
	modeDisplayed.reset(new ModeLayout(layoutIn));
	scaleLayout = layoutIn.get_order();
	tuningSize = scaleLayout.size();

	modeSize = modeDisplayed.get()->scaleSize;
	modeOrder = modeDisplayed.get()->get_highest_order();
	modalKeysSize = modeDisplayed.get()->get_num_modal_notes();

	keysOrder.resize(modeOrder + 1);
	float modeDegree = -1;

	// Setup keys for layout
	PianoKeyComponent* key;
	for (int i = 0; i < notesToShow; i++)
	{
		key = keys.getUnchecked(i);

		key->order = scaleLayout.at(i % tuningSize);
		keysOrder[key->order].push_back(key);

		auto ratios = get_key_proportions(key);
		key->orderWidthRatio = ratios.x;
		key->orderHeightRatio = ratios.y;

		key->setColour(0, get_key_color(key));
		key->setColour(1, get_key_color(key).contrasting(0.2));
		key->setColour(2, Colours::yellow);

		key->modeDegree = keysOrder[key->order].size() + (key->order / (modeOrder + 1.0));

		if (key->order == 0)
		{
			key->modeDegree = ++modeDegree;
			key->toBack();
		}
		else
		{
			key->modeDegree = modeDegree + (key->order / (modeOrder + 1.0f));
			key->toFront(false);
		}

		key->setVisible(true);
	}

	// Update grid properties
	grid.set_mode_keys(keysOrder[0].size(), modeOrder);

	// Calculate properties
	displayIsReady = true;
	resized();
}

void ViewPianoComponent::apply_steps_layout(juce::String strIn)
{
	ModeLayout layout = ModeLayout(strIn);
	apply_layout(layout);
}

void ViewPianoComponent::apply_steps_layout(std::vector<int> stepsIn)
{
	ModeLayout layout = ModeLayout(stepsIn);
	apply_layout(layout);
}

//===============================================================================================

// The function that determines default key proportions and chooses custom ones if set
Point<float> ViewPianoComponent::get_key_proportions(PianoKeyComponent* keyIn)
{
	Point<float>  out;
	float defaultRatio = 1.0f - (keyIn->order > 0) * 0.8 * (2, 1.0f / (modeOrder - keyIn->order + 2));

	if (keyIn->widthMod > 0)
		out.x = keyIn->widthMod;
	else
		out.x = defaultRatio;

	if (keyIn->heightMod > 0)
		out.y = keyIn->heightMod;
	else
		out.y = defaultRatio;

	return out;
}

Colour ViewPianoComponent::get_key_color(PianoKeyComponent* keyIn)
{
	// implement custom colors
	return keyOrderColors.at(keyIn->order % keyOrderColors.size());
}

void ViewPianoComponent::all_notes_off()
{
	PianoKeyComponent* key;

	for (int i = 0; i < keys.size(); i++)
	{
		key = keys.getUnchecked(i);
		triggerKeyNoteOff(key);
	}
	repaint();
}

void ViewPianoComponent::isolate_last_note()
{
	if (lastKeyClicked >= 0 && lastKeyClicked < 128)
	{
		PianoKeyComponent* last = keys.getUnchecked(lastKeyClicked);
		PianoKeyComponent* key;

		for (int i = 0; i < keys.size(); i++)
		{
			key = keys.getUnchecked(i);

			if (key == last)
				continue;

			triggerKeyNoteOff(key);
		}
		repaint();
	}
}

bool ViewPianoComponent::check_keys_modal()
{
	PianoKeyComponent* key;
	bool modal = true;

	for (int i = 0; i < keysOn.size(); i++)
	{
		key = keysOn[i];
		modal *= (floor(key->modeDegree) == key->modeDegree);
	}

	return modal;
}

void ViewPianoComponent::transpose_keys_modal(int modalStepsIn)
{
	PianoKeyComponent* key;
	int newDeg = -1;
	float velocity;

	for (int i = 0; i < keysOn.size(); i++)
	{
		key = keysOn[i];
		velocity = key->velocity;
		triggerKeyNoteOff(key);
		
		for (int i = 0; i < keysOrder[0].size(); i++)
		{
			if (key->modeDegree == keysOrder[0].at(i)->modeDegree)
			{
				newDeg = i + modalStepsIn;
			}
		}

		if (newDeg < 0 || newDeg > keysOrder[0].size())
			continue;

		key = keysOrder[0][newDeg];
		triggerKeyNoteOn(key, velocity);
	}
}

void ViewPianoComponent::transpose_keys(int stepsIn)
{
	PianoKeyComponent* key;
	int newKey;
	float velocity;

	for (int i = 0; i < keysOn.size(); i++)
	{
		key = keysOn[i];
		velocity = key->velocity;
		triggerKeyNoteOff(key);

		newKey = key->keyNumber + stepsIn;
		if (newKey < 0 || newKey > 127)
			continue;

		key = keys.getUnchecked(newKey);
		triggerKeyNoteOn(key, velocity);
	}
}



//===============================================================================================

ApplicationCommandTarget* ViewPianoComponent::getNextCommandTarget()
{
	return findFirstTargetParentComponent();
}

void ViewPianoComponent::getAllCommands(Array< CommandID > &c)
{
	Array<CommandID> commands{
		CommandIDs::setPianoHorizontal,
		CommandIDs::setPianoVerticalL,
		CommandIDs::setPianoVerticalR,
		CommandIDs::sendScaleToPiano };

	c.addArray(commands);
}

void ViewPianoComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result)
{
	switch (commandID)
	{
	case CommandIDs::setPianoHorizontal:
		result.setInfo("Horizontal Layout", "Draws the piano as you'd sit down and play one", "Piano", 0);
		result.setTicked(pianoOrientationSelected == PianoOrientation::horizontal);
		result.addDefaultKeypress('w', ModifierKeys::shiftModifier);
		break;
	case CommandIDs::setPianoVerticalL:
		result.setInfo("Vertical Left Layout", "Draws the piano with the left facing orientation", "Piano", 0);
		result.setTicked(pianoOrientationSelected == PianoOrientation::verticalLeft);
		result.addDefaultKeypress('a', ModifierKeys::shiftModifier);
		break;
	case CommandIDs::setPianoVerticalR:
		result.setInfo("Vertical Right Layout", "Draws the piano with the right facing orientation", "Piano", 0);
		result.setTicked(pianoOrientationSelected == PianoOrientation::verticalRight);
		result.addDefaultKeypress('d', ModifierKeys::shiftModifier);
		break;
	default:
		break;
	}
}

bool ViewPianoComponent::perform(const InvocationInfo &info)
{
	switch (info.commandID)
	{
	case CommandIDs::setPianoHorizontal:
		// TBI
		break;
	case CommandIDs::setPianoVerticalL:
		// TBI
		break;
	case CommandIDs::setPianoVerticalR:
		// TBI
		break;
	default:
		return false;
	}
	return true;
}

//===============================================================================================


void ViewPianoComponent::mouseExit(const MouseEvent& e)
{
	PianoKeyComponent* key = keys.getUnchecked(lastKeyOver);

	if (!shiftHeld || key->activeColor < 2)
		key->activeColor = 0;
}

void ViewPianoComponent::mouseDown(const MouseEvent& e)
{
	PianoKeyComponent* key = get_key_from_position(e);
	if (key)
	{		
        if (shiftHeld && !altHeld && key->activeColor == 2)
        {
            // note off
            triggerKeyNoteOff(key);
            lastKeyClicked = 0;
        }
        else
        {
			if (altHeld)
			{
				PianoKeyComponent* oldKey = keys.getUnchecked(lastKeyClicked);
				triggerKeyNoteOff(oldKey);
			}

            triggerKeyNoteOn(key, get_velocity(key, e));
            lastKeyClicked = key->keyNumber;
        }
	}
}

void ViewPianoComponent::mouseDrag(const MouseEvent& e)
{
	PianoKeyComponent* key = get_key_from_position(e);

	if (key)
	{
		if (key->keyNumber != lastKeyClicked)
		{
			PianoKeyComponent* oldKey = keys.getUnchecked(lastKeyClicked);
			if (!shiftHeld)
			{
				triggerKeyNoteOff(oldKey);
			}

			triggerKeyNoteOn(key, get_velocity(key, e));
			lastKeyClicked = key->keyNumber;
			repaint();
		}
	}
}

void ViewPianoComponent::mouseUp(const MouseEvent& e)
{
	PianoKeyComponent* key = get_key_from_position(e);

	if (key)
	{
		if (!shiftHeld)
		{
			isolate_last_note();
			key->activeColor = 1;
			repaint();
		}
	}
}

void ViewPianoComponent::mouseMove(const MouseEvent& e)
{
	PianoKeyComponent* key = get_key_from_position(e);

	if (key)
	{
		if (key->activeColor < 2)
		{
			key->activeColor = 1;
			repaint();
		}
		lastKeyOver = key->keyNumber;
	}
}

//===============================================================================================

bool ViewPianoComponent::keyStateChanged(bool isKeyDown)
{
	if (!KeyPress::isKeyCurrentlyDown(KeyPress::upKey) && upHeld)
	{
		upHeld = false;
	}

	if (!KeyPress::isKeyCurrentlyDown(KeyPress::downKey) && downHeld)
	{
		downHeld = false;
	}

	return isKeyDown;
}
bool ViewPianoComponent::keyPressed(const KeyPress& key)
{
	if (KeyPress::isKeyCurrentlyDown(KeyPress::upKey) && !upHeld)
	{
		upHeld = true;

		PianoKeyComponent* pk;
		int inv;

		for (int i = 0; i < keys.size(); i++)
		{
			inv = keys.size() - i - 1;

			if (keyboardState.isNoteOn(midiChannelSelected, inv))
			{
				pk = keys.getUnchecked(inv);
				triggerKeyNoteOff(pk);

				if ((inv + 1) >= keys.size())
					continue;

				pk = keys.getUnchecked(inv + 1);
				triggerKeyNoteOn(pk, 0.75f);
			}
		}

		if (lastKeyClicked < keys.size() - 1)
			lastKeyClicked++;

		repaint();
	}

	if (KeyPress::isKeyCurrentlyDown(KeyPress::downKey) && !downHeld)
	{
		downHeld = true;

		PianoKeyComponent* pk;

		for (int i = 0; i < keys.size(); i++)
		{
			if (keyboardState.isNoteOn(midiChannelSelected, i))
			{
				pk = keys.getUnchecked(i);
				triggerKeyNoteOff(pk);

				if (i == 0)
					continue;

				pk = keys.getUnchecked(i-1);
				triggerKeyNoteOn(pk, 0.75f);
			}
		}

		if (lastKeyClicked > 0)
			lastKeyClicked--;

		repaint();
	}

	return false;
}

void ViewPianoComponent::modifierKeysChanged(const ModifierKeys& modifiers)
{
	if (!shiftHeld && modifiers.isShiftDown())
	{
		shiftHeld = true;
	}
	else if (shiftHeld && !modifiers.isShiftDown())
	{
		shiftHeld = false;

		isolate_last_note();

		PianoKeyComponent* key = keys.getUnchecked(lastKeyClicked);

		if (!(key->isMouseOver() && key->isMouseButtonDownAnywhere()))
			triggerKeyNoteOff(keys.getUnchecked(lastKeyClicked));
	
		repaint();
	}
    
    if (!altHeld && modifiers.isAltDown())
    {
        altHeld = true;
		isolate_last_note();
		repaint();
    }
    else if (altHeld && !modifiers.isAltDown())
    {
        altHeld = false;
    }
    
    if (!ctrlHeld && modifiers.isCtrlDown())
    {
        ctrlHeld = true;
    }
    else if (ctrlHeld && !modifiers.isCtrlDown())
    {
        ctrlHeld = false;
    }
}

//===============================================================================================


void ViewPianoComponent::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));   // clear the background
	g.setColour(Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

	if (!displayIsReady)
	{
		g.setColour(Colours::white);
		g.setFont(14.0f);
		g.drawText("ViewPianoComponent", getLocalBounds(), Justification::centred, true);   // draw some placeholder text
	}
}

void ViewPianoComponent::resized()
{
	// Calculate key sizes
	keyHeight = getHeight() - 1;
	keyWidth = keyHeight * defaultKeyWHRatio;

	// Adjust Parent bounds and grid
    pianoWidth = modalKeysSize * keyWidth;
	setSize(pianoWidth, getHeight());
	grid.setBounds(Rectangle<int>(0, 0, pianoWidth, getHeight()));

	// Resize keys
	PianoKeyComponent* key;
	int w, h;
	for (int i = 0; i < keys.size(); i++)
	{
		key = keys.getUnchecked(i);
		w = keyWidth * key->orderWidthRatio;
		h = keyHeight * key->orderHeightRatio;
		key->setSize(w, h);
		grid.place_key(key);
	}
}

void ViewPianoComponent::visibilityChanged()
{
	if (isShowing())
		setWantsKeyboardFocus(true);
	else
		setWantsKeyboardFocus(false);
}

//===============================================================================================

void ViewPianoComponent::triggerKeyNoteOn(PianoKeyComponent* key, float velocityIn)
{
	if (velocityIn > 0)
	{
		keyboardState.noteOn(midiChannelSelected, key->mappedMIDInote, velocityIn);
		key->activeColor = 2;
		key->velocity = velocityIn;
		keysOn.add(key);
	}
}

void ViewPianoComponent::triggerKeyNoteOff(PianoKeyComponent* key)
{
    keyboardState.noteOff(midiChannelSelected, key->mappedMIDInote, 0);
	
	key->activeColor = 0;
	key->velocity = 0;

	if (key->isMouseOver())
		key->activeColor = 1;
	
	keysOn.removeAllInstancesOf(key);
}

void ViewPianoComponent::handleNoteOn(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity)
{
	PianoKeyComponent* key = keys.getUnchecked(midiNote);
	key->externalMidiState = 1;
}

void ViewPianoComponent::handleNoteOff(MidiKeyboardState* source, int midiChannel, int midiNote, float velocity)
{
	PianoKeyComponent* key = keys.getUnchecked(midiNote);
	key->externalMidiState = 0;
}
