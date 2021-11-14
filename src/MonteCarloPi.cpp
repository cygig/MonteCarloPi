/* MonteCarloPi by cygig v0.8.3
 * https://github.com/cygig/MonteCarloPi
 */

#include "MonteCarloPi.h"

//===================//
/* Static Memembers */
//==================//

const double MonteCarloPi::referencePi = 3.14159265359;



//===============//
/* Constructors */
//==============//

MonteCarloPi::MonteCarloPi(){
  srand(seed);
}

MonteCarloPi::MonteCarloPi(unsigned int mySeed){
  seed=mySeed;
  srand(seed);
}



//===============//
/* Computations */
//==============//

void MonteCarloPi::runOnce()
{
    // We need to add 1 as it will generate up to randomMax -1.
    // We need the resulting number to be between 0 and 1.
    xP = (double)rand() / (double)RAND_MAX;
    yP = (double)rand() / (double)RAND_MAX;

    // If x^2+y^2 <= 1, it is inside the circle
    if ((xP*xP + yP*yP)<=1){ inCircle++; }
    
    inSquare++;
}



double MonteCarloPi::piLoop(unsigned long myLoop)
{
    for (unsigned long i=0; i<myLoop; i++){
        runOnce();
    }

    return computePi();

}



double MonteCarloPi::piToDP(byte myDP)
{
    setDP(myDP);
    while(!isAccurate())
    {
        runOnce();
        computePi();
    }
    
    return pi;
}



void MonteCarloPi::setDP(byte myDP)
{
  dp=myDP;
  multiplier=1; //accurate to 0 dp
  
  for (int i=0; i<dp; i++){ // increase accuracy
    multiplier*=10.0;
  }
    
  intRef = (unsigned long)floor(referencePi*(double)multiplier);

}



bool MonteCarloPi::isAccurate()
{
    if((unsigned long)floor(pi*(double)multiplier) == intRef) {
      /*// Sanity Check
      Serial.print(F("pi is "));
      Serial.print(pi,10);
      Serial.print(F(" | intRef is "));
      Serial.print(intRef);
      Serial.print(F(" | piRef is "));
      Serial.println((unsigned long)floor(pi*(double)multiplier));
      */
      return true;
    } 
    
    else { 
        return false; 
    }
}


double MonteCarloPi::computePi()
{
    pi = computePi(inCircle, inSquare);
    return pi;
}



double MonteCarloPi::computePi(unsigned long myCircles, unsigned long mySquares){
    double piAns = 4.0 * (double)myCircles / (double)mySquares;
    return  piAns;

}



//======================//
/* GET & SET FUNCTIONS */
//======================//

unsigned long MonteCarloPi::getSquares(){
    return inSquare;
}

unsigned long MonteCarloPi::getCircles(){
    return inCircle;
}

double MonteCarloPi::getPi(){
    return pi;
}

byte MonteCarloPi::getDP(){
    return dp;
}

double MonteCarloPi::getX(){
  return xP;
}

double MonteCarloPi::getY(){
  return yP;
}

void MonteCarloPi::setCircleSquare(unsigned long myCircles, unsigned long mySquares){
  inCircle = myCircles;
  inSquare = mySquares;
}

void MonteCarloPi::reseed(){
  srand(seed);
}

void MonteCarloPi::setSeed(unsigned int mySeed){
  seed=mySeed;
  srand(seed);
}

void MonteCarloPi::reset(){
  pi=0;
  xP=0;
  yP=0;
  inSquare=0;
  inCircle=0;
  timer=0;
}



//=================//
/* TIMER FUNCTIONS */
//=================//

void MonteCarloPi::startTimer(){
  timer=millis();
}

unsigned long MonteCarloPi::stopTimer(){
  timer = millis()-timer;
  return timer;
}

void MonteCarloPi::resetTimer(){
  timer=0;
}

unsigned long MonteCarloPi::getTime(){
  return timer;
}
