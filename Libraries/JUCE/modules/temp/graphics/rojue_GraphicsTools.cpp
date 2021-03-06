#include "rojue_GraphicsTools.h"


void rojue::copyImage(juce::Image *sourceImage, juce::Image *targetImage)
{
  int w = targetImage->getWidth();
  int h = targetImage->getHeight();
  if(  sourceImage->getWidth()  != targetImage->getWidth()
    || sourceImage->getHeight() != targetImage->getHeight()
    || sourceImage->getFormat() != targetImage->getFormat() )
  {
    Graphics g(*targetImage);
    g.fillAll(Colours::red);
    g.setColour(Colours::black);
    g.drawText("Images incompatible in rojue::copyImage", 0, 0, w, h, Justification::centred, false);
    return;
  }
  targetImage->setPixelData(0, 0, w, h, sourceImage->getSharedImage()->getPixelData(0, 0), sourceImage->getSharedImage()->getLineStride());
    // maybe optimize this by retrieving the BitmapData and doing a memcpy - but comparing with "do-nothing", it doesn't seem to be
    // much of a difference, so it probably doesn't matter -- hmm - in another try it used more

  //targetImage->clear(Rectangle<int>(0,0,w,h), Colours::white); // seems to use more CPU than setPixelData

  //int numPixels = w*h;
  //int byteSize  = targetImage->getSharedImage()->getPixelStride();
  //int dummy = 0;
}

int rojue::drawBitmapFontText(Graphics &g, int x, int y, const String& textToDraw, const BitmapFont* fontToUse, 
                              const Colour& colourToUse, int kerning, Justification justification)
{
  //jassert(colourToUse != Colours::yellow); // for test only
  if( kerning == -1 )
    kerning = fontToUse->getDefaultKerning();

  int hFlags = justification.getOnlyHorizontalFlags();
  if( justification.testFlags(hFlags & Justification::horizontallyCentred) )
    x -= fontToUse->getTextPixelWidth(textToDraw, kerning)/2;
  else if( justification.testFlags(hFlags & Justification::right) )
    x -= fontToUse->getTextPixelWidth(textToDraw, kerning);

  int vFlags = justification.getOnlyVerticalFlags();
  if( justification.testFlags(vFlags & Justification::verticallyCentred) )
    y -= fontToUse->getFontAscent()/2;
  else if( justification.testFlags(vFlags & Justification::bottom) )
    y -= fontToUse->getFontAscent();

  const  Image* image;
  tchar  c;

  // make sure that g's opacity is set up right, otherwise glyph-images may be drawn transparent
  // regardless of their own opacity values:
  g.setColour(colourToUse); 

  for(int i=0; i<textToDraw.length(); i++)
  {
    c     = textToDraw[i];
    image = fontToUse->getGlyphImage(c, colourToUse);
    //jassert( image != NULL );
    if( image != NULL )
    {
      g.drawImageAt(*image, x, y, false);
      x += fontToUse->getGlyphWidth(c) + kerning;
    }
  }

  return x;
}

Colour rojue::getMixedColour(const Colour colour1, const Colour colour2, 
                             double weight1, double weight2)
{
  float a1 = colour1.getFloatAlpha();
  float r1 = colour1.getFloatRed();
  float g1 = colour1.getFloatGreen();
  float b1 = colour1.getFloatBlue();

  float a2 = colour2.getFloatAlpha();
  float r2 = colour2.getFloatRed();
  float g2 = colour2.getFloatGreen();
  float b2 = colour2.getFloatBlue();

  uint8 a  = (uint8) (255.f * (weight1*a1 + weight2*a2));
  uint8 r  = (uint8) (255.f * (weight1*r1 + weight2*r2));
  uint8 g  = (uint8) (255.f * (weight1*g1 + weight2*g2));
  uint8 b  = (uint8) (255.f * (weight1*b1 + weight2*b2));

  return Colour(r, g, b, a);
}

