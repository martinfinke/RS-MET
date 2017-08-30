rsModulationSetup::rsModulationSetup(AutomatableWidget* widgetToModulate)
  : widget(widgetToModulate), rsDeletionRequester(widgetToModulate)
{
  addWidget( modulationsLabel = new RTextField("Modulations") );
  modulationsLabel->setNoBackgroundAndOutline(true);
  modulationsLabel->setDescription(juce::String("Modulation setup"));

  addWidget( closeButton = new RClickButton(RButton::CLOSE) );
  closeButton->setDescription(juce::String("Closes the modulation setup window"));
  closeButton->setClickingTogglesState(false);
  closeButton->addRButtonListener(this);

  addWidget( addButton = new RButton("Add") );
  addButton->setDescription(juce::String("Adds a new modulation connection"));
  addButton->setClickingTogglesState(false);
  addButton->addRButtonListener(this);

  updateAmountSliderArray();
}

rsModulationSetup::~rsModulationSetup()
{
  delete sourcesPopUp;
}

//void rsModulationSetup::paint(Graphics& g)
//{
//  ColourSchemeComponent::paint(g);
//}

void rsModulationSetup::resized()
{
  int x   = 4;
  int y   = 4;
  int w   = getWidth();
  int h   = getHeight();
  int sh  = sliderHeight;
  int inc = sh+sliderDistance;

  closeButton->setBounds(w-16, 0, 16, 16);
  modulationsLabel->setBounds(x, y, w-8-16, sh); y += inc; 
  for(int i = 0; i < size(amountSliders); i++) {
    amountSliders[i]->setBounds(x, y, w-8, sh); y += inc; }
  y = h - sh - 4;
  addButton->setBounds(x, y, 40, 16);
}

void rsModulationSetup::rButtonClicked(RButton *button)
{
  if(button == closeButton)
    requestDeletion();
  else if(button == addButton)
    showConnectableSourcesPopUp();
}

void rsModulationSetup::rPopUpMenuChanged(RPopUpMenu* menuThatHasChanged)
{
  jassert(menuThatHasChanged == sourcesPopUp);

  // todo: wrap this into a member function of RPopUpMenu::getSelectedIdentifier (it's used 
  // multiple times):
  RTreeViewNode *selectedItem = sourcesPopUp->getSelectedItem();
  if(selectedItem == nullptr)
    return;
  int selectedIdentifier = selectedItem->getNodeIdentifier();


  if(selectedIdentifier > 0)
    addConnection(selectedIdentifier-1);
}

void rsModulationSetup::addConnection(int index)
{
  ModulatableParameter* mp = widget->getModulatableParameter();
  if(mp != nullptr)
  {
    std::vector<ModulationSource*> sources = mp->getDisconnectedSources();
    mp->addModulationSource(sources[index]);

    //// nnaaahh - this doesn't work - we need the ModulationConnection object to retrieve the
    //// amount-parameter
    //MetaControlledParameter* amountParam = mp->getAmountParameter();
    //addSliderFor(amountParam);
  }

  updateAmountSliderArray(); // actually, it's not necessary here to check in the existing
                             // slider array if the slider already exist (it doesn't), so we could
                             // have a function addSliderFor(MetaControlledParameter* p)
}

void rsModulationSetup::updateAmountSliderArray()
{
  // todo: remove all sliders for which there is no connection and create sliders for existing 
  // connections that do not yet have a slider

  ModulatableParameter* mp = widget->getModulatableParameter();
  if(mp != nullptr)
  {
    std::vector<ModulationConnection*> connections = mp->getConnections();
    for(int i = 0; i < size(connections); i++)
    {
      MetaControlledParameter* param = connections[i]->getAmountParameter();
      if(!hasSlider(param))
        addSliderFor(param);
    }
  }
  updateSize();
}

void rsModulationSetup::showConnectableSourcesPopUp()
{
  // create popup, if necessary:
  if(sourcesPopUp == nullptr)
  {
    sourcesPopUp = new RPopUpMenu(this); // maybe attach to the addButton instead of this?
    sourcesPopUp->registerPopUpMenuObserver(this); // uncomment later
    sourcesPopUp->setDismissOnFocusLoss(true);
  }

  // populate it:
  sourcesPopUp->clear();
  ModulatableParameter* mp = widget->getModulatableParameter();
  if(mp != nullptr)
  {
    std::vector<ModulationSource*> sources = mp->getDisconnectedSources();
    for(int i = 0; i < size(sources); i++)
    {
      //juce::String name = "Source " + juce::String(i); // preliminary - todo: retrieve name
      juce::String name = sources[i]->getModulationSourceName();
      sourcesPopUp->addItem(i+1, name);                // +1 bcs 0 is not allowed for the id
    }
  }

  // show it:
  int w = sourcesPopUp->getRequiredWidth(true);
  int h = sourcesPopUp->getRequiredHeight(true);
  sourcesPopUp->show(true, RPopUpComponent::BELOW, w, h); // showModally = true
}

