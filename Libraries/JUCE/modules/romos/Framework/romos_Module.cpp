#include "romos_Module.h"
#include "romos_ModuleContainer.h"
#include "romos_AudioConnection.h"
using namespace romos;


//=========================================================================================================================================
// class AudioInputPinData:


AudioInputPinData::AudioInputPinData()
{
  defaultValue = 0.0;
  reset();
}

AudioInputPinData::AudioInputPinData(const AudioInputPinData &other)
{
  copyDataFrom(other);
}

AudioInputPinData& AudioInputPinData::operator=(const AudioInputPinData &other)
{
  copyDataFrom(other);
  return *this;
}
void AudioInputPinData::setDefaultValue(double newDefaultValue)
{
  defaultValue = newDefaultValue;
}
void AudioInputPinData::reset()
{
  sourceModule      = NULL;
  outputPointer     = &defaultValue;
  outputIndex       = 0;
  outputFrameSize   = 0;
  outputVoiceStride = 0;
}

void AudioInputPinData::copyDataFrom(const AudioInputPinData &other)
{
  defaultValue      = other.defaultValue;
  sourceModule      = other.sourceModule;
  outputIndex       = other.outputIndex;
  outputFrameSize   = other.outputFrameSize;
  outputVoiceStride = other.outputVoiceStride;

  // in case of disconnected pins we don't want to point to the other's defaultValue but to our own:
  if( other.outputPointer == &other.defaultValue )
    outputPointer = &defaultValue;
  else
    outputPointer = other.outputPointer;
}


//=========================================================================================================================================
// class Module:

//-----------------------------------------------------------------------------------------------------------------------------------------
// construction/destruction:

romos::Module::Module(const rosic::rsString &_name, int _x, int _y, bool _polyphonic)
{
  parentModule         = NULL;
  processFrame         = NULL;
  processBlock         = NULL;
  audioOutputs         = NULL;
  numInputs            = 0;
  outFrameStride       = 0;
  hasHeaderFlag        = true;
  moduleTypeIdentifier = ModuleTypeRegistry::UNKNOWN_MODULE_TYPE; // the factory will set this up when a concrete module is created
  this->polyphonic     = _polyphonic;
  this->name           = _name;
  this->x              = _x;
  this->y              = _y;
}

romos::Module::~Module()
{
  rassert( !hasOutgoingAudioConnections() );
    // before deleting a module, you should disconnect its outputs (which may otherwise still be referenced by other modules)
}

void romos::Module::cleanUp()
{
  freeMemory();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// setup::

void romos::Module::setModuleName(const rsString& newName)
{
  // \todo this function seems to be called twice when entering a new name - find out why, fix it
  if( name != newName )
    name = newName;
}

void romos::Module::setPositionXY(int newX, int newY, bool sortSiblingsAfterMove)
{
  x = newX;
  y = newY;
  if( sortSiblingsAfterMove == true && parentModule != NULL )
    parentModule->sortChildModuleArray();
}

void romos::Module::setPolyphonic(bool shouldBePolyphonic)
{
  polyphonic = shouldBePolyphonic;
  assignProcessingFunctions();
  if( parentModule != NULL )
    parentModule->childPolyphonyChanged(this); // containers need to keep track of the polyphony of their children
}

void romos::Module::connectInputPinTo(int inputPinIndex, Module *sourceModule, int sourceOutputPinIndex)
{
  if( inputPinIndex < 0 || inputPinIndex >= (int) inputPins.size() )
    return;

  //rassert( sourceModule != this );  // temporary - to catch a bug
  if( sourceModule != NULL )
  {
    //sourceModule->mapApparentSourceToProcessingSource(sourceModule, sourceOutputPinIndex);
    // after this call, sourceModule and sourceOutputPinIndex refer to the actual module from which we drag the output data, all proxies
    // have been resolved

    inputPins[inputPinIndex].sourceModule      = sourceModule;
    inputPins[inputPinIndex].outputIndex       = sourceOutputPinIndex;
    inputPins[inputPinIndex].outputPointer     = sourceModule->getOutputPointer(sourceOutputPinIndex);
    inputPins[inputPinIndex].outputFrameSize   = sourceModule->getOutputFrameStride();
    inputPins[inputPinIndex].outputVoiceStride = sourceModule->getOutputVoiceStride();
  }
  else
    disconnectInputPin(inputPinIndex);
}

void romos::Module::disconnectInputPin(int inputPinIndex)
{
  rassert( inputPinIndex >= 0 && inputPinIndex < (int) inputPins.size() ); // trying to disconnect a non-existent input pin
  inputPins[inputPinIndex].reset();
}

void romos::Module::disconnectAllInputPins()
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
    disconnectInputPin(i);
}

