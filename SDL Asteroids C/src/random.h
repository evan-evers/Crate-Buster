#ifndef RANDOM_H
#define RANDOM_H

/*
* Functions for generating random numbers.
* Don't forget to initialize psuedorandomness with srand(time(NULL));
*/

int randInt(int upperBound);
int randIntRange(int lowerBound, int upperBound);
float randFloat(float upperBound);
float randFloatRange(float lowerBound, float upperBound);

#endif