#include "FilterExperiments.h"

//template <class T1, class T2>
//void rsScale(T1 *buffer, int length, T2 scaleFactor)
//{
//  for(int n = 0; n < length; n++)
//    buffer[n] *= scaleFactor;
//}

//-------------------------------------------------------------------------------------------------

void ladderResonanceManipulation()
{
  // We create two ladder filters with the same cutoff frequency, one with and one without 
  // resonance and plot the difference of the states of the two ladder filters when we pass
  // a sawtooth wave into them.

  // parameters:
  int   N   =  2000;      // number of samples
  float fs  = 44100;      // sample rate
  float fc  =  500;       // cutoff frequency
  float res =    0.99f;   // resonance
  float fIn =   80;       // input frequency

  // create and set up the filters:
  typedef RAPT::rsLadderFilter<float, float> LDR;  // for convenience

  LDR withReso;
  withReso.setSampleRate(fs);
  withReso.setCutoff(fc);
  withReso.setResonance(res);
  withReso.setMode(LDR::LP_24);

  LDR noReso;
  noReso.setSampleRate(fs);
  noReso.setCutoff(fc);
  noReso.setResonance(0);
  noReso.setMode(LDR::LP_24);

  // create time axis and input signal;
  vector<float> t(N), x(N);
  createTimeAxis(N, &t[0], fs);
  createWaveform(&x[0], N, 1, fIn, fs, 0.f, false);

  // filter input signal and record the difference between the state variables of teh two filters:
  vector<float> y0(N), y1(N), y2(N), y3(N), y4(N);
  vector<float> z0(N), z1(N), z2(N), z3(N), z4(N);
  vector<float> r0(N), r1(N), r2(N), r3(N), r4(N);
  vector<float> offset(N);
  float y[5], z[5];
  float dummy;
  for(int n = 0; n < N; n++)
  {
    // let the filter compute outputs (we don't actuualy need them):
    dummy = withReso.getSample(x[n]);
    dummy = noReso.getSample(  x[n]);

    // obtain the filter states:
    withReso.getState(y);
    noReso.getState(z);

    // record the states and state-differences:
    y0[n] = y[0];
    y1[n] = y[1];
    y2[n] = y[2];
    y3[n] = y[3];
    y4[n] = y[4];

    z0[n] = z[0];
    z1[n] = z[1];
    z2[n] = z[2];
    z3[n] = z[3];
    z4[n] = z[4];

    r0[n] = y[0] - z[0];
    r1[n] = y[1] - z[1];
    r2[n] = y[2] - z[2];
    r3[n] = y[3] - z[3];
    r4[n] = y[4] - z[4];

    // scale difference-states with appropriate factor to make the sinusoids have the same 
    // amplitude:
    r1[n] *= (float)SQRT2;      // s2^1, s2: sqrt(2)
    r2[n] *= 2;                 // s2^2
    r3[n] *= 2*(float)SQRT2;    // s2^3
    r4[n] *= 4;                 // s2^4

    // According to the model, this value would be zero because r0 and r4 are 180� out of phase. 
    // Any deviation from 0 is a shortcoming of the model and considered an offset to the idealized
    // value:
    offset[n] = r0[n] + r4[n];
  }

  // plot:
  GNUPlotter plt;
  plt.addDataArrays(N, &t[0], &x[0]);
  //plt.addDataArrays(N, &t[0], &y0[0], &y1[0], &y2[0], &y3[0], &y4[0]);
  //plt.addDataArrays(N, &t[0], &z0[0], &z1[0], &z2[0], &z3[0], &z4[0]);
  //plt.addDataArrays(N, &t[0], &r0[0], &r1[0], &r2[0], &r3[0], &r4[0]);
  plt.addDataArrays(N, &t[0], &r0[0], &r4[0]); // should be 180� out of phase
  plt.addDataArrays(N, &t[0], &offset[0]);
  //plt.addDataArrays(N, &t[0], &y0[0], &z0[0], &r0[0]);
  plt.plot();

  // Observations:
  // The states of the imaginary "pure-resonance" difference filters (resonant - nonResonant)
  // represent (decaying) sinusoids, where each of the successive states has the same sinusoid
  // multiplied by 1/sqrt(2) and phase-shifted by -45 degrees with respect to the stage before. 
  // We assume that at any time instant the resonance waveform has instantaneous amplitude a and 
  // phase p. We must then have:
  //
  // r4 = a             * sin(p)
  // r3 = a * sqrt(2)^1 * sin(p + 1*pi/4)
  // r2 = a * sqrt(2)^2 * sin(p + 2*pi/4) = a * 2 * sin(p + pi/2)
  // r1 = a * sqrt(2)^3 * sin(p + 3*pi/4)
  // r0 = a * sqrt(2)^4 * sin(p + 4*pi/4) = a * 4 * sin(p + pi) 
  //
  // That means, given the states r4,r2 we should be able to figure out the instantaneous amplitude
  // a and phase p. We just divide the state r2 by two and together with r4 we can take it as the 
  // sine and cosine part of our sine from which amplitude and phase can be computed. Having a and
  // p, we can do whatever we want to them (for example sync the phase to an input signal) and then 
  // compute our new states r0,...,r4 using the formulas above. Having done that, we may update the 
  // states of the resonant filter according to y0 = z0 + r0, ..., y4 = z4 + r4.  This should give 
  // us a ladder filter in which we can freely mess with the instantaneous phase of the resonance 
  // without needing to post-process the resonance signal.
  // hmm...well, all of this holds only approximately. Maybe we should estimate the sine parameters
  // not from r2,r4 but from r3,r4 (so we use the 2 most filtered outputs) and maybe we should 
  // compute the difference between the actual states and the idealized states according to the 
  // model and add this difference back after modification of the states, such that when we don't
  // apply any modification, we really don't apply any modification (when recomuting the states
  // according to the formulas)

  // Note: it works only when the compensation gain is applied at the input and when the filter
  // is linear.
}