bool rsModulationSetup::hasSlider(MetaControlledParameter* p)
{
  for(int i = 0; i < size(amountSliders); i++)
  {
    Parameter* ps = amountSliders[i]->getAssignedParameter();
    if(ps == p)
      return true;
  }
  return false;
}

void rsModulationSetup::addSliderFor(MetaControlledParameter* p)
{
  AutomatableSlider* s = new AutomatableSlider();
  amountSliders.push_back(s);
  s->assignParameter(p);
  // the slider needs a name that reflects the name of the ModulationSource ...but first the 
  // ModulationSource itself needs a proper name - i think, the slider will then use that too
  addWidget(s);
  updateSize();
}

void rsModulationSetup::updateSize()
{
  int width  = 200;  // maybe we should use the widget's width...but maybe not
  int height = 100;  // preliminary

  height  = (sliderHeight+sliderDistance) * size(amountSliders);
  height += 44;

  setSize(width, height); 
  resized(); // needed during development - might be redundant when finished
}

//=================================================================================================

AutomatableWidget::AutomatableWidget(RWidget *widgetToWrap)
{
  wrappedWidget = widgetToWrap;
}

AutomatableWidget::~AutomatableWidget()
{
  delete rightClickPopUp;
  delete modSetup;
}

bool AutomatableWidget::isPopUpOpen()
{
  return popUpIsOpen;

  // The code below doesn't work because apparently, when the user clicks on a widget while the
  // popup is open, the popup gets closed first and only after that, the mouseDown callback of the
  // widget is received, so isPopUpOpen would always return false in the mouseDown callback. The
  // desired behavior is that one right-click on the widget opens the popup and a second click
  // closes it. This behavior now requires that we maintain a popUpIsOpen flag here and that flag
  // should be set in the mouseDown method of the widget. Without that, a second right-click would
  // make the menu disappear for a fraction of a second and immediately reappear.

  //if(rightClickPopUp == nullptr)
  //  return false;
  //else
  //  return rightClickPopUp->isOpen();
}

void AutomatableWidget::rPopUpMenuChanged(RPopUpMenu* menuThatHasChanged)
{
  if(menuThatHasChanged != rightClickPopUp)
    return;
  RTreeViewNode *selectedItem = rightClickPopUp->getSelectedItem();
  if(selectedItem == nullptr)
    return;
  int selectedIdentifier = selectedItem->getNodeIdentifier();

  if(selectedIdentifier == MODULATION_SETUP)
  {
    showModulationSetup();
    return;
  }

  MetaControlledParameter* mcp = getMetaControlledParameter();
  if(mcp != nullptr)
  {
    switch(selectedIdentifier)
    {
    case META_ATTACH: mcp->attachToMetaParameter(
      (int)wrappedWidget->openModalNumberEntryField(mcp->getMetaParameterIndex()) ); break;
    case META_DETACH: mcp->detachFromMetaParameter();    break;
    }
  }

  AutomatableParameter* ap = getAutomatableParameter();
  if(ap != nullptr)
  {
    if(selectedIdentifier != MIDI_LEARN)
      ap->switchIntoMidiLearnMode(false); // turn off, if currently on and something else is selected
    switch(selectedIdentifier)
    {
    case MIDI_LEARN:  ap->switchIntoMidiLearnMode();             break;
    case MIDI_ASSIGN:
    {
      int result = (int)wrappedWidget->openModalNumberEntryField(ap->getAssignedMidiController());
      result = (int)clip(result, 0, 127);
      ap->assignMidiController(result);
    } break;
    case MIDI_MIN:    ap->setLowerAutomationLimit(ap->getValue());   break;
    case MIDI_MAX:    ap->setUpperAutomationLimit(ap->getValue());   break;
    case MIDI_REVERT: ap->revertToDefaults(false, false, false);     break;
    }
  }
}

