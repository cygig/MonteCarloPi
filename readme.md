# MonteCarloPi
MonteCarloPi is a library to benchmark Arduinos by estimating the value of pi. It uses the Monte Carlo method to estimate pi, and it works with both single core Arduino like the UNO as well as multi-core ones like ESP32.

Do note this method only estimates pi and will require a huge amount of good quality random numbers to give a good estimate. It is far from the most efficient method to estimate or calculate pi, but works well enough for the purpose of benchmarking.

![image](extras/montecarlopi.jpg)


# Contents
- [Updates](#updates)
- [How does It Work?](#how-does-it-work)
- [Circles, Squares, Quadrants and Quarters](#circles-squares-quadrants-and-quarters)
- [Benchmark Tests](#benchmark-tests)
  - [Loop](#Loop)
  - [Accurate to Decimal Places](#accurate-to-decimal-places)
- [Multi-threaded Benmchmark](#multi-threaded-benchmark)
  - [Loop Benchmark](#loop-benchmark)
  - [Accurate to Decimal Places Benchmark](#accurate-to-decimal-places-benchmark)
- [Default Values](#default-values)
- [Public Functions](#public-functions)

# How Does It Work?

# Updates
- v0.8.3
  - First upload

# How Does It Work?
Imagine inscribing a circle in a square. We then place dots (of the same size) randomly all over the square. If we can place infinite dots inside the square, we will be describing the area of the square.

We are interested in the number of dots in the square and inside the circle. Essentially, we are interested in finding out the ratio of the area of square to that of the circle.

![image](extras/Monte%20Carlo%20Pi_dots.svg)

Now we divide the square and circle into four parts, placing the centre of the square and circle at `0,0`, this makes it easy for two reasons:
- The radius of the circle is now 1, making it easy for calculations
- It is possible to determine if a dot landed within or on the circle using Pythagoras' theorem

We'll be interested in the top right quadrant. The area of the square quarter will conveniently be `1×1=1`. The area of a whole circle is `pi×1×1=pi`, thus the area for the circle quadrant is `pi/4`. 

![image](extras/Monte%20Carlo%20Pi_quads.svg)

With the theoretical area calculated, we want the numeric ratio between the area of circle quadrant and the area of square quarter. Counting the number of dots in those areas will give us an approximation.

```
Let c be the no. of dots in circle quadrant.  
Let s be the No. of dots in square quarter.  

c/s = (pi/4)/ 1
c/s = pi/4
pi = 4(c/s)
```

![image](extras/Monte%20Carlo%20Pi_area.svg)

Instead of spraying dots, we can instead randomly generate points where their x and y coordinates are between 0 and 1, inclusive. The total number of points generated is also the number of points inside the square quarter, since all points will land within.

How do we know if these points fall within the circle quadrant then? We can make use of Pythagoras' theorem, where in a right angle triangle with sides _a_, _b_ and _c_, where _c_ is the longest side:

```
c² = a² + b²
```

Given a point (x, y), a right angle triangle can be drawn with side _a_ being (0, 0) to (x, 0), side _b_ (x, 0) to (x, y) and side _c_ (x, y) to (0 ,0).

![image](extras/Monte%20Carlo%20Pi_triangle.svg)

Since the radius of the circle is 1, side _c_ cannot exceed 1 if (x, y) is within or on the circle. Imagine a pair of compasses can only be opened to 10cm wide, we will never be able to draw anything larger than a 10cm radius circle.


![image](extras/Monte%20Carlo%20Pi_points.svg)



So the maximum length of _c_ is 1. The maximum length of c<sup>2</sup>_ is 1<sup>2</sup>=1. For every randomly generated point (x, y), we need to check if:

```
x² + y² <= 1
```

If so, it is within the circle quadrant.

With all these information, we can estimate pi. The more random points we can generate, the more accurate the answer will be. 

# Circles, Squares, Quadrants and Quarters
While this document differentiate between a whole circle, whole square, circle quadrant and square quarter for the sake of explanation, it is not the case in the code. In the code, circle refers to the circle quadrant while square refers to the square quarter, since those are the areas of interest.

# Benchmark Tests
There are two main ways to benchmark an Arduino using this library.

## Loop
Looping is the most straightforward among the two. The library will

1. Generate a random point
2. Check if it is within the circle quadrant
3. Update the number of points
4. Repeat for the number of desired loops
5. Estimate pi from the accumulated data


We can then measure the time taken as a benchmark.

## Accurate to Decimal Places
In this mode, the library will

1. Generate a random point
2. Check if it is within the circle quadrant
3. Update the number of points
4. Estimate pi from the accumulated data
5. If the estimate is correct to the desired number of decimal points, it stops, else it goes on and on

As the random points are, well, random, there is a good chance that a handful of points will coincidentally form an accurate estimate of pi.

For example, if we are targeting an accuracy of 3 decimal places (3.141, dropping all the rest without rounding), we might get 3.141XXXXX after just 100 random points. The next time we repeat the test, it might take 100,000.

To mitigate such issue, we can use the `reseed()` function before executing this test. This will ensure the "random" numbers are always the same to level the playing field. 

The random numbers generated by Arduinos are usually not really random, but come from pre-compiled lists of numbers. Each seed will produce one list of numbers and initiating the seed will restart the same list. The same seed should always produce the same list of numbers.

However, this lists can be still be different across different devices. The 10th seed for a Arduino Uno may be different from the 10th seed for the Expressif ESP32.

Despite the shortfall, this can be still useful for benchmarking the same device (single core vs multi-core, with heatsink vs without etc.) While coincidences do happen, generally the faster the machine, the faster it takes to reach pi of the desired accuracy.

Do take note not to be overly ambitious when it comes to setting the number of decimal places to be accurate to. Due to the limitation of `float` and `double` in Arduino, it might take an unrealistically long time or may never reach the desired decimal place accuracy. For AVR Arduinos like Uno, 4 to 5 is the most you should go.

This mode relies on a copy of pi in the library as a reference. Also, take note that this library uses the standard `srand(...)` and `rand()`from C++, other parts of the software can mess up the random number generator if those are called.

# Multi-threaded Benchmark
It is easy to implement a single-threaded benchmark for a single-core Arduino:
```
#include <MonteCarloPi.h>
MonteCarloPi myPi;
...
myPi.startTimer();
pi = myPi.piLoop(500000); // Loop 500000 times
myPi.stopTimer();

// Display results
// Use getTime(), getSquares(), getPi
...
myPi.reset();
myPi.reseed();
...
myPi.startTimer();
pi = myPi.piToDP(4); // Estimate till correct to 4 decimal places
myPi.stopTimer();

// Display results
...
```

However, it takes more effort to implement a multi-threaded benchmark on a multi-core Arduino. Included in this library is an example for the ESP32. A flowchart is provided to show the steps taken for a dual-core Arduino (like the ESP32) to run a dual-threaded benchmark.

The example makes use of the freeRTOS supported by ESP32 to create and delete multiple tasks or threads. In our tests, the dual cores of ESP32 seemed to perform inconsistently: sometimes Core 0 is faster, sometimes Core 1 is faster and sometimes they are almost as fast. We are not sure why, and we will assume the cores perform differently.

In the example, we will allow freeRTOS to pick whichever core it wants to for single-threaded benchmark. 

While the example demonstrates the benchmark with two threads, feel free to experiment with other number of threads from 1 to 183, where anything more seems to crash the ESP32.

## Loop Benchmark
For Loop benchmark, we use a ticketing system to ensure the exact number of loop runs. Using a mutex, we can assign a ticket number to each loop on different threads that is non-repeating and non-overlapping.

However, using mutex to issue tickets one at a time is slow. So we use chunks of tickets instead. If `ticketChunk` is 1000, the thread will loop 1000 times in one go before obtaining the next ticket. If there are 500,000 loops, then there will be 500 portions to share among the threads.

We cannot use a `ticketChunk` value that is too large. If the cores' performances are not balanced, it may lead to the last chuck being processed by one slow thread alone for a long time while the rest had been deleted.

In an extreme example, there are only two portions for two threads, each thread/core taking one portion. If one core is slower than the other by a lot, then towards the end, one thread will be slowly crunching the data while the other will long gone after finishing its portion. In another extreme scenario, there will only be one portion for two threads/cores, making one of them not contributing.

We cannot use a `ticketChunk` too small, else the frequent taking and releasing of mutex will slow things down.

## Accurate to Decimal Places Benchmark
For Accurate to Decimal Places benchmark, we pin one thread to each core and have them run independently as fast as possible until one of them produce a result that satisfy the accuracy.

## Flowchart
![image](extras/MonteCarloPi_flowchart.svg)

# Default Values
To maintain consistency, these are the default values for some of the constants in the library. It is recommended to try these out first before tweaking them.

| Constant | Variable Name | Default Value | Location | Remarks |
| ---      | ---           | ---           | ---      | ---     |
|No. of Loops | `myLoop` | 500,000 | Example file | Can take a few minutes for slower Arduino like the Uno|
| Decimal Place Accuracy | `decP` | 5 | Example file | |
| Cores | `cores` | 2 | Example file| For ESP32 multi-threaded benchmark |
| Threads | `threads` | 2 | Example | For ESP32 multi-threaded benchmark |
| Ticket Chunk | `ticketChunk` | 1000 | Example file | For ESP32 multi-threaded benchmark |
| Seed | `seed` | 69 | Header file| Arbitrarily chosen |
| Pi Reference | `referencePi` |3.14159265359 | Header file | Static constant double |



# Public Functions

## MonteCarloPi()
Constructor. Will use the default seed.

## MonteCarloPi(_unsigned int_ mySeed)
Constructor where you can set your own seed.

## _double_ piLoop(_unsigned long_ myLoop)
The most basic of all benchmark, see [Loop](#loop). myLoop is the number of times you wish to loop. Returns the estimated value of pi.

## _double_ piToDP(_byte_ myDP)
Estimates pi repeatedly until the desired decimal places, which is myDP, is reached. Returns the estimated value of pi. See [Accurate to Decimal Places](#accurate-to-decimal-places).

## _void_ runOnce()
Generates one random point. Checks if the random point falls within the circle quadrant and increment the total number of points in circle quadrant or square quarter accordingly. Mainly used for multi-core benchmark.

## _void_ setDP(_byte_ myDP)
Sets the decimal place accuracy via myDP. For example setting `setDP(2)`will cause `isAccurate()` to return `true` if the estimation of pi is 3.14XXXXX. See [Accurate to Decimal Places](#accurate-to-decimal-places). Mainly used for multi-core benchmark.

## _bool_ isAccurate()
Checks if the estimated pi value is accurate to the decimal place accuracy set in `setDP(byte myDP)` or `piToDP(byte myDP)`. Remember to run `setDP(byte myDP)` or `piToDP(byte myDP)` before using this function. Mainly used for multi-core benchmark.

## _double_ computePi()
Computes and returns an estimate for pi with the given number of points in the circle quadrant and square quarter. Mainly used for multi-core benchmark.
 
## _static double_ computePi(_unsigned long_ myCircles, _unsigned long_ mySquares)
A static function that allows us to compute pi with our own number of points in circle quadrant(myCircles) and square quarter(mySquares). Mainly used for multi-core benchmark.

A static function is one that can be used without an instance of a class. For example, we should call `MonteCarloPi::computePi(7855,10000)` instead of `myMonteCarloPi.computePi(7855,10000)` after including this library.
 
## _unsigned long_ getSquares()
Returns the total number of points inside the square quarter. This is also the same as the total number of loops performed or the total number of random points generated (since they all fall within the square quarter).

## _unsigned long_ getCircles()
Returns the total number of points inside the circle quadrant.

## _double_ getPi()
Returns the current estimate of pi. `computePi()`, `piLoop(unsigned long myLoop)` or `piToDP(byte myDP)` must run first.

## _byte_ getDP()
Returns the previously set decimal place accuracy via `setDP(byte myDP)` or `piToDP(byte myDP)`.

## _double_ getX()
Returns the most recent x-coordinate of the randomly generated point.

## _double_ getY()
Returns the most recent y-coordinate of the randomly generated point.

## _void_ setCircleSquare(_unsigned long_ myCircles, _unsigned long_ mySquares)
Manually set the the number of points in the circle quadrant(myCircles) and square quarter(mySquares).

## _void_ reset()
Reset all relevant parameters to run the benchmark again. Does not reseed the random number, use `reseed()` if you wish to do so.

## _void_ reseed()
Reset the seed so the pseudo-random number list will restart again. See: [Accurate to Decimal Places](#accurate-to-decimal-places)

## _void_ setSeed(_unsigned int_ mySeed)
Set the seed of the random number generator to mySeed. 

## _void_ startTimer()
Starts the timer. This is useful to time the benchmark. Relies on `millis()`.

## _unsigned long_ stopTimer()
Stops the timer.

## _void_ resetTimer()
Resets the timer to 0.

## _unsigned long_ getTime()
Get the time in milliseconds between `startTimer()` and `stopTimer()`. `startTimer()` needs to run followed by `stopTimer()` for this function to give a meaningful result. 

This function will return the time between the program starting and when `startTimer()` is called if no `stopTimer()` is used.