/** Moving average filter for non-uniformly spaced samples. N: num samples, x: abscissa-values
(mostly time), y: ordinate values, avg: average - the output (must be distinct from y), width: 
length/width/range of the support of the filter, weightFunc: normalized weighting function, 
should have a support in the range -1..+1, this is the filter kernel as continuous function. 
\todo: maybe let the user pass a functor for the weighting function isntead of a function 
pointer (maybe it's possible to write a functor-wrapper for ordinary functions with automatic
type casting such that ordinary functions can still be passed as usual?) */
template<class Tx, class Ty>
void movingAverage(int N, Tx* x, Ty* y, Ty* avg, Tx width, Ty (*weightFunc)(Tx))
{
  Tx w2  = 0.5f * width;                       // half width
  Tx w2r = 1 / w2;                             // reciprocal of half width
  Tx dist;                                     // distance
  int k;                                       // inner loop index
  for(int n = 0; n < N; n++){                  // outer loop over all points
    Ty wgt = weightFunc(0);                    // weight
    Ty sw  = wgt;                              // sum of weights
    Ty swv = wgt * y[n];                       // sum of weighted values
    k = n-1;                                   // immediate left neighbour
    while(k >= 0 && (dist = x[n]-x[k]) <= w2){ // left side loop
      wgt  = weightFunc(dist * w2r);           // compute weight for distance
      sw  += wgt;                              // accumulate weight sum
      swv += wgt * y[k];                       // accumulate weighted values
      k--; }                                   // jump to next neighbour
    k = n+1;
    while(k < N  && (dist = x[k]-x[n]) <= w2){ // right side loop
      wgt  = weightFunc(dist * w2r);
      sw  += wgt;
      swv += wgt * y[k];
      k++; }
    avg[n] = swv / sw; }
}
// Weighting functions for nonuniform MA. Maybe implement more, see here:
// https://en.wikipedia.org/wiki/Kernel_(statistics)#Kernel_functions_in_common_use
// actually, for use in movingAverage, the checks against > 1 are superfluous bcs the loops there
// ensure already that x <= 1. maybe we can wrap all this stuff into a class 
// NonUniformMovingAverage. We could have functions like setNominalWidth (the width used for 
// uniform weighting), setWeightingFunction(T (*func)(T), T area) and for functions with unknown
// area just setWeightingFunction(T (*func)(T)) which finds the area by numerical integration
// and then calls setWeightingFunction(T (*func)(T), T area) with the numerically computed area
// ...finally an opportunity to implement and use numerical integration algorithms :-)
// other functions to try: (1-x^a)/(1+x^a), a = 1, 1/2, 1/3, 2, 
float uniform(float x)       // rename to uniform, maybe scale by 0.5
{
  if(fabs(x) > 1)
    return 0;
  return 1;
}
float triangular(float x)      // half-area: 0.5 (integral from 0 to 1)
{
  x = fabs(x);
  if(x > 1)
    return 0;
  return 1 - x;
}
float rationalTent(float x)   // half-area: log(4)-1
{
  x = fabs(x);
  if(x > 1)
    return 0;
  return (1-x) / (1+x);
}
float parabolic(float x)      // half-area: 2/3  (Epanechikov)
{
  if(fabs(x) > 1)
    return 0;
  return 1 - x*x;
}
float cubicBell(float x)
{
  return rsPositiveBellFunctions<float>::cubic(fabs(x));
}
float quinticBell(float x)
{
  return rsPositiveBellFunctions<float>::quintic(fabs(x));
}
float hepticBell(float x)
{
  return rsPositiveBellFunctions<float>::heptic(fabs(x));
}
void nonUniformMovingAverage()
{
  // user parameters:
  static const int N = 200;  // number of samples
  float minDist = 1.f;       // minimum distance between successive samples
  float maxDist = 10.f;      // maximum ...
  float minY    = -10.f;     // minimum y value
  float maxY    = +10.f;     // maximum
  float width   =  30.f;     // filter support width for box, others are scaled by their area
  int   seed    = 0;

  // create input data:
  float x[N], y[N];
  float t = 0.f; // time
  float dt;      // time delta between samples
  x[0] = t;
  y[0] = (float)round(rsRandomUniform(minY, maxY, seed)); 
  for(int n = 1; n < N; n++){
    //dt = (float)round(rsRandomUniform(minDist, maxDist));
    dt = (float)rsRandomUniform(minDist, maxDist);
    t += dt;
    x[n] = t;
    y[n] = (float)rsRandomUniform(minY, maxY); 
    //y[n] = (float)round(rsRandomUniform(minY, maxY)); 
  }

  // create filtered versions:
  float a = 1.f / float(log(4.f)-1); // reciprocal of area under rationalTent weighting function
                                     // ...the definite integral from 0 to 1
  float y0[N], y1[N], y2[N], y3[N], y4[N],
    yR[N],  // rational
    yP[N];  // parabolic

  // 0,1,2,3,4 is the smoothness of the weight function

  movingAverage(N, x, y, y0,  width,      uniform);
  movingAverage(N, x, y, y1,  width*2,    triangular);
  movingAverage(N, x, y, yR,  width*a,    rationalTent); 
  movingAverage(N, x, y, yP,  width*1.5f, parabolic);
  movingAverage(N, x, y, y2,  width*2,    cubicBell);
  movingAverage(N, x, y, y3,  width*2,    quinticBell);
  movingAverage(N, x, y, y4,  width*2,    hepticBell);

  // For all weighting functions other than the box, we scale the support width by factor to
  // equal to the reciprocal of the half of the area under the function to make the plots more 
  // easily comparable. 

  // plot:
  GNUPlotter plt;
  //plt.addDataArrays(N, x, y, y0, y1, y2, y3, y4);
  //plt.addDataArrays(N, x, y0, y1, yR);
  //plt.addDataArrays(N, x, y0, y1, y2, y3, y4, yR);
  //plt.addDataArrays(N, x, y0, y2, y4);
  plt.addDataArrays(N, x, y, y0, y1, yR, yP);
  plt.plot();

  // Observations: The rationalTent weighting seems best in terms of smoothing. It shows the 
  // smallest deviations from the average, i.e. the curve is the most "inside" of all (stays
  // closer to the mean than the others). 
  // ToDo: maybe try to find even better weighting functions. Maybe the smoothness is related to 
  // the area? smaller area -> more width widening -> better smoothing? maybe because the jitter
  // gets averaged out better?
}

