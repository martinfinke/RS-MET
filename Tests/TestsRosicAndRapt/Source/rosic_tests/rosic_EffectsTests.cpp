#include "rosic_EffectsTests.h"
using namespace rotes;

#include "rosic/rosic.h"
using namespace rosic;

bool rotes::testFastGeneralizedHadamardTransform()
{
  bool result = true;

  // 4-point FGWHT:
  double x4[4] = {4, -8, 12, -4};
  double y4[4];

  rosic::copyBuffer(x4, y4, 4);
  rosic::FeedbackDelayNetwork::fastGeneralizedHadamardTransform(y4, 4, 2);
  result &= y4[0] ==   4;
  result &= y4[1] ==  28;
  result &= y4[2] == -12;
  result &= y4[3] == - 4;

  rosic::copyBuffer(x4, y4, 4);
  rosic::FeedbackDelayNetwork::fastGeneralizedHadamardTransform(y4, 4, 2, 2, 3, 5, 7);
  result &= y4[0] ==  4;
  result &= y4[1] == 24;
  result &= y4[2] ==  4;
  result &= y4[3] == 44;


  // 8-point FWHT:
  double x8[8] = {1, 4, -2, 3, 0, 1, 4, -1};
  double y8[8];

  rosic::copyBuffer(x8, y8, 8);
  rosic::FeedbackDelayNetwork::fastGeneralizedHadamardTransform(y8, 8, 3);
  result &= y8[0] ==  10;
  result &= y8[1] == - 4;
  result &= y8[2] ==   2;
  result &= y8[3] == - 4;
  result &= y8[4] ==   2;
  result &= y8[5] == -12;
  result &= y8[6] ==   6;
  result &= y8[7] ==   8;

  rosic::copyBuffer(x8, y8, 8);
  rosic::FeedbackDelayNetwork::fastGeneralizedHadamardTransform(y8, 8, 3, 2, 3, 5, 7);
  result &= y8[0] ==   149;
  result &= y8[1] ==   357;
  result &= y8[2] ==   360;
  result &= y8[3] ==   862;
  result &= y8[4] ==   362;
  result &= y8[5] ==   866;
  result &= y8[6] ==   875;
  result &= y8[7] ==  2092;

  // forward/backward trafo - check if input is reconstructed:
  rosic::copyBuffer(x8, y8, 8);
  rosic::FeedbackDelayNetwork::fastGeneralizedHadamardTransform(       y8, 8, 3, 2, 3, 5, -7);
  rosic::FeedbackDelayNetwork::fastInverseGeneralizedHadamardTransform(y8, 8, 3, 2, 3, 5, -7);
  result &= fabs(rosic::maxError(x8, y8, 8)) < 1.e-15;

  return result;
}

bool rotes::testFeedbackDelayNetwork()
{
  bool result = true;

  //FeedbackDelayNetwork16 *fdn16 = new FeedbackDelayNetwork16;

  FeedbackDelayNetwork fdn;

  static const int N = 100000;
  //double hL[N], hR[N];
  double *hL = new double[N];
  double *hR = new double[N];


  rosic::fillWithImpulse(hL, N);
  rosic::fillWithImpulse(hR, N);
  for(int n = 0; n < N; n++)
    fdn.processFrame(&hL[n], &hR[n]);

  double t[N];
  rosic::fillWithIndex(t, N);
  //Plotter::plotData(N, t, hL, hR);
  //Plotter::plotData(N, t, hL);


    
  rosic::writeToStereoWaveFile("d:\\TmpData\\FDNImpulseResponse.wav", hL, hR, N, 44100, 16);



  delete[] hL;
  delete[] hR;




  return result;
}