void rojue::fillRectWithBilinearGradientSlow(Graphics &graphics, int x, int y, int w, int h,      
                                             Colour topLeftColour, Colour topRightColour,                       
                                             Colour bottomLeftColour, Colour bottomRightColour)
{
  // require at least 2 pixels widtha nd height:
  if( w <= 1 || h <= 1 )
    return;

  // check, if all four colours are equal, if so just fill the graphics object with that colour:
  if( topLeftColour == topRightColour 
    && topRightColour == bottomLeftColour
    && bottomLeftColour == bottomRightColour )
  {
    graphics.fillAll(topLeftColour);
  }

  // ToDo: use the alpha-channels of the colours as well..., conditional compilation for Mac/Win with 
  // different byte orderings for the a,r,g,b values

  // allocate an empty image-object: 
  Image *background = new Image(juce::Image::RGB, w, h, false);

  // allocate memory for the raw pixel-data (h lines, each line has w colums, 
  // each pixel has 3 bytes for blue, green and red (in that order)):
  uint8 *pixelData  = new uint8[h*w*3];

  int baseAddress;    
  // address for a blue-value (is followed by addresses for green and red)

  int baseAddressInc = 3*w; 
  // increment for the baseAddress per iteration of the inner loop 

  // get the colour-components of the 4 edges as float-values:
  // top-left (abbreviated as t_l):
  uint8 t_l_r = topLeftColour.getRed();
  uint8 t_l_g = topLeftColour.getGreen(); 
  uint8 t_l_b = topLeftColour.getBlue(); 
  // top-right (abbreviated as t_r):
  uint8 t_r_r = topRightColour.getRed();
  uint8 t_r_g = topRightColour.getGreen(); 
  uint8 t_r_b = topRightColour.getBlue(); 
  // bottom-left (abbreviated as b_l):
  uint8 b_l_r = bottomLeftColour.getRed();
  uint8 b_l_g = bottomLeftColour.getGreen(); 
  uint8 b_l_b = bottomLeftColour.getBlue(); 
  // bottom-right (abbreviated as b_r):
  uint8 b_r_r = bottomRightColour.getRed();
  uint8 b_r_g = bottomRightColour.getGreen(); 
  uint8 b_r_b = bottomRightColour.getBlue(); 

  // declare variables for the top and bottom line colour-components:
  uint8 t_r, t_g, t_b, b_r, b_g, b_b;

  // declare variables for the colour components at the current pixel (in 
  // double and int format:
  uint8   r, g, b;

  int p   = 0; // current x-position (in pixels, depends on i)
  int q   = 0; // current y-position (in pixels, depends on j)
  int wm1 = w-1;
  int hm1 = h-1;

  for(int i=0; i<w; i++)
  {
    // colour components of one pixel in the top-line:
    t_r = t_l_r + (p*(t_r_r-t_l_r))/wm1;
    t_g = t_l_g + (p*(t_r_g-t_l_g))/wm1;
    t_b = t_l_b + (p*(t_r_b-t_l_b))/wm1;

    // colour components of one pixel in the bottom-line:
    b_r = b_l_r + (p*(b_r_r-b_l_r))/wm1;
    b_g = b_l_g + (p*(b_r_g-b_l_g))/wm1;
    b_b = b_l_b + (p*(b_r_b-b_l_b))/wm1;

    // draw the current top-line pixel:
    baseAddress = 3*i;
    pixelData[baseAddress+0] = t_b;  // blue comes first,
    pixelData[baseAddress+1] = t_g;  // green comes second,
    pixelData[baseAddress+2] = t_r;  // red comes third in memory

    // draw the current bottom-line pixel:
    baseAddress = 3*i+w*(h-1)*3;
    pixelData[baseAddress+0] = b_b; 
    pixelData[baseAddress+1] = b_g;
    pixelData[baseAddress+2] = b_r;

    // increment the x-position:
    p++; 

    // fill the column between 'top' and 'bottom':
    baseAddress = 3*i;
    q = 1; // ? -> 1?
    for(int j=1; j<(h-1); j++) 
    {
      r = t_r + (q*(b_r-t_r))/hm1;
      g = t_g + (q*(b_g-t_g))/hm1;
      b = t_b + (q*(b_b-t_b))/hm1;

      baseAddress += baseAddressInc;

      pixelData[baseAddress+0] = b;  // the blue-value
      pixelData[baseAddress+1] = g;  // the green-value
      pixelData[baseAddress+2] = r;  // the red-value

      // increment the y-position:
      q++; 
    }
  }

  // copy the generated pixel-data into the image-object:
  background->setPixelData(0, 0, w, h, pixelData, 3*w);

  // draw the image into the graphics-object:
  graphics.drawImageAt(*background, x, y);

  // free dynamically allocated memory:
  delete   background;
  delete[] pixelData;
}

