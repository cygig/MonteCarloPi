/* MonteCarloPi by cygig v0.8.3
 * Single-core Benchmark Example
 * 
 * This example shows the basic of running a single-core benchmark.
 * 
 * This example should work on most Arduinos (e.g. Uno)
 */
 
#include <MonteCarloPi.h>

MonteCarloPi myPi;
double pi;
const unsigned long myLoop=500000; // Loop 500,000 times
const byte decP=5; // Accurate to 5 decimal places
char sChar;

void setup()
{
  Serial.begin(9600);

  // Wait for Serial to be ready
  // Works on AVR Arduino (Uno, Leonardo etc) but not ESP32
  // For ESP32, press reset after opening Serial Monitor
  while(!Serial){}

  Serial.println(F("MonteCarloPi by cygig\nPi is 3.14159265359\n"));

  // Needed for ESP32 or the results isnt consistent
  // Restarts the random numbers
  // Optional for most others
  myPi.reseed();

  do
  { 
    // Run benchmark
    benchmark();

    // Clear the Serial buffer
    while( Serial.available() ){
      char a=Serial.read(); 
    }

    // Ask user if they want to run the benchmark again
    Serial.print(F("Run again? Y/N: "));

    // Keep waiting as long as user does not input anything
    while(Serial.available()==0){}

    // Once the user input something, read it
    sChar=Serial.read();
    Serial.println('\n');
  }
  while (sChar=='Y' || sChar=='y'); // Repeat if user input 'Y' or 'y'
}



void loop(){ /* NOTHING */ }



void benchmark()
{
  Serial.print(F("Estimating pi in "));
  Serial.print(myLoop);
  Serial.println(F(" loops..."));
  myPi.startTimer(); // Built-in timer function
  pi = myPi.piLoop(myLoop);
  myPi.stopTimer(); // Remember to stop the timer

  Serial.print(F("Pi is estimated to be "));
  Serial.println(pi, decP+1);
  
  Serial.print(F("It took "));
  Serial.print( myPi.getTime() ); // Get how long the benchmark took in ms
  Serial.print(F("ms to get through "));
  Serial.print( myPi.getSquares() ); // No. of squares is the same as the total number of loops
  Serial.println(F(" loops.\n"));

  myPi.reset(); // Reset to run another test
  myPi.reseed(); // Reseed to make sure the "random" numbers are the same
  
  Serial.print(F("Estimating pi to "));
  Serial.print(decP);
  Serial.println(F(" decimal place(s)..."));
  myPi.startTimer();
  pi = myPi.piToDP(decP);
  myPi.stopTimer();

  Serial.print(F("Pi is estimated to be "));
  Serial.print(pi, decP+1);
  Serial.print(F(", accurate to "));
  Serial.print( myPi.getDP() );
  Serial.println(F(" decimal place(s)."));
  
  Serial.print(F("It took "));
  Serial.print( myPi.getTime() );
  Serial.print(F("ms and "));
  Serial.print( myPi.getSquares() );
  Serial.println(F(" loops.\nNote that rounding errors may occur.\n"));

  myPi.reset();
  myPi.reseed();
}
