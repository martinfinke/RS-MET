#ifndef ParticleSystem_h
#define ParticleSystem_h

#include "rapt/rapt.h"
using namespace RAPT;

/** This is just a skeleton/stub/idea for a particle system */


/** A list of physical constants. Physical simulations may keep a pointer to an object of that type
and outlying code may modify this object. ...hmm...not sure if that's a good idea - maybe each 
simulation should keep the constants it actually needs as members.

References:
 -https://physics.nist.gov/cuu/Constants/Table/allascii.txt

todo:
 -move to rapt library 
 -add more constants
*/

template<class T>
class rsPhysicalConstants
{

public:

  T boltzmann   = 1.38064852e-23;   // J K^-1, Boltzmann constant
  T gravitation = 6.67408e-11;      // m^3 kg^-1 s^-2, Newtonian constant of gravitation 
  T lightspeed  = 299792458;        // m s^-1, speed of light in vacuum
  T planck      = 6.626070040e-34;  // J s, Planck constant
  // ...more to come

};

/** Class for representing particles that can move in 3D space. Each particle is characterized at 
each time-instant by its current position and velocity (time varying) and its mass (constant). */

template<class T>
class rsParticle
{

public:

  rsVector3D<T> pos;  // position
  rsVector3D<T> vel;  // velocity
  T mass;
  T charge;

};


/**  */

template<class T>
class rsParticleSystem
{

public:

  //-----------------------------------------------------------------------------------------------
  // \name Construction/Destruction:

  /** Constructor. */
  rsParticleSystem(size_t numParticles);

  /** Destructor. */
  //~rsParticleSystem(); 

  //-----------------------------------------------------------------------------------------------
  // \name setup:

  void setNumParticles(int newNumParticles);

  /** Sets the constant used to scale all the gravitational forces. */
  void setGravitationalConstant(T newConstant) { cG = newConstant; }

  /** Sets the constant used to scale all the electric forces. */
  void setElectricConstant(T newConstant); { cE = newConstant; }

  /** Sets the constant used to scale all the magnetic forces. */
  void setMagneticConstant(T newConstant); { cM = newConstant; }

  //-----------------------------------------------------------------------------------------------
  // \name Inquiry:

  /*
  T getKineticEnergy();

  T getPotentialEnergy();

  T getTotalEnergy() { return getKineticEnergy() + getPotentialEnergy(); }

  T getTotalMomentum();

  T getTotalAngularMomentum();
  */

  //-----------------------------------------------------------------------------------------------
  // \name Processing:

  /** Computes the force that particle p1 experiences due to the presence of particle p2. */
  rsVector3D<T> getForceBetween(const rsParticle<T>& p1, const rsParticle<T>& p2);

  /** Computes the forces that act on each particle and stores them in our "forces" member. */
  void updateForces();

  /** Updates the particle velocities according to the computed forces and the particle masses, 
  using F = m*a -> a = F/m -> v += a. */
  void updateVelocities();

  /** Updates the particle positions according to the current velocities by advancing their 
  positions one step into direction of the respective velocity. */
  void updatePositions();

  /** Updates the state of the system. */
  void updateState()
  {
    updateForces();
    updateVelocities();
    updatePositions();
  }



  //-----------------------------------------------------------------------------------------------
  // \name Misc:

  void reset();

protected:



  std::vector<rsParticle<T>> particles;
  std::vector<rsVector3D<T>> forces;
  T stepSize = 0.01;
  T cG = 1, cE = 1, cM = 1;


};

#endif
