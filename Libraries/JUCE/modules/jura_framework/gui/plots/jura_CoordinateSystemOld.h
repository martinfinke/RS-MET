#ifndef jura_CoordinateSystemOld_h
#define jura_CoordinateSystemOld_h 


// is this really the right file from the old codebase?
// OK - yes - it seems, we have always used the ones with suffix "Old"

/**

This class represents the visible range for a CoordinateSystem (and is used inside this class).

*/

class JUCE_API CoordinateSystemRangeOld
{

public:

  /** Standard Constructor. */
  CoordinateSystemRangeOld(double initMinX = -2.2, double initMaxX = +2.2, 
    double initMinY = -2.2, double initMaxY = +2.2);

  /** Destructor. */
  virtual ~CoordinateSystemRangeOld();  

  /** Returns the minimum x-value. */
  virtual double getMinX() const { return minX; }

  /** Returns the maximum x-value. */
  virtual double getMaxX() const { return maxX; }

  /** Returns the minimum y-value. */
  virtual double getMinY() const { return minY; }

  /** Returns the maximum y-value. */
  virtual double getMaxY() const { return maxY; }

  /** Sets the minimum x-value. */
  virtual void setMinX(double newMinX);

  /** Sets the maximum x-value. */
  virtual void setMaxX(double newMaxX);

  /** Sets up the minimum and maximum value for x .*/
  virtual void setRangeX(double newMinX, double newMaxX);

  /** Sets the minimum y-value. */
  virtual void setMinY(double newMinY);

  /** Sets the maximum y-value. */
  virtual void setMaxY(double newMaxY); 

  /** Sets up the minimum and maximum value for y .*/
  virtual void setRangeY(double newMinY, double newMaxY);

  /** Lets the range represented by the member-variables be clipped to another range such that
  the minima/maxima are no smaller/larger than those of the rangeToClipTo. */
  virtual void clipRange(CoordinateSystemRangeOld rangeToClipTo);


  /** Compares two ranges for equality. */
  bool operator==(const CoordinateSystemRangeOld& r2) const  
  {
    if( r2.minX == minX && r2.maxX == maxX && r2.minY == minY && r2.maxY == maxY )
      return true;
    else
      return false;
  }

  /** Compares two ranges for inequality. */
  bool operator!=(const CoordinateSystemRangeOld& r2) const  
  {
    return !(*this == r2);
  }

protected:

  double minX, maxX, minY, maxY;

  juce_UseDebuggingNewOperator;
};


//=================================================================================================

/** This class is a component, intended to be used as base-class for all components that need some
underlying coordinate-system, such as function-plots, XY-pads, etc. It takes care of the coordinate
axes, a coarse and a fine grid, conversion between component-coordinates and the coordinates in the
desired coordinate-system (which can be lin- or log-scaled).

\todo:
-factor out a CoordinateConverter2D class that contains all the math but is devoid of any drawing 
 operations
-actually, this needs only to be written for 1D and then 2 instances can be used, but we may use a 
 2D wrapper and delegations
-rename to Plot or PlotBase  */

class JUCE_API CoordinateSystemOld : virtual public DescribedComponent
{

  friend class CoordinateSystemZoomer;

public:

  enum captionPositions
  {
    NO_CAPTION = 0,
    TOP_CENTER,
    CENTER
  };

  enum axisPositions
  {
    INVISIBLE = 0,
    ZERO,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
  };

  enum axisAnnotationPositions
  {
    NO_ANNOTATION = 0,
    LEFT_TO_AXIS,
    RIGHT_TO_AXIS,
    ABOVE_AXIS,
    BELOW_AXIS
  };

  CoordinateSystemOld(const juce::String& newDescription = juce::String("some 2D widget"));
  ///< Constructor.

  virtual ~CoordinateSystemOld();
  ///< Destructor.

  //-----------------------------------------------------------------------------------------------
  // component-overrides:

