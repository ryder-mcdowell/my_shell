#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
using namespace std;

typedef struct InputLine {
  char **args;
  int count;
} InputLine;

typedef struct Segment {
  char **args;
  int count;
  struct Segment *next;
} Segment;


InputLine *parseInput(char line[80]);
Segment *parseSegments(char line[80]);
Segment *createNewSegment(vector<char *> argsVector, Segment *previous);
