#include "romos_ModularSystemTest.h"
using namespace romos;

//-----------------------------------------------------------------------------------------------------------------------------------------
// construction/destruction:

ModularSystemTest::ModularSystemTest(const char *testName) 
: UnitTest(testName)
{
  tolerance         = 0.0;
  signalLength      = 1000;
  //signalLength      = 10;    // for test
  numInputChannels  = 2;
  numOutputChannels = 2;
  blockSize         = 200;

  topLevelModule    = modularSystem.getTopLevelModule();
  inModuleL         = (AudioInputModule*)  topLevelModule->getChildModule(0);
  inModuleR         = (AudioInputModule*)  topLevelModule->getChildModule(1);
  outModuleL        = (AudioOutputModule*) topLevelModule->getChildModule(2);
  outModuleR        = (AudioOutputModule*) topLevelModule->getChildModule(3);

  voiceAllocator.reset();  // maybe use a function initialize which is even stronger (resets even more state variables)

  fillInputSignalArraysRandomly(1);
  clearOutputSignalArrays();
  clearDesiredOutputSignalArrays();
  fillTimeAxisWithSampleIndices();
}

ModularSystemTest::~ModularSystemTest()
{
  //ModuleFactory::deleteModule(topLevelModule);
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// running the tests:

bool ModularSystemTest::runTest()
{
  initTest();  
  fillDesiredOutputSignalArrays();
  bool testPassed = false;

  clearOutputSignalArrays();
  modularSystem.reset();
  processOutputSignal(true);
  testPassed = doOutputsMatchDesiredOutputs();

  //plotDesiredAndActualOutput(signalLength, 0);
  //plotOutputErrors(signalLength, 0);

  return testPassed;
}

void ModularSystemTest::initTest()
{
  createAndConnectTestChildModules();
  fillInputSignalArraysWithTestSignal();
  fillDesiredOutputSignalArrays();
}

bool ModularSystemTest::doOutputsMatchDesiredOutputs()
{
  // maybe wrap into a function printMaxError
  double maxError = 0.0;
  for(int channelIndex = 0; channelIndex < numOutputChannels; channelIndex++)
  {
    rosic::subtract(outputs[channelIndex], desiredOutputs[channelIndex], outputErrors[channelIndex], signalLength);
    double maxErrorInChannel = rosic::maxAbs(outputErrors[channelIndex], signalLength);
    if( maxErrorInChannel > maxError )
      maxError = maxErrorInChannel;
  }
  //printf("%e %s", maxError, "\n");

  bool outputsMatch = true;
  for(int channelIndex = 0; channelIndex < numOutputChannels; channelIndex++)
  {
    for(int frameIndex = 0; frameIndex < signalLength; frameIndex++)
    {
      outputsMatch &= fabs((float) outputs[channelIndex][frameIndex] - (float) desiredOutputs[channelIndex][frameIndex]) <= tolerance;
      //outputsMatch &= (float) outputs[channelIndex][frameIndex] == (float) desiredOutputs[channelIndex][frameIndex];
    }
  }
  return outputsMatch;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// filling of the internal arrays:

void ModularSystemTest::fillInputSignalArraysRandomly(int seed)
{
  randomUniform(-1.0, 1.0, seed);
  for(int channelIndex = 0; channelIndex < numInputChannels; channelIndex++)
  {
    for(int frameIndex = 0; frameIndex < signalLength; frameIndex++)
      inputs[channelIndex][frameIndex] = randomUniform(-1.0, 1.0); 
  }

  rosic::convertBuffer(inputs[0], inputsFloat[0], signalLength);
  rosic::convertBuffer(inputs[1], inputsFloat[1], signalLength);
}

void ModularSystemTest::clearInputSignalArrays()
{
  memset(inputs, 0, maxNumInputChannels * maxSignalLength * sizeof(double));
}

void ModularSystemTest::clearOutputSignalArrays()
{
  memset(outputs, 0, maxNumOutputChannels * maxSignalLength  * sizeof(double));
}

void ModularSystemTest::clearDesiredOutputSignalArrays()
{
  memset(desiredOutputs, 0, maxNumOutputChannels * maxSignalLength * sizeof(double));
}

void ModularSystemTest::fillTimeAxisWithSampleIndices()
{
  for(int frameIndex = 0; frameIndex < maxSignalLength; frameIndex++)
    timeAxis[frameIndex] = (double) frameIndex;
}
   
void ModularSystemTest::fillInputSignalArraysWithTestSignal()    
{
  fillInputSignalArraysRandomly(1);
}

void ModularSystemTest::fillDesiredOutputSignalArrays()
{
  clearDesiredOutputSignalArrays();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// inquiry:




//-----------------------------------------------------------------------------------------------------------------------------------------  
// processing:

void ModularSystemTest::processOutputSignal(bool useSinglePrecision)
{
  copyBuffer(inputs[0],      outputs[0],      signalLength);
  copyBuffer(inputs[1],      outputs[1],      signalLength);
  copyBuffer(inputsFloat[0], outputsFloat[0], signalLength);
  copyBuffer(inputsFloat[1], outputsFloat[1], signalLength);

  int blockStart      = 0;
  int remainingFrames = signalLength;
  int tmpBlockSize    = blockSize;

  while( remainingFrames > 0 )
  {
    tmpBlockSize = blockSize;  // shrink to match size until next event later

    int numFramesUntilNextEvent = INT_MAX;
    if( events.size() > 0 )
    {
      numFramesUntilNextEvent = events[0].getDeltaFrames() - blockStart;
      tmpBlockSize            = rmin(numFramesUntilNextEvent, blockSize);
      if( numFramesUntilNextEvent == 0 )
      {
        NoteEvent currentEvent = events[0];
        handleEvent(currentEvent);
        rosic::removeElementByIndex(events, 0);
      }
    }

    if( useSinglePrecision == true )
      modularSystem.getBlockOfSampleFramesStereo(&(outputsFloat[0][blockStart]), &(outputsFloat[1][blockStart]), tmpBlockSize);
    else
      modularSystem.getBlockOfSampleFramesStereo(&(outputs[0][blockStart]),      &(outputs[1][blockStart]),      tmpBlockSize);

    blockStart      += tmpBlockSize;
    remainingFrames -= tmpBlockSize;
  }


  if( useSinglePrecision == true )
  {
    rosic::convertBuffer(outputsFloat[0], outputs[0], signalLength);
    rosic::convertBuffer(outputsFloat[1], outputs[1], signalLength);
  }
}

void ModularSystemTest::handleEvent(NoteEvent eventToHandle)
{
  if( eventToHandle.getVelocity() == 0 )
    modularSystem.noteOff(eventToHandle.getKey());
  else
    modularSystem.noteOn(eventToHandle.getKey(), eventToHandle.getVelocity());
}







//-----------------------------------------------------------------------------------------------------------------------------------------
// setup:


//-----------------------------------------------------------------------------------------------------------------------------------------
// information output functions:


void ModularSystemTest::plotDesiredAndActualOutput(int numFramesToPlot, int startFrame)
{
  Plotter::plotData(numFramesToPlot, &timeAxis[startFrame], desiredOutputs[0]+startFrame, desiredOutputs[1]+startFrame, 
                    outputs[0]+startFrame, outputs[1]+startFrame);
}

void ModularSystemTest::plotOutputErrors(int numFramesToPlot, int startFrame)
{
  Plotter::plotData(numFramesToPlot, &timeAxis[startFrame], outputErrors[0]+startFrame, outputErrors[1]+startFrame);
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// internal functions:

