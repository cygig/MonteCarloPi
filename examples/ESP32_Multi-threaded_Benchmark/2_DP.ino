void launchDP()
{
  // Set all flags false
  setAllFlags(dpDone, false);
  setAllFlags(dpFound, false);
  
  // Set all the decimal place accuracy of all elements of myPi_Multi to decP
  mpmSetDP();

  Serial.print(F("Estimating pi to "));
  Serial.print(decP);
  Serial.println(F(" decimal place(s)..."));

  for (byte i=0; i<threads; i++)
  {
    // Create a name for the task base on thread number
    char taskName[8]="piDP";
    char temp[4];
    itoa((int)i, temp, 10);//convert i to a cstring
    strcat(taskName, temp);

    // Store i globally
    tNo[i]=i; 
    
    // Assign threads to cores evenly
    cNo[i] = i%cores; 
    
    // Create thread
    xTaskCreatePinnedToCore(piDP, taskName, stackSize, (void*)&tNo[i], highPriority, NULL, cNo[i]);
  }
  // Creating a thread to check on the progress
  xTaskCreate(checkDP, "checkLoop", stackSize, NULL, lowPriority, NULL);
}



void piDP(void* myThread)
{
  byte thread = *((byte*)myThread);
  
  myPi_Multi[thread].startTimer();
  
  while( allFlagsFalse(dpFound) )
  // Keep doing as long as all of dpFound is false
  // i.e. stop if either or both threads found a satisfying solution
  {
    myPi_Multi[thread].runOnce();
    myPi_Multi[thread].computePi();

    if ( myPi_Multi[thread].isAccurate() )
    {
      myPi_Multi[thread].stopTimer();

      // Make sure only one core writes to pi
      xSemaphoreTake(mutexCS, portMAX_DELAY);
      pi=myPi_Multi[thread].getPi();
      xSemaphoreGive(mutexCS); // Release mutex
      
      dpFound[thread]=true;  
        
    }   
  }
  
  // Code lands here if the other thread is done but not this
  // We skip the calculations and stop the timer
  if (dpFound[thread]==false){ myPi_Multi[thread].stopTimer(); }

  // Mark as done regardless if the solution was found
  dpDone[thread]=true;
  
  vTaskDelete(NULL); // Delete itself
}



void checkDP(void *parameters)
{
  while(1) // Keep looping, we need this function to run non-stop
  {
    if ( allFlagsTrue(dpDone) )
    // Pass check if not all of dpDone is false
    // i.e. pass check if either or all threads found a satisfying solution
    {
      Serial.print(F("Pi is estimated to be "));
      Serial.print(pi, decP+1);
      Serial.print(F(", accurate to "));
      Serial.print(decP);
      Serial.println(F(" decimal place(s)."));

      byte fastest = mpmGetDPFastest();
      Serial.print(F("It took at best "));
      Serial.print( myPi_Multi[fastest].getTime() );
      Serial.print(F("ms and "));
      Serial.print( myPi_Multi[fastest].getSquares() );
      Serial.println(F(" loops.\n"));
      
      // Display which thread found pi
      for (byte i=0; i<threads; i++)
      {
        if (dpFound[i]==true && i!=fastest)
        {
          Serial.print(F("Thread "));
          Serial.print(i);
          Serial.println(F(" also found pi."));
        }
      }


      // Display individual results
      for (byte i=0; i<threads; i++)
      {
        if(i%col != 0){ Serial.print(F("\t|\t")); }// Separator 
        
        Serial.print(F("T"));
        Serial.print(i);
        Serial.print(F(" (C"));
        Serial.print(cNo[i]);
        Serial.print(F("): "));
        Serial.print( myPi_Multi[i].getTime() );
        Serial.print(F("ms, "));
        Serial.print( myPi_Multi[i].getSquares() );
        Serial.print(F(" loops, pi="));
        Serial.print(myPi_Multi[i].getPi(), decP+1);

        // println for last col in row or the last item
        if ( i%col == (col-1) || i==(threads-1)){ Serial.println(); } 
      }

      Serial.println("\nNote that rounding errors may occur.\n");

      // Reset for next round
      mpmReset();

      // Ask if user wants to restart, 
      // if so create the tasks for singlecore benchmarks
      restartQuery();

      // Delete itself after asking if the user wants to restart
      vTaskDelete(NULL);
    }
    
    // Wait a while if threads are not done yet
    // portTICK_PERIOD_MS is a constant by RTOS
    vTaskDelay(waitTime/portTICK_PERIOD_MS);
  }
}
