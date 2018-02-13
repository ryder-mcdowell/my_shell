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

InputLine *parseInput(char line[80]) {
  char *ptr;
  vector <char *> argsVector;
  InputLine *input;

  input = (InputLine *) malloc(sizeof(InputLine));

  //remove newline
  line[strlen(line) - 1] = '\0';

  //get line
  ptr = strtok(line, " ");
  while (ptr != NULL) {
    argsVector.push_back(ptr);
    ptr = strtok(NULL, " ");
  }
  input->count = argsVector.size();

  //convert to char-style
  input->args = (char **) malloc(sizeof(char *) * (input->count + 1));
  for (int i = 0; i < input->count; i++) {
    input->args[i] = strdup(argsVector[i]);    //need strdup?
  }
  input->args[input->count] = NULL;


  argsVector.clear();
  return input;
}

Segment *createNewSegment(vector<char *> argsVector, Segment *previous) {
  //create segment
  Segment *segment = (Segment *) malloc(sizeof(Segment));

  //set count
  segment->count = argsVector.size();

  //set args char-style
  segment->args = (char **) malloc(sizeof(char *) * (segment->count + 1));
  for (int i = 0; i < segment->count; i++) {
    segment->args[i] = strdup(argsVector[i]);    //need strdup?
  }
  segment->args[segment->count] = NULL;

  //set next
  segment->next = NULL;

  //set previous's next
  if (previous != NULL) {
    previous->next = segment;
  }

  return segment;
}

Segment *parseSegments(char line[80]) {
  char *ptr;
  vector <char *> argsVector;
  Segment *first = NULL;
  Segment *current = NULL;

  //remove newline
  line[strlen(line) - 1] = '\0';

  //get line
  ptr = strtok(line, " ");
  while (ptr != NULL) {
    argsVector.push_back(ptr);
    if (strcmp(ptr, "<" ) == 0 || strcmp(ptr, ">" ) == 0 || strcmp(ptr, "<<" ) == 0 || strcmp(ptr, ">>" ) == 0 || strcmp(ptr, "|" ) == 0) {
      argsVector.pop_back();
      if (first == NULL) {
        fprintf(stderr, "-FIRST SEGMENT\n");
        current = createNewSegment(argsVector, NULL);
        first = current;
      } else {
        fprintf(stderr, "-NEXT SEGMENT\n");
        current = createNewSegment(argsVector, current);
      }
      argsVector.clear();
    }
    ptr = strtok(NULL, " ");
  }
  if (first == NULL) {
    fprintf(stderr, "-ONE SEGMENT\n");
    first = createNewSegment(argsVector, NULL);
  } else {
    fprintf(stderr, "-LAST SEGMENT\n");
    createNewSegment(argsVector, current);
  }

  argsVector.clear();
  return first;
}


int main() {
  char line[80];


  while (fgets(line, 1000, stdin) != NULL) {

    InputLine *input = parseInput(strdup(line));
    Segment *segment = parseSegments(strdup(line));

    fprintf(stderr, "input count = %d\n", input->count);

    while (segment->next != NULL) {
      fprintf(stderr, "segment count = %d\n", segment->count);
      for (int i = 0; i < segment->count; i++) {

      }


      segment = segment->next;
    }
    fprintf(stderr, "segment count = %d\n", segment->count);


  }
}
