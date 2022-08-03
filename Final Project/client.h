#pragma once
#include "helper.h"
void checkArguments(int argc, char **argv, char *inputFile, char *ipAdrr, int *port);
void readFile(char *inputFile, request **requestList, int *requestSize);
int parseLine(char *line, request *req);
void *requestHandler(void *arg);
void clearResources();