void AutomatableWidget::updatePopUpMenu()
{
  if(rightClickPopUp == nullptr)
  {
    // popup used the 1st time - we need to create it:
    rightClickPopUp = new RPopUpMenu(wrappedWidget);
    rightClickPopUp->registerPopUpMenuObserver(this);
    rightClickPopUp->setDismissOnFocusLoss(true);
    wrappedWidget->addChildWidget(rightClickPopUp, false, false);
  }
  rightClickPopUp->clear();
  addPopUpMenuItems();
}

void AutomatableWidget::addPopUpMenuItems()
{
  addPopUpMetaItems();
  addPopUpMidiItems();
  addPopUpModulationItems();
}

void AutomatableWidget::addPopUpMidiItems()
{
  AutomatableParameter* ap = getAutomatableParameter();
  if(ap != nullptr)
  {
    // prepare some strings for the popup menu:
    int cc = ap->getAssignedMidiController();
    String ccString;
    if(cc > -1)
      ccString = "(currently CC" + String(cc) + ")";
    else
      ccString = "(currently none)";

    int defaultCc = ap->getDefaultMidiController();
    String defaultString;
    if(defaultCc > -1)
      defaultString = "CC" + String(defaultCc);
    else
      defaultString = "none";
    String minString = wrappedWidget->stringConversionFunction(ap->getLowerAutomationLimit());
    String maxString = wrappedWidget->stringConversionFunction(ap->getUpperAutomationLimit());

    rightClickPopUp->addItem(MIDI_LEARN,  "MIDI learn " + ccString);
    rightClickPopUp->addItem(MIDI_ASSIGN, "MIDI assign");
    rightClickPopUp->addItem(MIDI_MIN,    "use value as lower limit (currently " + minString + String(")"));
    rightClickPopUp->addItem(MIDI_MAX,    "use value as upper limit (currently " + maxString + String(")"));
    rightClickPopUp->addItem(MIDI_REVERT, "revert MIDI mapping to defaults");
  }
}

void AutomatableWidget::addPopUpMetaItems()
{
  MetaControlledParameter* mcp = getMetaControlledParameter();
  if(mcp != nullptr)
  {
    int mi = mcp->getMetaParameterIndex();
    String miString;
    if(mi > -1)
      miString = "(currently " + String(mi) + ": " + mcp->getMetaParameterName() + ")";
    else
      miString = "(currently none)";

    rightClickPopUp->addItem(META_ATTACH, "Meta attach " + miString);
    rightClickPopUp->addItem(META_DETACH, "Meta detach");
  }
}

void AutomatableWidget::addPopUpModulationItems()
{
  ModulatableParameter* mp = getModulatableParameter();
  if(mp != nullptr)
  {
    //// \todo: add sliders for the already connected sources
    //rightClickPopUp->addItem(MODULATOR_CONNECT, "Connect modulator...");
    //  // should open a 2nd level popup with the modulators available for connection

    rightClickPopUp->addItem(MODULATION_SETUP, "Modulation setup");
  }
}

void AutomatableWidget::openRightClickPopupMenu()
{
  updatePopUpMenu();
  int w = jmax(wrappedWidget->getWidth(), rightClickPopUp->getRequiredWidth(true));
  int h = jmin(200,                       rightClickPopUp->getRequiredHeight(true));
  //rightClickPopUp->show(false, RPopUpComponent::BELOW, w, h); // showModally = false
  rightClickPopUp->show(true, RPopUpComponent::BELOW, w, h); // showModally = true
  // If we don't show it modally (1st parameter = true), it will be immediately dismissed
  // after opening (so it appears as if it doesn't open at all). We could avoid it by calling
  // setDismissOnFocusLoss(false) in our constructor, but then it will stay open all the time
  // until we choose some option.

  popUpIsOpen = true;
}

void AutomatableWidget::closePopUp()
{
  if(rightClickPopUp != nullptr)
    rightClickPopUp->dismiss();
  popUpIsOpen = false;
}

void AutomatableWidget::showModulationSetup()
{
  //int ww = wrappedWidget->getWidth();        // widget width
  int wh = wrappedWidget->getHeight();       // widget height
  int x  = wrappedWidget->getScreenX();
  int y  = wrappedWidget->getScreenY() + wh; // preliminary

  if(modSetup == nullptr)
    modSetup = new rsModulationSetup(this);
  modSetup->setTopLeftPosition(x, y);
  modSetup->addToDesktop(ComponentPeer::windowHasDropShadow | ComponentPeer::windowIsTemporary);
  modSetup->setVisible(true);
  modSetup->toFront(true);

  // we need to show a popup window (similar to Straightliner's Osc "More" popup) with sliders for
  // the modulation amounts for connected sources, a facility to add more connections, text-fields
  // for the min/max values of the sliders (should be on left and right sides to the slider) and 
  // maybe a button to set relative modulation mode. The window should appear below or next to the
  // slider...maybe it can somehow frame it to make it more apparent to which slider it applies
}