void smoothingFilterOrders()
{
  // We plot the step responses of the rsSmoothingFilter for various orders.

  static const int numOrders = 4;   // number of filters with different orders
  bool expSpacing = false;          // if true, orders are 1,2,4,8,.. else 1,2,3,4,..
  int orderIncrement = 2;           // or 1,3,5,.. or 1,4,7,...
  static const int N = 300;         // number of samples
  float fs  = 1.0f;                  // sample rate
  float tau = 101.0f;                  // time constant

  // create and set up the smoother:
  rsSmoothingFilterFF smoother;
  //rsSmoothingFilterDD smoother;
  smoother.setTimeConstantAndSampleRate(tau, fs);
  //smoother.setNumSamplesToReachHalf(tau*fs);
  //smoother.setShape(rsSmoothingFilterFF::FAST_ATTACK);
  smoother.setShapeParameter(0.5f);

  // compute step responses:
  int order = 1;
  float y[numOrders][N];
  for(int i = 0; i < numOrders; i++)
  {
    smoother.setOrder(order);
    if(expSpacing) // update order for next iteration
      order *= 2;
    else
      order += orderIncrement;

    smoother.reset();
    y[i][0] = smoother.getSample(1.f);
    for(int n = 1; n < N; n++)
    {
      //y[i][n] = smoother.getSample(0.f);   // impulse response
      y[i][n] = smoother.getSample(1.f); // step response
    }
  }

  // plot:
  float t[N];
  createTimeAxis(N, t, fs);
  GNUPlotter plt;
  for(int i = 0; i < numOrders; i++)
    plt.addDataArrays(N, t, y[i]); 
  plt.plot();

  // Observations:
  // The step responses of the different orders are comparable in terms of overall transition time.
  // However, they do not meet in a common point. From a user's perspective, it would perhaps be
  // most intuitive, if he could just set up the time-instant, where the step-response goes through
  // 0.5. No matter what the order is, the time instant where it passes through 0.5 should remain 
  // fixed. To achieve that, i think, we need a closed form expression for the step-response and 
  // then set that expression equal to 0.5 and solve for the time-constant. If it's too hard to 
  // find such a formula, we could also create a table by just reading off the time-instants where
  // the various step responses go through 0.5 and then use that value as divider.

  // ToDo:
  // Measure the sample instants where the the step responses pass through 0.5 for some normalized
  // filter setting and create tables from that. These tables will eventually be used for scaling. 
  // It should be a 2D table, 1st dimension is the order (say 1..32) and 2nd dimension is the value
  // of the shape parameter (maybe from 0 to 4 in 32 steps?) the 2nd dimenstion can use linear
  // interpolation, the 1st dimension doesn't need that because inputs are integers anyway.
  // The values of the table should be overall scalers for all the time constants.


  // try higher orders - like 100 - see what kind of shaped is approached for order -> inf
}

