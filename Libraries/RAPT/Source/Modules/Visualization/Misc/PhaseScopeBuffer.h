#ifndef RAPT_PHASESCOPEBUFFER_H_INCLUDED
#define RAPT_PHASESCOPEBUFFER_H_INCLUDED

/** Implements the buffering for a phasescope analyzer. 

\todo
-subclass this from Image
-factor out some of the drawing functions into Image (or an ImageDrawer class) */

template<class TSig, class TPix, class TPar> // signal, pixel, parameter types
class PhaseScopeBuffer
{

public:

  /** Constructor. */
  PhaseScopeBuffer();

  /** Destructor. */
  virtual ~PhaseScopeBuffer();

  /** Sets the sample rate. */
  void setSampleRate(TPar newSampleRate);

  /** Sets the frame rate. */
  void setFrameRate(TPar newFrameRate);

  /** Sets the overall brightness. This parameter, together with the sample rate, determines the 
  weight by which new dot are added in. */
  void setBrightness(TPix newBrightness);

  /** Sets the time it takes for "color" to decay away. */
  void setDecayTime(TPar newDecayTime);

  /** Sets the size of the matrix into which we buffer the incoming samples. This should correspond 
  to the pixel size of the display. */
  void setSize(int newWidth, int newHeight);

  /** Switches anti-aliasing on/off. */
  void setAntiAlias(bool shouldAntiAlias);

  /** Sets up the density of the lines the connect our actual datapoints. It actually determines the 
  number of artificial datapoints that are inserted (by linear interpolation) between our actual 
  incoming datapoints. If set to zero, it will just draw the datapoints as dots. */
  void setLineDensity(TPar newDensity);

  /** Sets up a weight by which each pixel is not only accumulated into its actual place but also 
  into the neighbouring pixels to add more weight or thickness. It should be a value between 0 
  and 1. */
  void setPixelSpread(TPar newSpread);
  // maybe rename to setDotSpread

  /** Converts the raw left- and right signal amplitude values to the matrix indices, where the 
  data should be written. This is the xy-pixel coordinates (kept still as real numbers), where the 
  display is to be illuminated in response to the given amplitude values. */
  void convertAmplitudesToMatrixIndices(TSig &x, TSig &y);
  // rename to toPixelCoordinates

  /** Accepts one input sample frame for buffering. */
  void bufferSampleFrame(TSig left, TSig right);

  /** Applies our pixel decay-factor to the matrix of buffered values. This assumed to be called at 
  the frame rate. */
  void applyPixelDecay();

  /** Resets the internal buffer to all zeros. */
  void reset();

  /** Returns the buffered value at the given (x,y) pixel position. No bounds checking is done - 
  you must make sure that the indices are valid. */
  inline TPix getValueAt(int x, int y) { return buffer[x][y]; }

  /** Returns a pointer to our internally stored data matrix. */
  TPix** getDataMatrix() { return buffer; }

  /** Returns the sample rate. */
  inline TPar getSampleRate() { return sampleRate; }

  /** Returns the frame rate. */
  inline TPar getFrameRate() { return frameRate; }

  /** Returns the width in pixels. */
  inline int getWidth() { return width; }

  /** Returns the height in pixels. */
  inline int getHeight() { return height; }

protected:

  /** Returns the distance between the pixels with coordinates (x1,y1) and (x1,y2). This distance 
  is used in our line drawing function to determine the number of additional dots that are to be 
  inserted between our actual datapoints. */
  //float pixelDistance(float x1, float y1, float x2, float y2);

  /** Adds a line to the given x,y coordinates (in pixel coordinates). The starting point of the 
  line are the old pixel coordinates xOld, yOld (member variables). It takes into account our line 
  density - when it's set to zero, it will just draw a dot at the new given position. */
  void addLineTo(TSig x, TSig y);

  /** Adds a dot into our data matrix at the given position (given in matrix-index (i.e. pixel-) 
  coordinates using bilinear deinterpolation for anti-aliasing. */
  void addDot(TSig x, TSig y, TPix intensity);

  /** Like addDot but without anti-aliasing (we just round the coordinates to the nearest 
  integer). */
  void addDotFast(TSig x, TSig y, TPix intensity);

  /** Allocates the memory for our data matrix buffer. */
  void allocateBuffer();

  /** Frees the memory for our data matrix buffer. */
  void freeBuffer();

  /** Updates the pixel decay factor according to the settings of frame rate and desired decay 
  time. */
  void updateDecayFactor();

  /** Updates the pixel insertion factor (i.e. the weight by which new dots are multiplied when 
  they get added in according to the settings of sample rate and brightness parameter. */
  void updateInsertFactor();

  /** Accumulates the given value into the accumulator accu. This accumulation amounts to adding
  the value and the saturating at 1. \todo maybe this can be optimized and/or a different accumulation
  function can be used to get different contrast and saturation behavior.  */
  inline void accumulate(TPix &accu, TPix value)
  {
    //accu = min(1.f, accu + value);

    // this seems to be actually cheaper than the min() call and saturates more smoothly:
    accu += value;
    accu /= (1 + value);
    //accu /= (1 + accu);
  }

  bool antiAlias;      // flag to switch anti-aliasing on/off
  int  width, height;  // pixel width and height

  TPar sampleRate;
  TPar frameRate;
  TPar decayTime;      // pixel illumination time
  TPar decayFactor;    // factor by which pixels decay (applied at frameRate)
  TPar lineDensity;    // density of the artificial points between actual datapoints
  TPar thickness;      // line (or dot) thickness from 0 to 1. 0: one pixel, 1: 3 pixels
                       // maybe rename to spread or weight or something

  TPix brightness;     // determines weight by which dots are added in
  TPix insertFactor;   // factor by which are pixels "inserted" (applied at sampleRate)

  TSig xOld, yOld;     // pixel coordinates of old datapoint (one sample ago)

  // the actual matrix-shaped buffer, we use the indexing common in image processing: the first 
  // index points to a horizontal line and the second index is the pixel in this line, so the first
  // index runs from 0 to height-1 and the second from 0 to width-1:
  TPix **buffer;
  TPix *bufferFlat; 
};

#endif