void romos::Module::disconnectInputPinsWithInputFrom(romos::Module *sourceModuleToDisconnect)
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule == sourceModuleToDisconnect )
      disconnectInputPin(i);
  }
}

void romos::Module::disconnectInputPinsWithInputFrom(romos::Module *sourceModuleToDisconnect, int outputPinIndex)
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule == sourceModuleToDisconnect && inputPins[i].outputIndex == outputPinIndex )
      disconnectInputPin(i);
  }
}

void romos::Module::disconnectOutputPin(int outputPinIndex)
{
  if( parentModule != NULL )
  {
    for(unsigned int i = 0; i < parentModule->getNumChildModules(); i++)
      parentModule->getChildModule(i)->disconnectInputPinsWithInputFrom(this, outputPinIndex);
  }
}

void romos::Module::disconnectAllOutputPins()
{
  if( parentModule != NULL )
  {
    for(unsigned int i = 0; i < parentModule->getNumChildModules(); i++)
      parentModule->getChildModule(i)->disconnectInputPinsWithInputFrom(this);
  }
}

void romos::Module::updateInputPointersAndInFrameStrides()
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule != NULL )
    {
      inputPins[i].outputPointer     = inputPins[i].sourceModule->getOutputPointer(inputPins[i].outputIndex);
      inputPins[i].outputFrameSize   = inputPins[i].sourceModule->getOutputFrameStride();
      inputPins[i].outputVoiceStride = inputPins[i].sourceModule->getOutputVoiceStride();
    }
  }
}


//-----------------------------------------------------------------------------------------------------------------------------------------
// inquiry:

int romos::Module::getNumVoices() const
{
  return voiceAllocator.getNumVoices();  // preliminary - to always allocate for polyphonic output

  if( isPolyphonic() )
    return voiceAllocator.getNumVoices();
  else
    return 1;
}

romos::Module* romos::Module::getTopLevelModule()
{
  if( parentModule == NULL )
    return this;
  else
    return parentModule->getTopLevelModule();
}

int romos::Module::getIndexWithinParentModule()
{
  romos::ModuleContainer* parent = getParentModule();
  if( parent != NULL )
    return parent->getIndexOfChildModule(this);
  else
    return -1;
}

AudioInputPinData romos::Module::getAudioInputPinData(int pinIndex) const
{
  rassert( pinIndex >= 0 && pinIndex < (int) inputPins.size() ); // trying to inquire about a non-existent input pin
  return inputPins.at(pinIndex);

}

double* romos::Module::getOutputPointer(int pinIndex) const
{
  //rassert( pinIndex >= 0  && pinIndex < (int) numOutputs );
  return audioOutputs + pinIndex;  // to be overriden in container...
}

int romos::Module::getOutputFrameStride() const
{
  return outFrameStride;
}

int romos::Module::getOutputVoiceStride() const
{
  if( isPolyphonic() )
    return getOutputFrameStride() * processingStatus.getBufferSize();
  else
    return 0;
}

bool romos::Module::isInputPinConnected(int pinIndex) const
{
  rassert( pinIndex >= 0 && pinIndex < (int) inputPins.size() ); // trying to inquire about a non-existent input pin
  return inputPins.at(pinIndex).sourceModule != NULL;
}

bool romos::Module::isConnectedToAudioOutput() const
{
  std::vector<romos::Module*> targetModules = getConnectedTargetModules();
  return romos::containsModuleOfType(targetModules, ModuleTypeRegistry::AUDIO_OUTPUT);
}

bool romos::Module::hasIncomingConnectionFrom(const romos::Module *sourceModule) const
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule == sourceModule )
      return true;
  }
  return false;
}

