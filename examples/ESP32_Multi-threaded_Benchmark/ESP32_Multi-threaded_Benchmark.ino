/* MonteCarloPi by cygig v0.8.3
 * ESP32 Dual-core Benchmark Example
 * 
 * This example is meant for Expressif *ESP32* with two cores.
 * It uses both cores to estimate the value of pi.
 * Does not work for AVR Arduinos like Uno.
 */

#include <MonteCarloPi.h>


/* THREADS AND CORES */
const byte cores=2; //Should always be 2 for ESP32
const byte threads=2; 
byte tNo[threads]; // Global array to keep track of thread no
byte cNo[threads]; // Track the core a thread is pinned to


/* THREAD PARAMETERS */
const int stackSize=1000; // Allocate 1000 bytes per threads, estimated value
const int waitTime=500;
const byte lowPriority=1;
const byte highPriority=2;


/* TESTING PARAMETERS */
const unsigned long myLoop=500000; // Loop 500,000 times
const byte decP=5; // Accurate to 5 decimal places


/* MUTEXES */
// Mutexes to prevent the cores from writing to the same memory
// or serial monitor at the same time
SemaphoreHandle_t mutexCS;
SemaphoreHandle_t mutexPrint;


/* MONTECARLOPI */
MonteCarloPi myPi_Single; // One instant for single-core benchmark test
MonteCarloPi myPi_Multi[threads]; // One instance for each of the dual-core benchmark tests
double pi;


/* FLAGS */
// Flags for multi-core benchmark tests stored in arrays
bool loopTimerStopped[threads];
bool loopDone;
bool dpFound[threads];
bool dpDone[threads];


/* LOOP TICKETS */
// High ticketChunk, less resistant to imbalance cores
// Low thicketChunk, slower
unsigned long loopTicket; // Tracks the distribution of threads 
const unsigned int ticketChunk=1000; 


/* DISPLAY */
// How many individual result to display in one row
// Cannot be 0
const byte col=2;



void setup()
{
  Serial.begin(9600);

  // Press Enter in serial monitor to continue
  // if(Serial.available) does not seem available for ESP32
  while(Serial.available()<=0){}

  mutexCS = xSemaphoreCreateMutex();
  mutexPrint = xSemaphoreCreateMutex();

  Serial.print(F("MonteCarloPi by cygig\nPi is "));
  Serial.println(MonteCarloPi::referencePi, 11); // 3.14159265359
  Serial.println();
  
  // Creating the tasks for singlecore benchmarks
  xTaskCreate(
    singleCoreBenchmark,     // Function that should be called
    "singleCoreBenchmark",   // Name of the task (for debugging)
    stackSize,               // Stack size (bytes)
    NULL,                    // Parameter to pass
    highPriority,            // Task priority
    NULL                     // Task handle
  );

}


void loop(){/*NOTHING*/}


void singleCoreBenchmark(void* parameters)
{
  Serial.println(F("[ Single Core Benchmark ]\n"));

  // Single Core Loop
  Serial.print(F("Estimating pi in "));
  Serial.print(myLoop);
  Serial.println(F(" loops..."));
  
  myPi_Single.startTimer(); // Built in timer
  pi=myPi_Single.piLoop(myLoop);
  myPi_Single.stopTimer(); // Remember to stop the timer
  
  Serial.print(F("Pi is estimated to be "));
  Serial.println(pi, decP+1);
    
  Serial.print(F("It took "));
  Serial.print(myPi_Single.getTime());
  Serial.print(F("ms to get through "));
  Serial.print(myPi_Single.getSquares()); // No. of squares is same as the number of loops
  Serial.println(F(" loops.\n"));

  myPi_Single.reset(); // Reset all values for next test
  myPi_Single.reseed(); // Reseed to restart the random number list

  // Single Core DP
  Serial.print(F("Estimating pi to "));
  Serial.print(decP);
  Serial.println(F(" decimal place(s)..."));
  
  myPi_Single.startTimer();
  pi=myPi_Single.piToDP(decP);
  myPi_Single.stopTimer();

  Serial.print(F("Pi is estimated to be "));
  Serial.print(pi, decP+1);
  Serial.print(F(", accurate to "));
  Serial.print(myPi_Single.getDP());
  Serial.println(F(" decimal place(s)."));
  
  Serial.print(F("It took "));
  Serial.print(myPi_Single.getTime());
  Serial.print(F("ms and "));
  Serial.print(myPi_Single.getSquares());
  Serial.println(F(" loops.\nNote that rounding errors may occur.\n"));

  myPi_Single.reset();
  myPi_Single.reseed();
  
  // Create tasks for multicore loop benchmarks
  launchLoop();

  // Delete itself after launching Loop Benchmark
  vTaskDelete(NULL);
}
