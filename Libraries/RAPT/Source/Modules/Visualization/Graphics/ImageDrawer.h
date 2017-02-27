#ifndef RAPT_IMAGEDRAWER_H_INCLUDED
#define RAPT_IMAGEDRAWER_H_INCLUDED

/** A baseclass for drawing on images. It consolidates the data and functionality that all drawers 
have in common, regardless of what they draw. Subclasses like LineDrawer will do the actual drawing
and likely define more data and methods. */

template<class TPix, class TWgt, class TCor>  // pixel, weight, coordinate types
class ImageDrawer
{

public:

  /** The blend modes for determining a new pixel color as function of its current color, an 
  incoming color and a blend amount. */
  enum blendModes
  {
    BLEND_LINEAR = 0,
    BLEND_ADD_CLIP,
    BLEND_ADD_SATURATE,
    //BLEND_MULTIPLY,

    NUM_BLEND_MODES
  };


  /** \name Construction/Destruction */

  /** Constructor. */
  ImageDrawer(Image<TPix> *imageToDrawOn);


  /** \name Setup */

  /** Sets the image that we will draw on. */
  void setImageToDrawOn(Image<TPix> *imageToDrawOn);

  /** Sets the color with which we draw on the image. */
  inline void setColor(TPix newColor) { color = newColor; }

  /** Selects one of the blend modes. The blend mode is the function that is used to compute a new
  color for a pixel from an incoming desired color, the pixel's old color and a weight between
  0 and 1 that determines how to mix the old and the new color. */
  void setBlendMode(int newMode);
    // maybe make it possible to provide a function pointer to a custom blend function 


  /** \name Drawing */

  /** Blends the pixel color in the image at given coordinates with the color of this drawer 
  according to some weight. If the weight is 0, the pixel's color is unchanged, if it's 1, the new 
  color has the biggest impact.  What exactly happens depends on the blend-mode setting. */
  inline void plot(int x, int y, TWgt weight)
  {
    blendFunction((*image)(x, y), TPix(weight) * color);
  }


protected:

  Image<TPix> *image;

  TPix color;
  int blendMode;
  void (*blendFunction)(TPix& pixel, TPix color, TWgt weight);

  // maybe have a color member


  /** \name Blend functions */

  static void linearBlend(   TPix &pixel, TPix color, TWgt blend);
  static void addAndClip(    TPix &pixel, TPix color, TWgt blend);
  static void addAndSaturate(TPix &pixel, TPix color, TWgt blend);

};

//=================================================================================================

/** A class for drawing straight lines. The lines do not necessarily have to have a solid color -
instead, you can choose one of the line profiles that lets the color vary in dependence on the
distance of a pixel from the ideal geometric line.  */

template<class TPix, class TWgt, class TCor>  // pixel, weight, coordinate types
class LineDrawer : public ImageDrawer<TPix, TWgt, TCor>
{

public:


  /** The blend modes for determining a new pixel color as function of its current color, an 
  incoming color and a blend amount. */
  enum lineProfiles
  {
    PROFILE_FLAT = 0,      // solid/flat color
    PROFILE_LINEAR,        // metallic
    PROFILE_PARABOLIC,     // plastic
    PROFILE_CUBIC,         // cloudy
    //PROFILE_PARAMETRIC,  // add later: define solid-width and 2 slopes (like in the dot)

    NUM_LINE_PROFILES
  };


  /** Constructor. */
  LineDrawer(Image<TPix> *imageToDrawOn); // : ImageDrawer(imageToDrawOn) {}


  /** \name Setup */

  /** Selects one of the line profiles. */
  void setLineProfile(int newProfile);

  /** Sets the width of the line in pixels. */
  void setWidth(TCor newWidth);

  /** Sets the drawer up to draw half-circular end caps, if true. If false, the caps will be 
  rectangular. */
  void setRoundCaps(bool shouldBeRound);


  /** \name Drawing */

  /** Draws a line from (x0,y0) to (x1,y1). */
  void drawLine(TCor x0, TCor y0, TCor x1, TCor y1);

  // have members for simplified lineDrawing: drawLineWu, drawLineBresenham

protected:

  bool roundCaps = true;
  TCor w2;                // lineWidth/2
  int  profileIndex;
  TWgt (*lineProfile)(TCor distance, TCor halfWidth);

  //vector<Line2D> lines;


  /** \name Line profile functions */

  static TWgt profileFlat(     TCor distance, TCor halfWidth);
  static TWgt profileLinear(   TCor distance, TCor halfWidth);
  static TWgt profileParabolic(TCor distance, TCor halfWidth);
  static TWgt profileCubic(    TCor distance, TCor halfWidth);

};

#endif