void rojue::fillRectWithBilinearGradient(Graphics &graphics, int x, int y, int w, int h, 
                                         Colour topLeftColour, Colour topRightColour, 
                                         Colour bottomLeftColour, Colour bottomRightColour)
{
	// test:
	//graphics.fillAll(bottomLeftColour);
	//return;
	
  // require at least 2 pixels width and height:
  if( w <= 1 || h <= 1 )
    return;

  // check, if all four colours are equal, if so just fill the graphics object with that colour:
  if( topLeftColour == topRightColour 
    && topRightColour == bottomLeftColour
    && bottomLeftColour == bottomRightColour )
  {
    graphics.fillAll(topLeftColour);
  }

  // ToDo: use the alpha-channels of the colours as well...

  // allocate an empty image-object: 
  Image *background = new Image(juce::Image::RGB, w, h, false);

  // allocate memory for the raw pixel-data (h lines, each line has w colums, 
  // each pixel has 3 bytes for blue, green and red (in that order)):
  uint8 *pixelData  = new uint8[h*w*3];

  int    baseAddress;    
  // address for a blue-value (is followed by addresses for green and red)

  int    baseAddressInc = 3*w; 
  // increment for the baseAddress per iteration of the inner loop 

  // get the colour-components of the 4 edges as float-values:
  // top-left (abbreviated as t_l):
  double t_l_r = topLeftColour.getFloatRed();
  double t_l_g = topLeftColour.getFloatGreen(); 
  double t_l_b = topLeftColour.getFloatBlue(); 
  // top-right (abbreviated as t_r):
  double t_r_r = topRightColour.getFloatRed();
  double t_r_g = topRightColour.getFloatGreen(); 
  double t_r_b = topRightColour.getFloatBlue(); 
  // bottom-left (abbreviated as b_l):
  double b_l_r = bottomLeftColour.getFloatRed();
  double b_l_g = bottomLeftColour.getFloatGreen(); 
  double b_l_b = bottomLeftColour.getFloatBlue(); 
  // bottom-right (abbreviated as b_r):
  double b_r_r = bottomRightColour.getFloatRed();
  double b_r_g = bottomRightColour.getFloatGreen(); 
  double b_r_b = bottomRightColour.getFloatBlue(); 

  // declare variables for the top and bottom line colour-components:
  double t_r, t_g, t_b, b_r, b_g, b_b;

  // declare variables for the colour components at the current pixel (in 
  // double and int format:
  double r, g, b;
  uint8  r_int, g_int, b_int;

  double p = 0.0; // current relative x-position (from 0.0...1.0, depends on i)
  double q = 0.0; // current relative y-position (from 0.0...1.0, depends on j)
  double pInc = 1.0/(double)(w-1); // increment per iteration for p
  double qInc = 1.0/(double)(h-1); // increment per (inner) iteration for q

  for(int i=0; i<w; i++)
  {
    // colour components of one pixel in the top-line:
    t_r    = (1.0-p)*t_l_r + p*t_r_r;
    t_g    = (1.0-p)*t_l_g + p*t_r_g;
    t_b    = (1.0-p)*t_l_b + p*t_r_b;

    // colour components of one pixel in the bottom-line:
    b_r    = (1.0-p)*b_l_r + p*b_r_r;
    b_g    = (1.0-p)*b_l_g + p*b_r_g;
    b_b    = (1.0-p)*b_l_b + p*b_r_b;

    // draw the current top-line pixel:
    baseAddress = 3*i;
    r_int = (uint8) (255 * t_r);
    g_int = (uint8) (255 * t_g);
    b_int = (uint8) (255 * t_b);
#ifndef _MSC_VER   // #ifdef JUCE_LITTLE_ENDIAN seems to not solve the wrong-color thing
    pixelData[baseAddress+0] = r_int; 
    pixelData[baseAddress+1] = g_int;  
    pixelData[baseAddress+2] = b_int;  		
#else
    pixelData[baseAddress+0] = b_int;  // blue comes first,
    pixelData[baseAddress+1] = g_int;  // green comes second,
    pixelData[baseAddress+2] = r_int;  // red comes third in memory
#endif

    // draw the current bottom-line pixel:
    baseAddress = 3*i+w*(h-1)*3;
    r_int = (uint8) (255 * b_r);
    g_int = (uint8) (255 * b_g);
    b_int = (uint8) (255 * b_b);
#ifndef _MSC_VER
    pixelData[baseAddress+0] = r_int; 
    pixelData[baseAddress+1] = g_int;  
    pixelData[baseAddress+2] = b_int;  		
#else
    pixelData[baseAddress+0] = b_int;  // blue comes first,
    pixelData[baseAddress+1] = g_int;  // green comes second,
    pixelData[baseAddress+2] = r_int;  // red comes third in memory
#endif

    // increment the relative x-position:
    p += pInc; 

    // fill the column between 'top' and 'bottom':
    baseAddress = 3*i;
    q = 0.0;
    for(int j=1; j<(h-1); j++) 
    {
      r = (1.0-q)*t_r + q*b_r;
      g = (1.0-q)*t_g + q*b_g;
      b = (1.0-q)*t_b + q*b_b;

      r_int = (uint8) (255 * r);
      g_int = (uint8) (255 * g);
      b_int = (uint8) (255 * b);
			
      baseAddress += baseAddressInc;

#ifndef _MSC_VER
      pixelData[baseAddress+0] = r_int;
      pixelData[baseAddress+1] = g_int;
      pixelData[baseAddress+2] = b_int;						
#else
      pixelData[baseAddress+0] = b_int;  // the blue-value
      pixelData[baseAddress+1] = g_int;  // the green-value
      pixelData[baseAddress+2] = r_int;  // the red-value			
#endif
			
      q += qInc; // increment the relative y-position:
    }
  }

  // copy the generated pixel-data into the image-object:
  background->setPixelData(0, 0, w, h, pixelData, 3*w);

  // draw the image into the graphics-object:
  graphics.drawImageAt(*background, x, y);

  // free dynamically allocated memory:
  delete   background;
  delete[] pixelData;
}

