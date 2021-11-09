////////////////////////////
/* Flags Arrray Functions */
////////////////////////////

void setAllFlags(bool myFlag[], bool setValue)
{
  for (byte i=0; i<threads; i++){
    myFlag[i]=setValue;
  }
}


bool allFlagsTrue(bool myFlag[])
{
  for (byte i=0; i<threads; i++){
    if(myFlag[i]==false){ return false; }
  }
  return true;
}


bool allFlagsFalse(bool myFlag[])
{
  for (byte i=0; i<threads; i++){
    if(myFlag[i]==true){ return false; }
  }
  return true;
}



////////////////////////////////
/* myPi_Multi Array Functions */
////////////////////////////////

unsigned long mpmGetAllSquares()
{
  unsigned long result=0;
  for (MonteCarloPi MCP : myPi_Multi){
    result+=MCP.getSquares();
  }
  return result;
}


unsigned long mpmGetAllCircles()
{
  unsigned long result=0;
  for (MonteCarloPi MCP : myPi_Multi){
    result+=MCP.getCircles();
  }
  return result;
}


byte mpmGetLoopSlowest()
{
  byte slowest=0; // Use the first as a starting point to compare
  unsigned long slowestTime=myPi_Multi[slowest].getTime();
  unsigned long leastLoops=myPi_Multi[slowest].getSquares();

  for (byte i=0; i<threads; i++) // Do not start from i=1 since there might be 1 thread only
  {
    // Must be slower than the current thread or same speed but did lesser loops
    if ( ( myPi_Multi[i].getTime()>slowestTime ) || 
         ( myPi_Multi[i].getTime()==slowestTime && myPi_Multi[i].getSquares()<leastLoops ) )
    {
      slowest=i;
      slowestTime=myPi_Multi[i].getTime();
      leastLoops=myPi_Multi[i].getSquares();  
    }
  }
  return slowest;
}


// Find the fastest threads that has a solution with the most loops
byte mpmGetDPFastest()
{
  byte fastest=0; // Use the first as a starting point to compare
  unsigned long fastestTime = myPi_Multi[fastest].getTime();
  unsigned long mostLoops = myPi_Multi[fastest].getSquares();

  for (byte i=0; i<threads; i++) // Do not start from i=1 since there might be 1 thread only
  {
    if (dpFound[i]==true) // Must have solution first
    {
      // Must be either faster than the current thread or equal speed but does more loops
      if ( ( myPi_Multi[i].getTime()<fastestTime ) || 
           ( myPi_Multi[i].getTime()==fastestTime && myPi_Multi[i].getSquares()>mostLoops) )
      {
        fastest=i;
        fastestTime = myPi_Multi[i].getTime();
        mostLoops = myPi_Multi[i].getSquares();
      }
    }
  }
  return fastest;
}


void mpmReset()
{
  // Remember to use & to pass by reference to change the original values
  for (MonteCarloPi &MCP : myPi_Multi){
    MCP.reset();
    MCP.reseed();
  }

}


void mpmSetDP(){
  // Remember to use & to pass by reference to change the original values
  for (MonteCarloPi &MCP : myPi_Multi){
    MCP.setDP(decP);
  }
}



/////////////
/* Others */
////////////

void restartQuery()
{ 
  // Flush the Serial buffer, get rid of anything user input before this
  while (Serial.available()>0){
    char a = Serial.read();
  }

  Serial.print("Restart? Y/N: ");
  
  // Wait for user input, cannot leave empty in while loop else will trigger watchdog
  while (Serial.available()<=0){ vTaskDelay(waitTime/portTICK_PERIOD_MS); }
  char ans = Serial.read();
  Serial.println('\n');

  // If yes, create task to do single core benchmarks again
  if (ans == 'y' || ans == 'Y'){ 
    xTaskCreate(singleCoreBenchmark, "singleCoreBenchmark", stackSize, NULL, highPriority, NULL);
  }  
}
