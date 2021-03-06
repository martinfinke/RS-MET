#ifndef RAPT_ELLIPSE_H_INCLUDED
#define RAPT_ELLIPSE_H_INCLUDED

/** This is a class for dealing with ellipses. Ellipses are a special case of conic sections which 
have the general form A*x^2 + B*x*y + C*y^2 + D*x + E*y + F = 0. For ellipses, we have 
B^2 - 4*A*C < 0. The ellipse is set up in terms of user parameters defining an area scale factor 
(with respect to the unit circle), an aspect ratio, determining the ratio between the long and the 
short axis, a rotation angle, and a center point. Coordinates of points on the ellipse can be 
obtained by passing in a value of angle parameter p between 0 and 2*PI to the member function
getPointOnEllipse(). */

template<class T>
class rsEllipse : public rsConicSection<T>
{

public:

  /** \name Construction/Destruction */

  /** Constructor. Without any parameters, it creates a unit circle. */
  rsEllipse(T areaScaler = 1, T aspectRatio = 1, T rotationAngle = 0, T centerX = 0, 
    T centerY = 0);


  /** \name Setup */

  /** Sets all the parameters that determine the ellipse's size, shape, orientation and position
  at once. */
  void setParameters(T newAreaScale = 1, T newAspectRatio = 1, T newRotationAngle = 0, 
    T newCenterX = 0, T newCenterY = 0);

  //void setScaleFactor(T newFactor);
  //void setAspectRatio(T newRatio);
  //void setRotationAngle(T newAngle); // in radians
  //void setCenterX(T newX);
  //void setCenterY(T newY);


  /** \name Inquiry */

  /** Computes a point on the ellipse corresponding to the given angle parameter. 
  Not yet tested. */
  void getPointOnEllipse(T angle, T* x, T* y) const;

  /** Returns the total area of the ellipse (not yet tested). */
  inline T getArea() const { return scale*T(PI); }

  /** Returns true if the point with given x,y coordinates is inside the ellipse. 
  todo: maybe move up to rsConicSection */
  inline bool isPointInside(T x, T y, T tol = 0) const { return this->evaluate(x, y) < -tol; }

  /** Returns true if the point with given x,y coordinates is outside the ellipse. */
  inline bool isPointOutside(T x, T y, T tol = 0) const { return this->evaluate(x, y) > tol; }
   // "this" needed for mac builds - why?


protected:

  /** \name Misc */

  /** Updates the parametric and implicit equation coefficients according to user parameters. */
  void updateCoeffs();

  T scale = 1, ratio = 1, angle = 0, centerX = 0, centerY = 0;  // user parameters
  T Axc, Axs, Ayc, Ays;                                         // parametric equation coeffs

};

#endif