bool romos::Module::hasIncomingConnectionFrom(const romos::Module *sourceModule, int sourceOutputPinIndex) const
{
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule == sourceModule && inputPins[i].outputIndex == sourceOutputPinIndex )
      return true;
  }
  return false;
}

bool romos::Module::hasDelayedIncomingConnection() const
{
  for(unsigned int pinIndex = 0; pinIndex < inputPins.size(); pinIndex++)
  {
    if( inputPins[pinIndex].sourceModule != NULL && modulePointerLessByXY(this, inputPins[pinIndex].sourceModule) )
      return true;
  }
  return false;
}

std::vector<romos::Module*> romos::Module::getConnectedTargetModules() const
{
  if( parentModule != NULL )
    return parentModule->getConnectedTargetModulesOf(this);
  else
  {
    //triggerRuntimeError("Module::getConnectedTargetModules called for module that has no parent");
      // this function relies on an inquiry using the parent module - for free standing modules, it won't work. in production code, a
      // module always has a parent unless it's the top-level module in which case this function should not be called
    return std::vector<romos::Module*>();
  }
}

std::vector<romos::Module*> romos::Module::getConnectedTargetModulesOfPin(int outputPinIndex) const
{
  if( parentModule != NULL )
    return parentModule->getConnectedTargetModulesOf(this, outputPinIndex);
  else
  {
    //triggerRuntimeError("Module::getConnectedTargetModulesOfPin called for module that has no parent");
      // this function relies on an inquiry using the parent module - for free standing modules, it won't work. in production code, a
      // module always has a parent unless it's the top-level module in which case this function should not be called
    return std::vector<romos::Module*>();
  }
}

unsigned int romos::Module::getNumIncomingAudioConnections() const
{
  int result = 0;
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule != NULL )
      result++;
  }
  return  result;
}

std::vector<AudioConnection> romos::Module::getIncomingAudioConnections()
{
  std::vector<AudioConnection> result;
  for(unsigned int i = 0; i < inputPins.size(); i++)
  {
    if( inputPins[i].sourceModule != NULL )
      rosic::appendElement(result, AudioConnection(inputPins[i].sourceModule, inputPins[i].outputIndex, this, (int) i));
  }
  return result;
}

std::vector<AudioConnection> romos::Module::getOutgoingAudioConnections()
{
  std::vector<AudioConnection> result;
  ModuleContainer *parent = getParentModule();
  if( parent == NULL )
    return result;

  Module *sibling;  // with respect to "this"
  for(unsigned int i = 0; i < parent->getNumChildModules(); i++)
  {
    sibling = parent->getChildModule(i);
    for(unsigned int j = 0; j < sibling->getNumInputPins(); j++)
    {
      AudioInputPinData pin = sibling->getAudioInputPinData(j);
      if( pin.sourceModule == this )
        rosic::appendElement(result, AudioConnection(this, (int) pin.outputIndex, sibling, (int) j) );
    }
  }
  return result;
}

std::vector<AudioConnection> romos::Module::getOutgoingAudioConnectionsFromPin(int pinIndex)
{
  std::vector<AudioConnection> result;
  std::vector<AudioConnection> outgoingAudioConnections = getOutgoingAudioConnections();
  for(unsigned int i = 0; i < outgoingAudioConnections.size(); i++)
  {
    if( outgoingAudioConnections[i].getSourceOutputIndex() == pinIndex )
      rosic::appendElement(result, outgoingAudioConnections[i]);
  }
  return result;
}

void romos::Module::getExtremeCoordinates(std::vector<Module*> &modules, int &xMin, int &yMin, int &xMax, int &yMax)
{
  if( modules.size() == 0 )
  {
    xMin = yMin = xMax = yMax = 0;
    return;
  }
  xMin = yMin = INT_MAX;
  xMax = yMax = INT_MIN;
  int x, y;
  for(unsigned int i = 0; i < modules.size(); i++)
  {
    x = modules[i]->getPositionX();
    y = modules[i]->getPositionY();
    if( x < xMin )
      xMin = x;
    if( y < yMin )
      yMin = y;
    if( x > xMax )
      xMax = x;
    if( y > yMax )
      yMax = y;
  }
}