  virtual void mouseDown(const MouseEvent& e);
  /**< Lets a context menu pop up when the right button is clicked to allow export of the content
  as image or svg drawing. */

  virtual void mouseEnter(const MouseEvent& e);
  /**< Overrides mouseEnter for displaying the inspection Label. */

  virtual void mouseMove(const MouseEvent& e);
  /**< Overrides mouseMove for displaying the inspection Label. */

  virtual void mouseExit(const MouseEvent& e);
  /**< Overrides mouseExit for displaying the inspection Label. */

  virtual void mouseDrag(const MouseEvent& e);
  /**< Overrides mouseDrag to let it be called by a CoordinateSystemZoomer (override it in
  subclasses, if you want to respond to mouseDrag events). */

  virtual void mouseUp(const MouseEvent& e);
  /**< Overrides mouseUp to let it be called by a CoordinateSystemZoomer (override it in
  subclasses, if you want to respond to mouseUp events). */

  virtual void mouseDoubleClick(const MouseEvent& e);
  /**< Overrides mouseDoubleClick to let it be called by a CoordinateSystemZoomer (override it in
  subclasses, if you want to respond to mouseDoubleClick events). */

  //virtual void mouseWheelMove(const MouseEvent &e, float wheelIncrementX, float wheelIncrementY);
  virtual void mouseWheelMove(const MouseEvent &e, const MouseWheelDetails &wheel);
  /**< Overrides mouseWheelMove to let it be called by a CoordinateSystemZoomer (override it in
  subclasses, if you want to respond to mouseWheelMove events). */


  virtual void resized();
  ///< Overrides the resized()-function of the component base-class.

  virtual void paint(Graphics &g);
  ///< Overrides the paint-function of the component base-class.

  //-----------------------------------------------------------------------------------------------
  // range-management:

  virtual void setMaximumRange(double newMinX, double newMaxX, double newMinY, double newMaxY);
  /**< Sets the maximum for the currently visible range. For logarithmic x- and/or y-axis-scaling,
  make sure that the respective minimum value is greater than zero! */

  virtual void setMaximumRange(CoordinateSystemRangeOld newMaximumRange);
  /**< Sets the maximum for the currently visible range. */

  virtual void setMaximumRangeX(double newMinX, double newMaxX);
  /**< Sets the maximum visible range for the y-axis. */

  virtual void setMaximumRangeY(double newMinY, double newMaxY);
  /**< Sets the maximum visible range for the y-axis. */

  virtual CoordinateSystemRangeOld getMaximumRange() { return maximumRange; }
  /**< Returns the maximum for the currently visible range. */

  virtual void setMaximumRangeMinX(double newMinX);
  /**< Sets the minimum value for the range of x. */

  virtual double getMaximumRangeMinX() { return maximumRange.getMinX(); }
  /**< Returns the minimum value for the range of x. */

  virtual void setMaximumRangeMaxX(double newMaxX);
  /**< Sets the maximum value for the range of x. */

  virtual double getMaximumRangeMaxX() { return maximumRange.getMaxX(); }
  /**< Returns the maximum value for the range of x. */

  virtual void setMaximumRangeMinY(double newMinY);
  /**< Sets the minimum value for the range of y. */

  virtual double getMaximumRangeMinY() { return maximumRange.getMinY(); }
  /**< Returns the minimum value for the range of y. */

  virtual void setMaximumRangeMaxY(double newMaxY);
  /**< Sets the maximum value for the range of y. */

  virtual double getMaximumRangeMaxY() { return maximumRange.getMaxY(); }
  /**< Returns the maximum value for the range of y. */

  virtual void setCurrentRange(double newMinX, double newMaxX, double newMinY, double newMaxY);
  /**< Sets the currently visible range. For logarithmic x- and/or y-axis-scaling, make sure that
  the respective minimum value is greater than zero! */

  virtual void setCurrentRange(CoordinateSystemRangeOld newRange);
  /**< Sets the currently visible range. */