void rojue::fillRectWithBilinearGradient(Graphics &graphics, Rectangle<int> r,                                 
                                         Colour topLeftColour, Colour topRightColour,                                 
                                         Colour bottomLeftColour, Colour bottomRightColour)
{
  fillRectWithBilinearGradient(graphics, 
    r.getX(), r.getY(), r.getWidth(), r.getHeight(),
    topLeftColour, topRightColour, bottomLeftColour, bottomRightColour);
}


void rojue::fillRectWithBilinearGrayScaleGradient(Graphics &graphics, int x, int y, int w, int h, 
                                                  float topLeftWhite, float topRightWhite,                                            
                                                  float bottomLeftWhite, float bottomRightWhite)
{
  // allocate an empty image-object: 
  Image *background = new Image(juce::Image::RGB, w, h, false);

  // allocate memory for the raw pixel-data (h lines, each line has w colums, 
  // each pixel has 3 bytes for blue, green and red (in that order)):
  uint8 *pixelData  = new uint8[h*w*3];

  int    baseAddress;    
  // address for a blue-value (is followed by addresses for green and red)

  int    baseAddressInc = 3*w; 
  // increment for the baseAddress per iteration of the inner loop 

  // get the white values of the 4 edges as float-values:
  double t_l_w = topLeftWhite;
  double t_r_w = topRightWhite;
  double b_l_w = bottomLeftWhite;
  double b_r_w = bottomRightWhite;


  // declare variables for the top and bottom line white-values:
  double t_w, b_w;

  // declare variables for the colour components at the current pixel (in 
  // double and int format:
  double white;
  uint8  white_int;

  double p = 0.0; // current relative x-position (from 0.0...1.0, depends on i)
  double q = 0.0; // current relative y-position (from 0.0...1.0, depends on j)
  double pInc = 1.0/(double)(w-1); // increment per iteration for p
  double qInc = 1.0/(double)(h-1); // increment per (inner) iteration for q

  for(int i=0; i<w; i++)
  {
    // white-value of one pixel in the top-line:
    t_w    = (1.0-p)*t_l_w + p*t_r_w;

    // white-value of one pixel in the bottom-line:
    b_w    = (1.0-p)*b_l_w + p*b_r_w;

    // draw the current top-line pixel:
    baseAddress = 3*i;
    white_int = (uint8) (255 * t_w);
    pixelData[baseAddress+0] = white_int;  // blue comes first,
    pixelData[baseAddress+1] = white_int;  // green comes second,
    pixelData[baseAddress+2] = white_int;  // red comes third in memory

    // draw the current bottom-line pixel:
    baseAddress = 3*i+w*(h-1)*3;
    white_int = (uint8) (255 * b_w);
    pixelData[baseAddress+0] = white_int; 
    pixelData[baseAddress+1] = white_int;
    pixelData[baseAddress+2] = white_int;

    // increment the relative x-position:
    p += pInc; 

    // fill the column between 'top' and 'bottom':
    baseAddress = 3*i;
    q = 0.0;
    for(int j=1; j<(h-1); j++) 
    {
      white     = (1.0-q)*t_w + q*b_w;
      white_int = (uint8) (255 * white);

      baseAddress += baseAddressInc;

      pixelData[baseAddress+0] = white_int;  // the blue-value
      pixelData[baseAddress+1] = white_int;  // the green-value
      pixelData[baseAddress+2] = white_int;  // the red-value

      q += qInc; // increment the relative y-position:
    }
  }

  // copy the generated pixel-data into the image-object:
  background->setPixelData(0, 0, w, h, pixelData, 3*w);

  // draw the image into the graphics-object:
  graphics.drawImageAt(*background, x, y);

  // free dynamically allocated memory:
  delete   background;
  delete[] pixelData;
}

