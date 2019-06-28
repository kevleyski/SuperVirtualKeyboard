/*
  ==============================================================================

    PluginState.cpp
    Created: 7 Apr 2019 12:27:01pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "PluginState.h"

SvkPluginState::SvkPluginState()
{
    appCmdMgr.reset(new ApplicationCommandManager());
    undoManager.reset(new UndoManager());

	pluginStateNode = ValueTree(IDs::pluginStateNode);

	pluginSettings.reset(new SvkPluginSettings());
	pluginSettingsNode = pluginSettings->pluginSettingsNode;
	pluginStateNode.addChild(pluginSettingsNode, -1, nullptr);
    
    midiProcessor.reset(new SvkMidiProcessor());
    midiSettingsNode = midiProcessor->midiSettingsNode;
    pluginStateNode.addChild(midiSettingsNode, -1, nullptr);

	presetManager.reset(new SvkPresetManager(pluginSettingsNode));
	presetLibraryNode = presetManager->presetLibraryNode;
	presetManager->addChangeListener(this);

	modeLoaded.reset(new Mode());
    modePresetNode = modeLoaded->modeNode;
    pluginStateNode.addChild(modePresetNode, -1, nullptr);

	virtualKeyboard.reset(new VirtualKeyboard::Keyboard(midiProcessor.get()));
	virtualKeyboard->addListener(midiProcessor.get()); // generates MIDI from UI
	pianoNode = virtualKeyboard->getNode();

	textFilterIntOrSpace.reset(new TextFilterIntOrSpace());
	textFilterInt.reset(new TextFilterInt());
    
	presetWorkingNode = ValueTree(IDs::presetNode);
	pluginStateNode.addChild(presetWorkingNode, -1, nullptr);

    updatePluginToPresetLoaded();
}

SvkMidiProcessor* SvkPluginState::getMidiProcessor()
{
    return midiProcessor.get();
}

UndoManager* SvkPluginState::getUndoManager()
{
	return undoManager.get();
}

SvkPreset* SvkPluginState::getPresetLoaded()
{
	return &presetWorking;
}

Mode* SvkPluginState::getModeLoaded()
{
	return modeLoaded.get();
}

VirtualKeyboard::Keyboard* SvkPluginState::getKeyboard()
{
	return virtualKeyboard.get();
}

NoteMap* SvkPluginState::getMidiInputMap()
{
    return midiProcessor->getMidiInputFilter()->getNoteMap();
}
NoteMap* SvkPluginState::getMidiOutputMap()
{
    return midiProcessor->getMidiOutputFilter()->getNoteMap();
}

bool SvkPluginState::isPresetEdited()
{
    return presetEdited;
}

void SvkPluginState::loadMode(int presetIndexIn)
{
	presetManager->loadPreset(presetIndexIn);
}

void SvkPluginState::loadMode(ValueTree modeNodeIn)
{
	presetManager->loadPreset(modeNodeIn);
}

void SvkPluginState::recallState(ValueTree nodeIn)
{
	if (nodeIn.hasType(IDs::presetNode))
	{
		presetManager->loadPreset(nodeIn, false);
		return;
	}

	ValueTree childNode = nodeIn.getChildWithName(IDs::presetNode);

	if (childNode.hasType(IDs::presetNode))
	{
		presetManager->loadPreset(childNode, false);
	}
	else
	{
		updatePluginToPresetLoaded();
		return;
	}

	childNode = nodeIn.getChildWithName(IDs::pluginSettingsNode);

	if (childNode.isValid())
		pluginSettings->restoreNode(childNode);

	childNode = nodeIn.getChildWithName(IDs::pluginEditorNode);

	if (childNode.isValid())
		pluginEditorNode = childNode.createCopy();
}


void SvkPluginState::setMidiRootNote(int rootNoteIn)
{
	rootNoteIn = totalModulus(rootNoteIn, 128);
	midiProcessor->setRootNote(rootNoteIn);
	modeLoaded->setRootNote(rootNoteIn);
    virtualKeyboard->updateKeys();
    
    presetEdited = true;
}

void SvkPluginState::updatePluginToPresetLoaded()
{
    DBG("Updating plugin from this loaded preset:");
    presetWorking = SvkPreset(*presetManager->getPresetLoaded());
	presetWorkingNode = presetWorking.parentNode;
	DBG("COMITTED:\n" + presetWorkingNode.toXmlString());
    presetEdited = false;
    
    DBG(presetWorking.parentNode.toXmlString());
    
    midiProcessor->setMidiMaps(presetWorking.theMidiSettingsNode);
    
    modeLoaded->restoreNode(presetWorking.theModeNode, false);
    midiProcessor->setScaleSize(modeLoaded->getScaleSize());
	
	//virtualKeyboard->restoreDataNode(presetWorking.theKeyboardNode);
    
    sendChangeMessage();
}

void SvkPluginState::updatePluginFromParentNode()
{
    DBG("Updating plugin from memory block with this preset:");
    
    presetManager->loadPreset(modePresetNode, false);
    presetWorking = SvkPreset(*presetManager->getPresetLoaded());
	presetWorkingNode = presetWorking.parentNode;
    presetEdited = false;
    
    DBG(presetWorking.parentNode.toXmlString());
    
    midiProcessor->restoreFromNode(presetWorking.theMidiSettingsNode);

    modeLoaded->restoreNode(modePresetNode, false);
    midiProcessor->setScaleSize(modeLoaded->getScaleSize());
    
    sendChangeMessage();
}

void SvkPluginState::commitPresetChanges()
{
    presetWorking.updateModeNode(modeLoaded->modeNode);
    presetWorking.updateKeyboardNode(pianoNode);
    presetWorking.updateMidiNode(midiProcessor->midiMapNode);
    presetManager->commitPresetNode(presetWorking.parentNode);
    
    presetWorking = SvkPreset(*presetManager->getPresetLoaded());
	presetWorkingNode = presetWorking.parentNode;
    presetEdited = false;
}

bool SvkPluginState::savePreset()
{
    commitPresetChanges();
    return presetManager->savePreset();
}


bool SvkPluginState::loadPreset()
{
    return presetManager->loadPreset();
}

void SvkPluginState::changeListenerCallback(ChangeBroadcaster* source)
{
	// Preset loaded
	if (source == presetManager.get())
	{
        updatePluginToPresetLoaded();
	}
}
