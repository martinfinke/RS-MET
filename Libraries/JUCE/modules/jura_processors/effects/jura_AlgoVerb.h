#ifndef jura_AlgoVerbAudioModule_h
#define jura_AlgoVerbAudioModule_h


/** This class wraps rosic::AlgoVerb into a rosof::AudioModule to facilitate its use as plugIn. */

class AlgoVerbAudioModule : public AudioModule
{

  friend class AlgoVerbModuleEditor;

public:

  //-----------------------------------------------------------------------------------------------
  // construction/destruction:

  AlgoVerbAudioModule(CriticalSection *newPlugInLock, rosic::AlgoVerb *algoVerbToWrap);

  //-----------------------------------------------------------------------------------------------
  // automation and state management:

  virtual void parameterChanged(Parameter* parameterThatHasChanged);

  //-----------------------------------------------------------------------------------------------
  // parameter settings:

  virtual void setSampleRate(double newSampleRate)
  {
    wrappedAlgoVerb->setSampleRate((float)newSampleRate);
  }

  //---------------------------------------------------------------------------------------------
  // audio processing:

  virtual void getSampleFrameStereo(double* inOutL, double* inOutR)
  {
    wrappedAlgoVerb->getSampleFrameStereo(inOutL, inOutR);
  }

  //---------------------------------------------------------------------------------------------
  // event processing:

  virtual void reset()
  {
    wrappedAlgoVerb->reset();
  }

protected:

  void initializeAutomatableParameters();

  rosic::AlgoVerb *wrappedAlgoVerb;

  juce_UseDebuggingNewOperator;
};

//=================================================================================================

class AlgoVerbModuleEditor : public AudioModuleEditor, public RComboBoxObserver
{

public:

  //---------------------------------------------------------------------------------------------
  // construction/destruction:

  AlgoVerbModuleEditor(CriticalSection *newPlugInLock, 
    AlgoVerbAudioModule* newAlgoVerbAudioModule);

  //---------------------------------------------------------------------------------------------
  // callbacks:

  virtual void rButtonClicked(RButton *buttonThatWasClicked);
  virtual void rComboBoxChanged(RComboBox *rComboBoxThatHasChanged);
  virtual void resized();
  virtual void updateWidgetsAccordingToState();



protected:

  /** Updates the plots on the GUI. */
  virtual void updatePlots();

  AlgoVerbAudioModule *algoVerbModuleToEdit;

  juce::Rectangle<int> globalRect, earlyRect, lateRect;

  RTextField *globalLabel, *earlyLabel, *lateLabel;

  RSlider   *dryWetSlider, *lateLevelSlider, *referenceDelayTimeSlider, *densitySlider, 
    *diffusionSlider, *latePreDelaySlider, *decayTimeSlider, *lowDecayScaleSlider, 
    *lowCrossFreqSlider, *highDecayScaleSlider, *highCrossFreqSlider;

  RComboBox *feedbackMatrixComboBox, *injectionVectorComboBox, *outputVectorComboBox;

  RButton   *pingButton, *earlyPingButton, *latePingButton, *allpassModeButton, 
    *wetPinkingButton;

  /*
  RSlider *coarseSlider, *fineSlider, *grainLengthInMillisecondsSlider, *grainLengthInCyclesSlider,
  *grainLengthInBeatsSlider, *feedbackSlider, *dryWetSlider;
  RComboBox *grainLengthUnitComboBox;
  RButton *invertButton, *reverseButton, *antiAliasButton; // *formantPreserveButton, *monoButton;
  */

  WaveformDisplay *impulseResponsePlot;


  juce_UseDebuggingNewOperator;
};


#endif 