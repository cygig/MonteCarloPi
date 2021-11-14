// MonteCarloPi by cygig v0.8.3
// Loops Functions  



// Function to prepare and launch multi-core Loop benchmark
void launchLoop()
{
  // Initialise variables
  setAllFlags(loopTimerStopped, false);
  loopTicket=0;
  loopDone=false;
  
  Serial.println(F("[ Dual-Core Benchmark ]\n"));
  
  Serial.print(F("Estimating pi for "));
  Serial.print(myLoop);
  Serial.println(F(" loops..."));

  // Creating many threads for multi-core loop benchmarks
  
  for (byte i=0; i<threads; i++)
  {
    // Create a name for the task base on thread number
    char taskName[10]="piLoop";
    char temp[4];
    itoa((int)i, temp, 10);//convert i to a cstring
    strcat(taskName, temp);

    // We're basically storing the value of i in a global variable,
    // which is a byte array where the content of the element is a number the same as the position.
    // This is because this loop will likely end before the test and i is passed by reference
    tNo[i]=i;

    // Assign threads to cores evenly, no need to store globally since pass by value
    cNo[i] =  i%cores;
    
    // Parameters: Function, name, stack size (bytes), parameters, priority, handle, pin to core
    // We're passing the pointer of the byte parameter for the task using &, 
    // then casting it to void pointer (void*) as that is the only acceptable datatype 
    xTaskCreatePinnedToCore(piLoop, taskName, stackSize, (void*)&tNo[i], highPriority, NULL, cNo[i]);
  }
  
  // Creating a thread to check on the progress
  xTaskCreate(checkLoop, "checkLoop", stackSize, NULL, lowPriority, NULL);
}



// Note to self: Use a atomic pointer to allocate space before doing.
void piLoop(void* myThread) // 1 func
{
  // We cast the void pointer back to byte pointer (byte*)
  // We then get the value using * and store another variable
  byte thread = *((byte*)myThread);

  myPi_Multi[thread].startTimer();

  unsigned long myTicket;
  
  // Keep doing as long as loopDone is flagged false
  while(!loopDone) // 2 while
  { 
    // Issue ticket and increment by ticketChunk
    // Use mutex to ensure this only happen one at a time
    if (xSemaphoreTake(mutexCS, portMAX_DELAY) == pdTRUE) // 3 if
    {
      myTicket=loopTicket;
      loopTicket+=ticketChunk;
      xSemaphoreGive(mutexCS);

      // Compute for each runs in the chunk
      for (unsigned int i=0; i< ticketChunk; i++) // 4 for
      { 
        // Only compute if the ticket no is less than myLoop
        if (myTicket<myLoop){
          myPi_Multi[thread].runOnce();
          myTicket++;
        } 
        
        else{  // 5 else
          loopDone=true; // flag to other threads loops ended
          break; // break from for loop
        } // 5 else
        
      } //4 for
      
    } // 3 if
    
  } // 2 while

  myPi_Multi[thread].stopTimer();
  
  loopTimerStopped[thread]=true; // Mark the timing as taken down

  vTaskDelete(NULL); // Delete itself

} // 1 func



// This function checks on the progress
void checkLoop(void* parameter)
{
  while(1) // Keep looping, we need this function to run non-stop
  {
    // All threads are done, including stopping timer
    if( allFlagsTrue(loopTimerStopped) ) 
    { 
      // We need to take into account the time taken to compute the accumulated values
      // This is usually 0ms as it is too fast
      unsigned long computeTime=millis();

      // Add up the total number points in circle and square
      unsigned long totalCircles = mpmGetAllCircles();
      unsigned long totalSquares = mpmGetAllSquares();
      
      // Use the static function to compute the results
      pi = MonteCarloPi::computePi(totalCircles, totalSquares);

      // Stop the timer
      computeTime = millis()-computeTime; 

      // We get the slowest time among the two threads
      // and all the time needed to compute pi to it
      byte slowest = mpmGetLoopSlowest();
      unsigned long totalTime = myPi_Multi[slowest].getTime() + computeTime;
      
      Serial.print(F("Pi is estimated to be "));
      Serial.println(pi, decP+1);
      
      Serial.print(F("It took "));
      Serial.print(totalTime);
      Serial.print(F("ms to get through "));
      Serial.print(totalSquares);
      Serial.println(F(" loops.\n"));

      
      // Display individual thread results
      for (byte i=0; i<threads; i++)
      {
         // print | for every col except first in row
        if (i%col != 0){ Serial.print(F("\t|\t")); }
        
        Serial.print(F("T"));
        Serial.print(i);
        Serial.print(F(" (C"));
        Serial.print(cNo[i]);
        Serial.print(F("): "));
        Serial.print( myPi_Multi[i].getTime() );
        Serial.print(F("ms, "));
        Serial.print( myPi_Multi[i].getSquares() );
        Serial.print(F(" loops"));
        
        // println for last col in row
        // i%col will result in 0 to col-1
        if ( i%col == (col-1) ){ Serial.println(); } 
      }

      if (threads%col != 0){ Serial.print(F("\t|\t" )); } // Not all rows filled, print |
      Serial.print(F("Compute Pi: "));
      Serial.print(computeTime);
      Serial.println(F("ms\n"));


      // Reset for next round
      mpmReset();

      // Create tasks for multicore DP benchmarks
      launchDP();

      // Delete itself after launching DP Benchmark
      vTaskDelete(NULL);
    }

    // Wait a while if threads are not done yet
    // portTICK_PERIOD_MS is a constant by RTOS
    vTaskDelay(waitTime/portTICK_PERIOD_MS);
  }
}