  virtual void setCurrentRangeX(double newMinX, double newMaxX);
  /**< Sets the currently visible range for the y-axis. */

  virtual void setCurrentRangeY(double newMinY, double newMaxY);
  /**< Sets the currently visible range for the y-axis. */

  virtual CoordinateSystemRangeOld getCurrentRange() { return currentRange; }
  /**< Returns the currently visible range. */

  virtual void setCurrentRangeMinX(double newMinX);
  /**< Sets the minimum value of x. */

  virtual double getCurrentRangeMinX() { return currentRange.getMinX(); }
  /**< Returns the minimum value of x. */

  virtual void setCurrentRangeMaxX(double newMaxX);
  /**< Sets the maximum value of x. */

  virtual double getCurrentRangeMaxX() { return currentRange.getMaxX(); }
  /**< Returns the maximum value of x. */

  virtual void setCurrentRangeMinY(double newMinY);
  /**< Sets the minimum value of y. */

  virtual double getCurrentRangeMinY() { return currentRange.getMinY(); }
  /**< Returns the minimum value of y. */

  virtual void setCurrentRangeMaxY(double newMaxY);
  /**< Sets the maximum value of y. */

  virtual double getCurrentRangeMaxY() { return currentRange.getMaxY(); }
  /**< Returns the maximum value of y. */

  /** Returns a string that represents the info-line to be shown when mouse is over pixel (x,y). */
  virtual juce::String getInfoLineForPixelPosition(int x, int y);


  virtual PlotColourScheme getPlotColourScheme() const { return plotColourScheme; }

  //-----------------------------------------------------------------------------------------------
  // appearance:

  /** Sets up the colour-scheme. */
  virtual void setColourScheme(const PlotColourScheme& newColourScheme);

  /** Sets up the colour-scheme to be used for the right-click popup menu. */
  virtual void setPopUpColourScheme(const WidgetColourScheme& newColourScheme)
  {
    popUpColourScheme = newColourScheme;
  }

  /** Sets up the colour-scheme from an XmlElement. */
  virtual void setColourSchemeFromXml(const XmlElement& xml);

  /** Changes one of the colours for the graphs if a colour with this index exists (and returns
  true in this case) - if the index is out of range, it does nothing and returns false. */
  virtual bool changeGraphColour(int index, Colour newColour);


  virtual void setCaption(const juce::String &newCaption, int newPosition = TOP_CENTER);
  /**< Sets up a caption for the CoordinateSystemOld and the position where it should appear. */

  virtual void setAxisPositionX(int newAxisPositionX);
  /**< Sets the position of the x-axis. For possible values see
  enum positions. */

  virtual void setAxisPositionY(int newAxisPositionY);
  /**< Sets the position of the y-axis. For possible values see
  enum positions. */

  virtual void setupAxisX(double newMin, double newMax, bool shouldBeLogScaled, double newLogBase,
    int newAxisPosition, double newCoarseGridInterval, double newFineGridInterval);
  /**< Sets up several x-axis parameters at once */

  virtual void setupAxisY(double newMin, double newMax, bool shouldBeLogScaled, double newLogBase,
    int newAxisPosition, double newCoarseGridInterval,
    double newFineGridInterval);
  /**< Sets up several y-axis parameters at once */

  virtual void setHorizontalCoarseGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the horizontal coarse grid. */

  virtual void setHorizontalCoarseGridInterval(double newGridInterval);
  /**< Sets the interval of the horizontal coarse grid. */

  virtual void setHorizontalCoarseGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the horizontal coarse grid. */

  virtual void setHorizontalFineGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the horizontal fine grid. */

  virtual void setHorizontalFineGridInterval(double newGridInterval);
  /**< Sets the interval of the horizontal fine grid. */

  virtual void setHorizontalFineGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the horizontal fine grid. */

  virtual void setVerticalCoarseGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the vertical coarse grid. */

  virtual void setVerticalCoarseGridInterval(double newGridInterval);
  /**< Sets the interval of the vertical coarse grid. */

