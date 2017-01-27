template<class TPix, class TWgt, class TCor>
ImagePainter<TPix, TWgt, TCor>::ImagePainter(Image<TPix> *imageToPaintOn, AlphaMask<TWgt> *maskToUse)
{
  antiAlias = true;
  useMask = false;
  setNeighbourWeightsForSimpleDot(0, 0);

  setImageToPaintOn(imageToPaintOn);
  setAlphaMaskForDot(maskToUse);
}

// setup

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::setImageToPaintOn(Image<TPix> *imageToPaintOn)
{
  image = imageToPaintOn;
  //if(image != nullptr)
  //{
  //  wi = image->getWidth();
  //  hi = image->getHeight();
  //}
  //else
  //  wi = hi = 0;
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::setAlphaMaskForDot(AlphaMask<TWgt> *maskToUse)
{
  mask = maskToUse;
  //wb = mask->getWidth();
  //hb = mask->getHeight();
  // todo: handle nullptr case
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::setNeighbourWeightsForSimpleDot(TWgt straight, TWgt diagonal)
{
  straightNeighbourWeight = straight;
  diagonalNeighbourWeight = diagonal;
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::setAntiAlias(bool shouldAntiAlias)
{
  antiAlias = shouldAntiAlias;
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::setUseAlphaMask(bool shouldUseMask)
{
  useMask = shouldUseMask;
}

// painting

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::paintDot(TCor x, TCor y, TPix color)
{
  // todo: get rid of this dispatcher code - use a member function pointer instead.

  if(useMask)
  {
    if(antiAlias)
      paintDotViaMask(x, y, color);
    else
      paintDotViaMask((int) round(x), (int) round(y), color);
  }
  else
  {
    if(antiAlias)
      paintDot3x3(x, y, color, straightNeighbourWeight, diagonalNeighbourWeight);
    else
      paintDot3x3((int) round(x), (int) round(y), color, 
        straightNeighbourWeight, diagonalNeighbourWeight);
  }
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::paintDot3x3(int x, int y, TPix color, TWgt weightStraight, 
  TWgt weightDiagonal)
{
  int wi = image->getWidth();
  int hi = image->getHeight();

  if(x >= 0 && x < wi && y >= 0 && y < hi)
    accumulate((*image)(x, y), color);

  // apply thickness:
  if(weightStraight > 0.f && x >= 1 && x < wi-1 && y >= 1 && y < hi-1)
  {
    TPix a, ta, sa;
    a  = color;
    ta = a * (TPix)weightStraight;
    sa = a * (TPix)weightDiagonal;

    accumulate((*image)(x-1, y-1), sa);
    accumulate((*image)(x,   y-1), ta);
    accumulate((*image)(x+1, y-1), sa);

    accumulate((*image)(x-1, y  ), ta);
    accumulate((*image)(x+1, y  ), ta);

    accumulate((*image)(x-1, y+1), sa);
    accumulate((*image)(x,   y+1), ta);
    accumulate((*image)(x+1, y+1), sa);
  }
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::paintDot3x3(TCor x, TCor y, TPix color, TWgt weightStraight, 
  TWgt weightDiagonal)
{
  int wi = image->getWidth();
  int hi = image->getHeight();

  int xi = (int)floor(x);  // integer part of x
  int yi = (int)floor(y);  // integer part of y
  x -= xi;                 // fractional part of x
  y -= yi;                 // fractional part of y

  // compute weights for bilinear deinterpolation (maybe factor out):
  TPix a, b, c, d;
  d = TPix(x*y);
  c = TPix(y)-d;
  b = TPix(x)-d;
  a = d+TPix(1-x-y);

  // compute values to accumulate into the 4 pixels:
  a *= color;  // (xi,   yi)
  b *= color;  // (xi+1, yi)
  c *= color;  // (xi,   yi+1)
  d *= color;  // (xi+1, yi+1)

  // accumulate values into the pixels:
  if(xi >= 0 && xi < wi-1 && yi >= 0 && yi < hi-1)
  {
    accumulate((*image)(xi,   yi  ), a);
    accumulate((*image)(xi+1, yi  ), b);
    accumulate((*image)(xi,   yi+1), c);
    accumulate((*image)(xi+1, yi+1), d);
  }
  // \todo: maybe use a "blend" function that can be set up by the user as a function pointer 
  // this will allow for different blend modes

  // apply thickness:
  if(weightStraight > 0.f && xi >= 1 && xi < wi-2 && yi >= 1 && yi < hi-2)
  {
    TPix t, s, sa, sb, sc, sd, ta, tb, tc, td;
    t = (TPix)weightStraight;      // weight for direct neighbour pixels
    s = (TPix)weightDiagonal;      // weight for diagonal neighbour pixels

    sa = s*a;
    sb = s*b;
    sc = s*c;
    sd = s*d;

    ta = t*a;
    tb = t*b;
    tc = t*c;
    td = t*d;

    accumulate((*image)(xi-1, yi-1), sa);
    accumulate((*image)(xi,   yi-1), ta+sb);
    accumulate((*image)(xi+1, yi-1), tb+sa);
    accumulate((*image)(xi+2, yi-1), sb);

    accumulate((*image)(xi-1, yi  ), ta+sc);
    accumulate((*image)(xi,   yi  ), sd+tb+tc);
    accumulate((*image)(xi+1, yi  ), sc+ta+td);
    accumulate((*image)(xi+2, yi  ), tb+sd);

    accumulate((*image)(xi-1, yi+1), tc+sa);
    accumulate((*image)(xi,   yi+1), sb+ta+td);
    accumulate((*image)(xi+1, yi+1), sa+tb+tc);
    accumulate((*image)(xi+2, yi+1), td+sb);

    accumulate((*image)(xi-1, yi+2), sc);
    accumulate((*image)(xi,   yi+2), tc+sd);
    accumulate((*image)(xi+1, yi+2), td+sc);
    accumulate((*image)(xi+2, yi+2), sd);
  }
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::paintDotViaMask(int x, int y, TPix color)
{
  int wi = image->getWidth();
  int hi = image->getHeight();
  int wm = mask->getWidth();
  int hm = mask->getHeight();

  // write coordinates in target image:
  int xs = x  - wm/2;    // start x-coordinate
  y      = y  - hm/2;    // start y coordinate
  int xe = xs + wm-1;    // end x coordinate
  int ye = y  + hm-1;    // end y coordinate

  // read coordinates in mask image:
  int mxs = 0;          // start x
  int my  = 0;          // start y

  // checks to not write beyond image bounds:
  if(xs < 0)
  {
    mxs = -xs;
    xs  = 0;
  }
  if(y < 0)
  {
    my = -y;
    y  =  0;
  }
  if(xe >= wi)
    xe = wi-1;
  if(ye >= hi)
    ye = hi-1;

  // the actual painting loop over the pixels in target image and brush image:
  int mx;
  while(y <= ye)
  {
    x  = xs;
    mx = mxs;
    while(x <= xe)
    {
      //// debug:
      //rsAssert(x  >= 0 && x  < wi);
      //rsAssert(y  >= 0 && y  < hi);
      //rsAssert(mx >= 0 && mx < wm);
      //rsAssert(my >= 0 && my < hm);

      accumulate((*image)(x, y), TPix(color * (*mask)(mx, my)));
      x++;
      mx++;
    }
    y++;
    my++;
  }
}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::paintDotViaMask(TCor x, TCor y, TPix color)
{
  paintDotViaMask((int)round(x), (int)round(y), color);
  return;
    // preliminary - calls the non anti-aliased version

  int wi = image->getWidth();
  int hi = image->getHeight();
  int wm = mask->getWidth();
  int hm = mask->getHeight();

  int  xi = (int)floor(x);  // integer part of x
  int  yi = (int)floor(y);  // integer part of y
  TCor xf = x - xi;         // fractional part of x
  TCor yf = y - yi;         // fractional part of y

  // compute pixel coverages (these are the same as the weights for bilinear deinterpolation):
  TPix a, b, c, d;
  d = TPix(xf*yf);
  c = TPix(yf)-d;
  b = TPix(xf)-d;
  a = d+TPix(1-xf-yf);

  // write coordinates in target image:
  int xs = xi - wm/2;    // start x-coordinate
  yi     = yi - hm/2;    // start y coordinate
  int xe = xs + wm;      // end x coordinate
  int ye = yi + hm;      // end y coordinate

  // read coordinates in mask image:
  int mxs = 0;           // start x
  int my  = 0;           // start y

  // checks to not write beyond image bounds (maybe factor out, it's the same as in 
  // the non-anti-aliased version):
  if(xs < 0)
  {
    mxs = -xs;
    xs  = 0;
  }
  if(yi < 0)
  {
    my = -yi;
    yi =  0;
  }
  if(xe >= wi)
    xe = wi-1;
  if(ye >= hi)
    ye = hi-1;

  // paint corners:
  accumulate((*image)(xs, ys), TPix(a * color * (*mask)(0,    0)));     // top left
  accumulate((*image)(xe, ys), TPix(b * color * (*mask)(wm-1, 0)));     // top right
  accumulate((*image)(xs, ye), TPix(c * color * (*mask)(0,    hm-1)));  // bottom left
  accumulate((*image)(xe, ye), TPix(d * color * (*mask)(wm-1, hm-1)));  // bottom right






  // paint edges:


  // paint interior rectangle:





  // the actual painting loop over the pixels in target image and brush image:
  int mx;






}

template<class TPix, class TWgt, class TCor>
void ImagePainter<TPix, TWgt, TCor>::drawDottedLine(TCor x1, TCor y1, TCor x2, TCor y2, TPix color, 
  TCor density)
{
  TCor dx = x2-x1;
  TCor dy = y2-y1;
  TCor pixelDistance = sqrt(dx*dx + dy*dy);
  int  numDots = rsMax(1, (int)floor(density*pixelDistance));
  TPix scaledColor = (TPix) (color / (TPix)numDots); // maybe make this scaling optional
  TCor scaler = (TCor)(1.0 / numDots);
  TCor k;
  for(int i = 1; i <= numDots; i++)
  {
    k = scaler * i;  // == i / numDots
    paintDot(x1 + k*dx, y1 + k*dy, scaledColor);
  }
}