void rojue::fillRectWithDefaultBackground(Graphics &g, int x, int y, int w, int h)
{
  fillRectWithBilinearGradient(g, x, y, w, h, 
    Colours::white, Colour(190,200,225), Colour(190,200,225), Colours::white);
}

void rojue::fillRectWithDefaultBackground(Graphics &g, Rectangle<int> r)
{
  fillRectWithDefaultBackground(g, r.getX(), r.getY(), r.getWidth(), r.getHeight());
}


void rojue::drawTriangle(Graphics &g, float x1, float y1, float x2, float y2,                
                         float x3, float y3, bool fill)
{
  // make a path from the 3 given points:
  Path	trianglePath;
  trianglePath.startNewSubPath(x1, y1);
  trianglePath.lineTo(x2, y2);
  trianglePath.lineTo(x3, y3);
  trianglePath.lineTo(x1, y1);

  // draw or fill the path:
  if( fill == true )
    g.fillPath(trianglePath);
  else
    g.strokePath(trianglePath, PathStrokeType(1.0));
}

void rojue::drawBlockDiagramPlus(Graphics &g, float x, float y, float w, float h, float thickness)
{
  g.drawEllipse(x, y, w, h, thickness);
  g.drawLine(x, y+0.5f*h, x+w, y+0.5f*h, thickness);
  g.drawLine(x+0.5f*w, y, x+0.5f*w, y+h, thickness);
}

void rojue::fitLineToRectangle(double &x1, double &y1, double &x2, double &y2,              
                               double xMin, double yMin, double xMax, double yMax)
{
  // catch some special cases:
  if( x1 == x2 ) // vertical line, infinite slope
  {
    y1 = yMin;
    y2 = yMax;
    return;
  }
  if( y1 == y2 ) // horizontal line, zero slope
  {
    x1 = xMin;
    x2 = xMax;
    return;
  }

  // calculate slope 'a' and constant term 'b' for the line equation y = a*x + b:
  double a = (y2-y1)/(x2-x1);
  double b = y1-a*x1;

  // calculate x- and y-values at the rectangle's boundaries:
  double yAtMinX = a*xMin+b;
  double yAtMaxX = a*xMax+b;
  double xAtMinY = (yMin-b)/a;
  double xAtMaxY = (yMax-b)/a;

  if( yAtMinX > yMin && yAtMinX < yMax )    
    // line intercepts left boundary
  {
    x1 = xMin;    
    y1 = a*xMin+b;
    if( xAtMaxY > xMin && xAtMaxY < xMax )      
      // line intercepts left and top boundary (chops off the top-left corner)
    {
      x2 = (yMax-b)/a;
      y2 = yMax;
    }
    else if( xAtMinY > xMin && xAtMinY < xMax ) 
      // line intercepts left and bottom boundary (chops off the bottom-left corner)
    {
      x2 = (yMin-b)/a;
      y2 = yMin;
    }
    else 
      // line intercepts right boundary (divides the rectangle into top and bottom)
    {
      x2 = xMax;
      y2 = a*xMax+b;
    }
  }
  else if( yAtMaxX > yMin && yAtMaxX < yMax )
    // line intercepts right boundary
  {
    x2 = xMax;
    y2 = a*xMax+b;
    if( xAtMaxY > xMin && xAtMaxY < xMax )
      // line intercepts right and top boundary (chops off top right corner)
    {
      x1 = (yMax-b)/a;
      y1 = yMax;
    }
    else if( xAtMinY > xMin && xAtMinY < xMax )
      // line intercepts right and bottom boundary (chops off bottom right corner)
    {
      x1 = (yMin-b)/a;
      y1 = yMin;
    }
  }
  else if( xAtMaxY > xMin && xAtMaxY < xMax && xAtMinY > xMin && xAtMinY < xMax )
    // line intercepts top and bottom boundary (divides the rectangle into left and right)
  {
    x1 = (yMin-b)/a;
    y1 = yMin;
    x2 = (yMax-b)/a;
    y2 = yMax;
  }

}

