/* MonteCarloPi by cygig v0.8.3
 * https://github.com/cygig/MonteCarloPi
 */
 
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <limits.h>

#ifndef MONTECARLOPI_H
#define MONTECARLOPI_H


class MonteCarloPi
{  
  public:
    // Constructors
    MonteCarloPi(unsigned int mySeed);
    MonteCarloPi();


    // Computation
    void runOnce();
    double piLoop(unsigned long myLoop);
    double piToDP(byte myDP);
    void setDP(byte myDP);
    bool isAccurate();
    double computePi();
    static double computePi(unsigned long myCircles, unsigned long mySquares);

    // Get and Set functions
    unsigned long getSquares();
    unsigned long getCircles();
    double getPi();
    byte getDP();
    double getX();
    double getY();
    void setCircleSquare(unsigned long myCircles, unsigned long mySquares);
    void reset();
    void reseed();
    void setSeed(unsigned int mySeed);

    // Timing
    void startTimer();
    unsigned long stopTimer();
    void resetTimer();
    unsigned long getTime();

    // Pi Reference
    // Need to declare this static member outside
    static const double referencePi;

            
  private:
    // For Pi Computation
    byte dp;
    double xP, yP;
    unsigned long inSquare;
    unsigned long inCircle;
    double pi=0;

    // For DP calculations
    unsigned long multiplier; 
    unsigned long intRef;

    // Others
    unsigned int seed=69;//Default is 69
    unsigned long timer=0;
};


#endif