void AutomatableWidget::deleteObject(rsDeletionRequester* objectToDelete)
{
  if(objectToDelete == modSetup)
    modSetup->setVisible(false);
  else
    jassertfalse;
}

AutomatableParameter* AutomatableWidget::getAutomatableParameter()
{
  return dynamic_cast<AutomatableParameter*> (wrappedWidget->assignedParameter);
}

MetaControlledParameter* AutomatableWidget::getMetaControlledParameter()
{
  return dynamic_cast<MetaControlledParameter*> (wrappedWidget->assignedParameter);
}

ModulatableParameter* AutomatableWidget::getModulatableParameter()
{
  return dynamic_cast<ModulatableParameter*> (wrappedWidget->assignedParameter);
}

//=================================================================================================

AutomatableSlider::AutomatableSlider()
  : AutomatableWidget(this)
{

}

void AutomatableSlider::mouseDown(const MouseEvent& e)
{
  if(e.mods.isRightButtonDown())
  {
    if(!isPopUpOpen())
      openRightClickPopupMenu();
    else
      closePopUp();
  }
  else
  {
    RSlider::mouseDown(e);

    // doesn't work:
    //if(!isPopUpOpen())
    //  RSlider::mouseDown(e);
    //else
    //  closePopUp();
  }
}

void AutomatableSlider::rPopUpMenuChanged(RPopUpMenu* menuThatHasChanged)
{
  if( menuThatHasChanged != rightClickPopUp )
    return;
  RTreeViewNode *selectedItem = rightClickPopUp->getSelectedItem();
  if( selectedItem == NULL )
    return;
  int selectedIdentifier = selectedItem->getNodeIdentifier();

  switch( selectedIdentifier )
  {
  case ENTER_VALUE:   setValue(openModalNumberEntryField(getValue()),        true, false); break;
  case DEFAULT_VALUE: setValue(selectedItem->getNodeText().getDoubleValue(), true, false); break;
  default: AutomatableWidget::rPopUpMenuChanged(menuThatHasChanged);
  }
}

void AutomatableSlider::addPopUpMenuItems()
{
  addPopUpEnterValueItem();
  addPopUpDefaultValueItems();
  AutomatableWidget::addPopUpMenuItems();
}

void AutomatableSlider::addPopUpEnterValueItem()
{
  rightClickPopUp->addItem(ENTER_VALUE, "Enter Value");
}

void AutomatableSlider::addPopUpDefaultValueItems()
{
  if( defaultValues.size() > 0 )
  {
    RTreeViewNode *defaultValuesNode = new RTreeViewNode("Default Values");
    for(int i = 0; i < size(defaultValues); i++)
      defaultValuesNode->addChildNode(new RTreeViewNode(String(defaultValues[i]), DEFAULT_VALUE));
    rightClickPopUp->addTreeNodeItem(defaultValuesNode);
  }
}

//=================================================================================================

AutomatableComboBox::AutomatableComboBox()
  : AutomatableWidget(this)
{

}

void AutomatableComboBox::mouseDown(const MouseEvent& e)
{
  if( e.mods.isRightButtonDown() )
  {
    if(!isPopUpOpen())
      openRightClickPopupMenu();
    else
      AutomatableWidget::closePopUp();
  }
  else
    RComboBox::mouseDown(e);
}

void AutomatableComboBox::parameterChanged(Parameter* p)
{
  RWidget::parameterChanged(p);
  // not sure, why that's needed - isn't it supposed to be called anyway, i.e. if we don't override
  // parameterChanged, the RWidget baseclass method would be called? but somehow, it doesn't seem
  // to work
}

//=================================================================================================

AutomatableButton::AutomatableButton(const juce::String& buttonText)
  : RButton(buttonText), AutomatableWidget(this)
{

}

void AutomatableButton::mouseDown(const MouseEvent& e)
{
  if( e.mods.isRightButtonDown() )
  {
    if(!isPopUpOpen())
      openRightClickPopupMenu();
    else
      closePopUp();
  }
  else
    RButton::mouseDown(e);
}

void AutomatableButton::parameterChanged(Parameter* p)
{
  RWidget::parameterChanged(p);
}