  virtual void setVerticalCoarseGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the vertical coarse grid. */

  virtual void setVerticalFineGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the vertical fine grid. */

  virtual void setVerticalFineGridInterval(double newGridInterval);
  /**< Sets the interval of the vertical fine grid. */

  virtual void setVerticalFineGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the vertical fine grid. */

  virtual void setRadialCoarseGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the radial coarse grid. */

  virtual void setRadialCoarseGridInterval(double newGridInterval);
  /**< Sets the interval of the radial coarse grid. */

  virtual void setRadialCoarseGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the radial coarse grid. */

  virtual void setRadialFineGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the radial fine grid. */

  virtual void setRadialFineGridInterval(double newGridInterval);
  /**< Sets the interval of the radial fine grid. */

  virtual void setRadialFineGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the radial fine grid. */

  virtual void setAngularCoarseGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the angular coarse grid. */

  virtual void setAngularCoarseGridInterval(double newGridInterval);
  /**< Sets the interval of the angular coarse grid. */

  virtual void setAngularCoarseGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the angular coarse grid. */

  virtual void setAngularFineGridVisible(bool shouldBeVisible);
  /**< Sets the visibility of the angular fine grid. */

  virtual void setAngularFineGridInterval(double newGridInterval);
  /**< Sets the interval of the angular fine grid. */

  virtual void setAngularFineGrid(double newGridInterval,
    bool   shouldBeVisible);
  /**< Sets the interval and visibility of the angular fine grid. */

  virtual void setAngleUnitToDegrees(bool shouldBeInDegrees = true);
  /**< Sets the unit of the angle (as used by the angular grid) to degrees. If
  false, radiant will be assumed. */

  virtual bool isHorizontalCoarseGridVisible();
  /**< Informs, if the horizontal coarse grid is visible. */

  virtual bool isHorizontalFineGridVisible();
  /**< Informs, if the horizontal fine grid is visible. */

  virtual bool isVerticalCoarseGridVisible();
  /**< Informs, if the vertical coarse grid is visible. */

  virtual bool isVerticalFineGridVisible();
  /**< Informs, if the vertical fine grid is visible. */

  virtual bool isRadialCoarseGridVisible();
  /**< Informs, if the radial coarse grid is visible. */

  virtual bool isRadialFineGridVisible();
  /**< Informs, if the radial fine grid is visible. */

  virtual bool isAngularCoarseGridVisible();
  /**< Informs, if the angular coarse grid is visible. */

  virtual bool isAngularFineGridVisible();
  /**< Informs, if the angular fine grid is visible. */

  virtual double getHorizontalCoarseGridInterval();
  /**< Returns the interval of the horizontal coarse grid. */

  virtual double getHorizontalFineGridInterval();
  /**< Returns the interval of the horizontal fine grid. */

  virtual double getVerticalCoarseGridInterval();
  /**< Returns the interval of the vertical coarse grid. */

  virtual double getVerticalFineGridInterval();
  /**< Returns the interval of the vertical fine grid. */

  virtual double getRadialCoarseGridInterval();
  /**< Returns the interval of the radial coarse grid. */

  virtual double getRadialFineGridInterval();
  /**< Returns the interval of the radial fine grid. */

  virtual double getAngularCoarseGridInterval();
  /**< Returns the interval of the angular coarse grid. */

  virtual double getAngularFineGridInterval();
  /**< Returns the interval of the angular fine grid. */

  virtual void useLogarithmicScale(bool   shouldBeLogScaledX,
    bool   shouldBeLogScaledY,
    double newLogBaseX = 2.0,
    double newLogBaseY = 2.0);
  /**< Decides if either the x-axis or the y-axis or both should be
  logarithmically scaled and sets up the base for the logarithms. */

  virtual void useLogarithmicScaleX(bool   shouldBeLogScaledX,
    double newLogBaseX = 2.0);
  /**< Decides, if the x-axis should be logarithmically scaled and sets up the
  base for the logarithm. */