void smoothingFilterTransitionTimes()
{
  // We plot the step response for smoothing filters of a given order (and shape/asymmetry) for
  // different transition time constants. What we should see is time-stretched/compressed versions
  // of the same response. But the nonworking tuning tables suggest that somthing might be wrong
  // with that assumption, so we check.

  // user parameters:
  static const int N = 400;           // number of samples
  int order = 5;                      // order of the filters
  float asymmetry = 1.f;              // asymmetry parameter
  static const int numFilters = 5;    // number of transition times
  float transitionTimes[numFilters] = { 51.f, 101.f, 151.f, 200.1f, 251.f }; // transition times in samples


  // create and set up the smoother:                                                             
  rsSmoothingFilterFF smoother;
  smoother.setShapeParameter(asymmetry);
  smoother.setOrder(order);

  // create the step responses:
  float y[numFilters][N];
  for(int i = 0; i < numFilters; i++)
  {
    smoother.setNumSamplesToReachHalf(transitionTimes[i]);
    smoother.reset();
    for(int n = 0; n < N; n++)
      y[i][n] = smoother.getSample(1.f);
  }

  // plot:
  GNUPlotter plt;
  for(int i = 0; i < numFilters; i++)
    plt.addDataArrays(N, y[i]); 
  plt.plot();

  // Observations:
  // The intuitive assumption that scaling all the time constants by the same factor has the effect
  // of stretching/compressing the step-response by that amount turns out to be false. It seems to
  // be true for 1st order filters but for higher orders, especially when there's asymmetry, the 
  // actually observed step repsonses deviate from that simple rule more and more. 
  // Why is that? Is it because the design formula uses impulse-invariant transform and should use
  // step-invariant? ...or maybe even bilinear?

  // see here: http://web.cecs.pdx.edu/~tymerski/ece452/Chapter4.pdf
  // http://homes.esat.kuleuven.be/~maapc/Sofia/slides_chapter8.pdf
  // http://dspcan.homestead.com/files/IIRFilt/zfiltsii.htm
}