void romos::Module::getMidpointCoordinates(std::vector<Module*> &modules, int &xMid, int &yMid)
{
  int xMin, yMin, xMax, yMax;
  getExtremeCoordinates(modules, xMin, yMin, xMax, yMax);
  xMid = (xMin+xMax)/2;
  yMid = (yMin+yMax)/2;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// others:

void romos::Module::allocateMemory()
{
  //allocateAudioInputs();
  allocateAudioOutputs();
}

void romos::Module::freeMemory()
{
  delete[] audioOutputs;
  audioOutputs   = NULL;
}

/*
void romos::Module::allocateAudioInputs() // remove this middle-man function - serves no purpose anymore
{
  updateInputPointersAndInFrameStrides();
}
*/

void romos::Module::allocateAudioOutputs()
{
  delete[] audioOutputs;
  audioOutputs = NULL;
  int slotsToAllocate = processingStatus.getRequiredMemorySlotsPerPin() * outFrameStride;
  if( slotsToAllocate > 0)
  {
    audioOutputs = new double[slotsToAllocate];
    memset(audioOutputs, 0, slotsToAllocate*sizeof(double));
  }

  if( parentModule != NULL )
    parentModule->outputsWereReAllocated(this);
      // should update the input pointers and frameStride pointers for all child modules that are a target of this one
      // ...maybe just let them update all

  //std::vector<AudioConnection*> outgoingAudioConnections = getOutgoingAudioConnections();
  //for(unsigned int i = 0; i < outgoingAudioConnections.size(); i++)
  //  outgoingAudioConnections[i]->updateSourcePointer();
}





//-----------------------------------------------------------------------------------------------------------------------------------------
// (non-member) helper functions:

bool romos::modulePointerLessByXY(const romos::Module *left, const romos::Module *right)
{
  // make sure that input modules come first:
  if( left->isInputModule() && !right->isInputModule() )
    return true;
  if( right->isInputModule() && !left->isInputModule() )
    return false;

  // make sure that output modules come last:
  if( left->isOutputModule() && !right->isOutputModule() )
    return false;
  if( right->isOutputModule() && !left->isOutputModule() )
    return true;

  // compare by coordinates:
  if( left->getPositionX() < right->getPositionX() )
    return true;
  else if( right->getPositionX() < left->getPositionX() )
    return false;
  else
  {
    if( left->getPositionY() < right->getPositionY() )
      return true;
    else if( right->getPositionY() < left->getPositionY() )
      return false;
    else
      return false; // modules have same x- and y coordinates
  }
}

bool romos::containsModuleOfType(const std::vector<romos::Module*> &modules, int typeCode)
{
  for(unsigned int i = 0; i < modules.size(); i++)
  {
    if( modules.at(i)->getTypeIdentifier() == typeCode )
      return true;
  }
  return false;
}

/*
bool romos::modulePointerLessByYX(romos::Module *modulePointer1, romos::Module *modulePointer2)
{
  if( modulePointer1->getPositionY() < modulePointer2->getPositionY() )
    return true;
  else if( modulePointer2->getPositionY() < modulePointer1->getPositionY() )
    return false;
  else
  {
    if( modulePointer1->getPositionX() < modulePointer2->getPositionX() )
      return true;
    else if( modulePointer2->getPositionX() < modulePointer1->getPositionX() )
      return false;
    else
      return false; // modules have same x- and y coordinates
  }
}
*/

void romos::writeModuleStateToConsole(void *module, bool waitForKeyAfterOutput)
{
  //printf("%s", "\n");

  romos::Module *m = (romos::Module*) module;
  int maxPinIndex;

  if( m->getParentModule() != NULL )
    printf("%s %s", m->getParentModule()->getName().getRawString(), ": ");

  double pinValue;

  printf("%s %s", m->getName().getRawString(), " ");
  printf("%s %s", m->getTypeName().getRawString(), " ");
  printf("%s", "Ins: ");
  if( m->isInputModule() || m->isOutputModule() )
    maxPinIndex = 0;
  else
    maxPinIndex = m->getNumInputPins() - 1;
  for(int i = 0; i <= maxPinIndex; i++)
  {
    //pinValue = *(m->getAudioInputAddress() + i);  // may need offset for voice/frame later
    pinValue = *(m->inputPins[i].outputPointer);  // may need offset for voice/frame later
    printf("%.3f %s", pinValue, " ");
  }

  printf("%s", "Outs: ");
  if( m->isInputModule() || m->isOutputModule() )
    maxPinIndex = 0;
  else
    maxPinIndex = m->getNumOutputPins() - 1;
  for(int i = 0; i <= maxPinIndex; i++)
  {
    //pinValue = m->getAudioOutputAddress() + i;  // may need offset for voice/frame later
    pinValue = m->audioOutputs[i];                // may need offset for voice/frame later
    printf("%.3f %s", pinValue, " ");
  }

  if( waitForKeyAfterOutput == true )
    getchar();
}

void romos::retrieveModuleState(void *moduleAsVoid)
{
  romos::Module* module = (romos::Module*) moduleAsVoid;

  static const int maxNumVoices = 4;
  static const int maxNumPins   = 10;
  static const int maxNumFrames = 64;

  double ins [maxNumVoices][maxNumPins][maxNumFrames];
  double outs[maxNumVoices][maxNumPins][maxNumFrames];

  //rosic::fillWithZeros(ins,  maxNumVoices*maxNumPins*maxNumFrames);
  //rosic::fillWithZeros(outs, maxNumVoices*maxNumPins*maxNumFrames);

  for(int voiceIndex = 0; voiceIndex < maxNumVoices; voiceIndex++)
  {
    for(int pinIndex = 0; pinIndex < maxNumPins; pinIndex++)
    {
      for(int frameIndex = 0; frameIndex < maxNumFrames; frameIndex++)
      {
        ins [voiceIndex][pinIndex][frameIndex] = 0.0;
        outs[voiceIndex][pinIndex][frameIndex] = 0.0;
      }
    }
  }

  int numVoices     = rmin(maxNumVoices, module->getNumVoices());
  int numInputPins  = rmin(maxNumPins,   (int) module->getNumInputPins());
  int numOutputPins = rmin(maxNumPins,   (int) module->getNumOutputPins());
  int numFrames     = rmin(maxNumFrames, processingStatus.getBufferSize());


  double *pointer;
  int    frameDistance;
  int    voiceDistance;




  for(int voiceIndex = 0; voiceIndex < numVoices; voiceIndex++)
  {
    for(int pinIndex = 0; pinIndex < numInputPins; pinIndex++)
    {
      voiceDistance = module->getAudioInputPinData(pinIndex).outputFrameSize * processingStatus.getBufferSize();
      pointer       = module->getAudioInputPinData(pinIndex).outputPointer + voiceIndex * voiceDistance;
      frameDistance = module->getAudioInputPinData(pinIndex).outputFrameSize;
      for(int frameIndex = 0; frameIndex < numFrames; frameIndex++)
      {
        ins[voiceIndex][pinIndex][frameIndex] = *pointer;
        pointer += frameDistance;
      }
    }

    for(int pinIndex = 0; pinIndex < numOutputPins; pinIndex++)
    {
      //frameDistance = module->getNumOutputsInlined();
      //voiceDistance = module->outFrameStride * processingStatus.getBufferSize();
      frameDistance = module->getOutputFrameStride();
      voiceDistance = module->getOutputVoiceStride();
      pointer       = module->audioOutputs;
      for(int frameIndex = 0; frameIndex < numFrames; frameIndex++)
      {
        outs[voiceIndex][pinIndex][frameIndex] = *pointer;
        pointer += frameDistance;
      }
    }
  }


  pointer = module->audioOutputs;
  double dbg1[12];
  memcpy(dbg1, pointer, 12*sizeof(double));

  double timeAxis[maxNumFrames];
  fillWithIndex(timeAxis, maxNumFrames);
  //Plotter::plotData(numFrames, timeAxis, outs[0][0]);
  int dummy = 0;
}


void romos::triggerRuntimeError(const char *errorMessage)
{
  // insert code here to open a message box....

  DEBUG_BREAK;
}