  virtual bool isLogScaledX();
  /**< Informs, whether the x-axis is logarithmically scaled or not. */

  virtual void useLogarithmicScaleY(bool   shouldBeLogScaledY,
    double newLogBaseY = 2.0);
  /**< Decides, if the y-axis should be logarithmically scaled and sets up the
  base for the logarithm. */

  virtual bool isLogScaledY();
  /**< Informs, whether the y-axis is logarithmically scaled or not. */

  virtual void setAxisLabels(const juce::String &newLabelX, const juce::String &newLabelY,
    int newLabelPositionX = ABOVE_AXIS, int newLabelPositionY = RIGHT_TO_AXIS);
  /**< Sets the labels for the axes and their position. */

  virtual void setAxisLabelX(const juce::String &newLabelX, int newLabelPositionX = ABOVE_AXIS);
  /**< Sets the label for the x-axis and its position. */

  virtual void setAxisLabelY(const juce::String &newLabelY, int newLabelPositionY = RIGHT_TO_AXIS);
  /**< Sets the label for the y-axis and its position. */

  virtual void setAxisValuesPositionX(int newValuesPositionX);
  /**< Switches x-value annotation between below or above the x-axis
  (or off). */

  virtual void setAxisValuesPositionY(int newValuesPositionY);
  /**< Switches y-value annotation between left to or right to the y-axis
  (or off). */

  /** This function is used to pass a function-pointer with the address of a function which has a 
  double-parameter and a juce::String as return-value. The function will be used to convert the 
  values on the x-axis into corresponding strings for display on the axis. */
  virtual void setStringConversionForAxisX(
    juce::String (*newConversionFunction) (double valueToBeConverted));

  /** Like setStringConversionForAxisX but for y-axis. */
  virtual void setStringConversionForAxisY(
    juce::String (*newConversionFunction) (double valueToBeConverted));

  /** Like setStringConversionForAxisX but not for the axis but for the info-line. */
  virtual void setStringConversionForInfoLineX(
    juce::String (*newConversionFunction) (double valueToBeConverted));

  /** Like setStringConversionForAxisY but not for the axis but for the info-line. */
  virtual void setStringConversionForInfoLineY(
    juce::String (*newConversionFunction) (double valueToBeConverted));

  //-----------------------------------------------------------------------------------------------
  // state-management:

  virtual XmlElement* getStateAsXml(
    const juce::String& stateName = juce::String("CoordinateSystemState")) const;
  /**< Creates an XmlElement from the current state and returns it. */

  virtual bool setStateFromXml(const XmlElement &xmlState);
  /**< Restores a state based on an XmlElement which should have been created
  with the getStateAsXml()-function. */

  virtual XmlElement* getPlotAsSVG(int width, int height);
  /**< Returns the drawing as SVG compliant XmlElement. The caller must take care to delete the
  pointer to the XmlElement when it's not needed anymore. */

  virtual Image* getPlotAsImage(int width, int height);
  /**< Renders the plot to an image object of given width and height. The caller must take care to
  delete the pointer to the image when it's not needed anymore. */

  virtual void openExportDialog(int defaultWidth, int defaultHeight, 
    const juce::String &defaultFormat, const juce::File& defaultTargetFile);
  /**< Opens a dialog window to export the content of the CoordinateSystemOld to a png-image file 
  or svg vector drawing file. */

  virtual void setAutoReRendering(bool shouldAutomaticallyReRender);
  /**< With this function, the automatic re-rendering of the underlying image can be turned on or
  off. If on (default), everytime you change a parameter which will change the appearance of
  the CoordinateSystemOld, it will be re-rendered. However, when you want to change many
  parameters at a time, this can be wasteful in terms of CPU-load. In these cases, it can be
  useful to switch the automatic re-rendering temporarily off. */

