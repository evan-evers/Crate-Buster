#include <stdlib.h>

//returns a random non-negative integer
//upper bound included
int randInt(int upperBound) {
	return rand() % upperBound + 1;
}

//returns a random integer between lowerBound and upperBound
//upper bound included
int randIntRange(int lowerBound, int upperBound) {
	return rand() % (upperBound - lowerBound + 1) + lowerBound;
}

//returns a random non-negative float
//upper bound included
float randFloat(float upperBound) {
	return ((float)rand() / (float)RAND_MAX) * upperBound;
}

//returns a random non-negative float
//upper bound included
float randFloatRange(float lowerBound, float upperBound) {
	return ((float)rand() / (float)RAND_MAX) * (upperBound - lowerBound) + lowerBound;
}