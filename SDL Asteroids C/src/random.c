#include <stdlib.h>
#include <stdbool.h>

//returns a random non-negative integer
//upper bound not included
int randInt(int upperBound) {
	return rand() % upperBound;
}

//returns a random integer between lowerBound and upperBound
//upper bound not included
int randIntRange(int lowerBound, int upperBound) {
	return rand() % (upperBound - lowerBound) + lowerBound;
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

//pass in a percent chance between 0.0f and 100.0f
bool percentChance(float percentChance) {
	return percentChance > randFloat(99.99999999999999);
}