  MouseCursor currentMouseCursor;
  /**< We define a member for the mouse-cursor which is to be showed in order to let a
  CoordinateSystemZoomer access this. This is required, because the zoomer object must be above
  the actual CoordinateSystemOld and therefore prevent the CoordinateSystemOld to set it's own
  MouseCursor. Instead we just assign the member mouse-cursor and let the zoomer retrieve it. */

protected:

  /** Opens the PopupMenu that appears on right clicks. */
  void openRightClickPopupMenu();

  virtual void drawCoordinateSystem(Graphics &g, Image* targetImage = NULL, 
    XmlElement* targetSVG = NULL);
  /**< Draws all the stuff either on the internal image which will be displayed as the components
  content or on an arbitrary image (if only the first optional pointer argumnet is nonzero) or on
  an arbitrary image and an SVG compliant XmlElement (if both poiters are nonzero). */

  virtual void drawHorizontalGrid(Graphics &g, double interval,
    bool exponentialSpacing, Colour gridColour, float lineThickness,
    Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws a horizontal grid with a given interval in a given colour. Gets called by
  drawCoordinateSystem(). */

  virtual void drawVerticalGrid(Graphics &g, double interval,
    bool exponentialSpacing, Colour gridColour, float lineThickness,
    Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws a vertical grid with a given interval in a given colour. Gets called by
  drawCoordinateSystem(). */

  virtual void drawRadialGrid(Graphics &g, double interval, bool exponentialSpacing,
    Colour gridColour, float lineThickness, Image* targetImage = NULL,
    XmlElement* targetSVG = NULL);
  /**< Draws a radial grid with a given interval in a given colour. Gets called by
  drawCoordinateSystem(). */

  virtual void drawAngularGrid(Graphics &g, double interval, Colour gridColour,
    float lineThickness, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws an angular grid with a given interval in a given colour. Gets called by
  drawCoordinateSystem(). */

  virtual void drawCaption(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the caption/headline. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisX(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the x-axis. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisY(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the y-axis. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisLabelX(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the x-axis' label. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisLabelY(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the y-axis' label. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisValuesX(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the numeric values at the x-axis. Gets called by drawCoordinateSystem(). */

  virtual void drawAxisValuesY(Graphics &g, Image* targetImage = NULL, XmlElement* targetSVG = NULL);
  /**< Draws the numeric values at the y-axis. Gets called by drawCoordinateSystem(). */

  virtual void transformToImageCoordinates(double &x, double &y, const Image* theImage);
  /**< Function for converting the x- and y-coordinate values into the corresponding coordinates
  in an image. */

  virtual void transformFromImageCoordinates(double &x, double &y, const Image* theImage);
  /**< Function for converting the x- and y-coordinate values measured in the image's coordinate
  system to the corresponding coordinates of our plot. */

  virtual void transformToComponentsCoordinates(double &x, double &y);
  /**< Function for converting the x- and y-coordinate values into the corresponding coordinates in
  the component (double precision version).*/

  virtual void transformToComponentsCoordinates(float &x, float &y);
  /**< Function for converting the x- and y-coordinate values into the corresponding coordinates in
  the component (single precision version).*/

  virtual void transformFromComponentsCoordinates(double &x, double &y);
  /**< Function for converting the x- and y-coordinate values measured in the components coordinate
  system to the corresponding coordinates of our plot (double precision version). */

  virtual void transformFromComponentsCoordinates(float &x, float &y);
  /**< Function for converting the x- and y-coordinate values measured in the components coordinate
  system to the corresponding coordinates of our plot (single precision version). */

  virtual void addLineToSvgDrawing(XmlElement* theSVG, float x1, float y1, float x2, float y2,
    float thickness, Colour colour, bool withArrowHead = false);
  /**< Adds a line to an SVG drawing. */

  virtual void addTextToSvgDrawing(XmlElement* theSVG, juce::String theText, float x, float y,
    Justification justification = Justification::centredLeft);
  /**< Adds text-string to an SVG drawing. */

  /** Draws a text on the Coordinatesystem using a BitmapFont. */
  virtual void drawBitmapText(Graphics &g, const juce::String &text, double x, double y,
    double w, double h, BitmapFont const* font, Justification justification);

  virtual void updateBackgroundImage();
  /**< Updates the image object (re-draws it). Will be called, when something about the
  CoordinateSystemOld's appearance-settings was changed. */

  virtual void updateScaleFactors();
  /**< Updates the scale-factors which are needed when transforming from the CoordinateSystemOld's
  coordinates to Component's coordinates and vice versa. Will be called by setBounds(),
  setRange() and useLogarithmicScale(). */

  double getPlotHeight(Image *targetImage = NULL);
  /**< Returns either the height of this component or the height of the image (if the pointer is
  non-NULL). */

  double getPlotWidth(Image *targetImage = NULL);
  /**< Returns either the height of this component or the height of the image (if the pointer is
  non-NULL). */

  CoordinateSystemRangeOld currentRange, maximumRange;
  /**< The range- and maximum-range object for the coordinate system. */

  double scaleX;
  double scaleY;
  double pixelsPerIntervalX;
  double pixelsPerIntervalY;

  int    axisPositionX;
  int    axisPositionY;
  int    axisLabelPositionX;
  int    axisLabelPositionY;
  int    axisValuesPositionX;
  int    axisValuesPositionY;

  juce::String axisLabelX;
  juce::String axisLabelY;

  bool   horizontalCoarseGridIsVisible;
  bool   horizontalFineGridIsVisible;
  bool   verticalCoarseGridIsVisible;
  bool   verticalFineGridIsVisible;
  bool   radialCoarseGridIsVisible;
  bool   radialFineGridIsVisible;
  bool   angularCoarseGridIsVisible;
  bool   angularFineGridIsVisible;

  double horizontalCoarseGridInterval;
  double horizontalFineGridInterval;
  double verticalCoarseGridInterval;
  double verticalFineGridInterval;
  double radialCoarseGridInterval;
  double radialFineGridInterval;
  double angularCoarseGridInterval;
  double angularFineGridInterval;

  bool   angleIsInDegrees;

  bool   logScaledX;
  double logBaseX;
  bool   logScaledY;
  double logBaseY;
  bool   logScaledRadius;
  double logBaseRadius;


  //bool   valuePopup;

  bool useBitmapFont;
  bool showPositionAsDescription;
  bool showPopUpOnRightClick;

  int          captionPosition;
  juce::String captionString;
  DrawableText caption;
  /**< A caption for the CoordinateSystemOld to be used to describe what is shown. */

  Image*  backgroundImage;
  /**< This image will be used for the appearance of the coodinate system, it will be updated via
  the updateBackgroundImage()-function when something about the coordinate-system (axes, grid,
  etc.) changes - but only if autoReRender is true (which it is by default). */

  bool autoReRenderImage;
  // see above


  // functions for converting coordinates into strings:
  juce::String (*stringConversionForAxisX)     (double valueToConvert);
  juce::String (*stringConversionForAxisY)     (double valueToConvert);
  juce::String (*stringConversionForInfoLineX) (double valueToConvert);
  juce::String (*stringConversionForInfoLineY) (double valueToConvert);


  // color-scheme management:
  PlotColourScheme   plotColourScheme;
  WidgetColourScheme popUpColourScheme;

  //---------------------------------------------------------------------------------------------
  // make obsolete inherited methods unavailable to client code:

  /*
  virtual void setColours(const Colour newBackgroundColour, const Colour newOutlineColour,
    const Colour newHandleColour, const Colour newTextColour, const Colour newSpecialColour1,
    const Colour newSpecialColour2) {};
    */
  //virtual void setColourScheme(const WidgetColourScheme& newColourScheme) {};
  //virtual WidgetColourScheme getColourScheme() const { return RWidget::getColourScheme(); }

  juce_UseDebuggingNewOperator;
};


#endif