void rojue::clipLineToRectangle(double &x1, double &y1, double &x2, double &y2,                
                                double xMin, double yMin, double xMax, double yMax)
{
  bool   firstIsInside  = false;
  bool   secondIsInside = false;
  double xIn, yIn, xOut, yOut;

  if( x1 >= xMin && x1 <= xMax && y1 >= yMin && y1 <= yMax )
    firstIsInside = true;
  if( x2 >= xMin && x2 <= xMax && y2 >= yMin && y2 <= yMax )
    secondIsInside = true;

  if( firstIsInside && secondIsInside )
    return;
  if( firstIsInside == false && secondIsInside == false )
  {
    fitLineToRectangle(x1, y1, x2, y2, xMin, yMin, xMax, yMax);
    return;
  }

  if( firstIsInside && secondIsInside == false )
  {
    xIn  = x1;
    yIn  = y1;
    xOut = x2;
    yOut = y2;
  }
  else if( secondIsInside && firstIsInside == false )
  {    
    xIn  = x2;
    yIn  = y2;
    xOut = x1;
    yOut = y1;
  }

  // we have the case that one point is inside and the other one outside the rectangle and we have
  // already determined which is which - now we must find out the clipped outside value

  if( x1 == x2 ) // vertical line, infinite slope
  {
    if( yOut > yMax )
    {
      yOut = yMax;
      if( firstIsInside && secondIsInside == false )
      {
        y2 = yOut;
        return;
      }
      else if( secondIsInside && firstIsInside == false )
      {    
        y1 = yOut;
        return;
      }
    }
    else if( yOut < yMin )
    {
      yOut = yMin;
      if( firstIsInside && secondIsInside == false )
      {
        y2 = yOut;
        return;
      }
      else if( secondIsInside && firstIsInside == false )
      {    
        y1 = yOut;
        return;
      }
    }

  }
  if( y1 == y2 ) // horizontal line, zero slope
  {
    // something to do? i dont thibk so, but...

  }

  // calculate y-values at the rectangle's left and right boundaries:
  double a = (yOut-yIn)/(xOut-xIn);
  double b = yIn-a*xIn;
  double yAtMinX = a*xMin+b;
  double yAtMaxX = a*xMax+b;
  double xAtMinY = (yMin-b)/a;
  double xAtMaxY = (yMax-b)/a;

  if( xOut >= xMax ) // line intercepts right, top or bottom boundary
  {
    if( yAtMaxX > yMax )      // line intercepts top boundary
    {
      xOut = xAtMaxY;
      yOut = yMax;
    }
    else if( yAtMaxX < yMin ) // line intercepts bottom boundary
    {
      xOut = xAtMinY;
      yOut = yMin;
    }
    else                      // line intercepts right boundary
    {
      xOut = xMax;
      yOut = yAtMaxX;
    }
  }
  else if( xOut <= xMin ) // line intercepts left, top or bottom boundary
  {
    if( yAtMinX > yMax )      // line intercepts top boundary
    {
      xOut = xAtMaxY;
      yOut = yMax;
    }
    else if( yAtMinX < yMin ) // line intercepts bottom boundary
    {
      xOut = xAtMinY;
      yOut = yMin;
    }
    else                      // line intercepts left boundary
    {
      xOut = xMin;
      yOut = yAtMinX;
    }
  }
  else if( yOut > yMax ) // line intercepts top boundary
  {
    xOut = xAtMaxY;    
    yOut = yMax;
  }
  else if( yOut > yMax ) // line intercepts bottom boundary
  {
    xOut = xAtMinY;    
    yOut = yMin;
  }


  if( firstIsInside && secondIsInside == false )
  {
    x2 = xOut;
    y2 = yOut;
  }
  else if( secondIsInside && firstIsInside == false )
  {    
    x1 = xOut;
    y1 = yOut;
  }
}