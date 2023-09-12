#include "common.h"

#include "utility.h"

//reads a file and returns it as a string
char *readFile(char *filename) {
	char *buffer = NULL;	//the text buffer the file will be written into
	long  length;			//length of file
	FILE *file;				//holds the file in question

	file = fopen(filename, "rb");

	//check if file opened
	if (!file) {
		printf("ERROR: File could not be opened.\n");
		return NULL;
	}
	
	//find size of file in bytes (which is also the size of any ASCII text file in characters)
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);	//reset position indicator

	buffer = malloc(length);	//allocate space for the text buffer
	//memset(buffer, 0, length); //why was this here?
	fread(buffer, 1, length, file);	//read file into buffer

	fclose(file);	//cleanup

	return buffer;
}

//have "val" approach "target" by "increment"
//if "val" is less than "increment" away from "target" in either a negative or positive direction, "val" becomes equal to "target"
void approach(float *val, float target, float increment) {
	if (*val != target) {
		if (*val > target) {
			if (*val - increment > target)
				*val -= increment;
			else
				*val = target;
		} else {
			if (*val + increment < target)
				*val += increment;
			else
				*val = target;
		}
	}
}