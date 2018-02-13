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
        //first segment
        current = createNewSegment(argsVector, NULL);
        first = current;
      } else {
        //next segment
        current = createNewSegment(argsVector, current);
      }
      argsVector.clear();
    }
    ptr = strtok(NULL, " ");
  }
  if (first == NULL) {
    //only segment
    first = createNewSegment(argsVector, NULL);
  } else {
    //last segment
    createNewSegment(argsVector, current);
  }

  argsVector.clear();
  return first;
}

void redirectOut(int i, InputLine *input) {
  FILE *fd;
  int check_int;

  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, ">: only arg\n");
    } else {
      //first arg
      fprintf(stderr, ">: first arg\n");
    }
  } else if (i == input->count - 1) {
    //last arg
    fprintf(stderr, ">: last arg\n");
    fprintf(stderr, "Missing name for redirect\n");
  } else {
    //MIDDLE ARG
    fprintf(stderr, ">: middle arg\n");
    if( access( input->args[i + 1], F_OK ) != -1 ) {
      // file exists
      fd = fopen(input->args[i + 1], "w");

      fclose(fd);
    } else {
      // file doesn't exist
      fprintf(stderr, "File %s does not exist or is invalid\n", input->args[i + 1]);
    }


  }
}

void redirectIn(int i, InputLine *input) {
  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, "<: only arg\n");
    } else {
      //first arg
      fprintf(stderr, "<: first arg\n");
    }
  } else if (i == input->count - 1) {
    //last arg
    fprintf(stderr, "<: last arg\n");
  } else {
    //middle arg
    fprintf(stderr, "<: middle arg\n");
  }
}


int main() {
  char line[80];

  while (fgets(line, 1000, stdin) != NULL) {

    InputLine *input = parseInput(strdup(line));
    Segment *segment = parseSegments(strdup(line));

    while (segment->next != NULL) {
      for (int i = 0; i < input->count; i++) {
        //redirection
        if (strcmp(">", input->args[i]) == 0) {
          redirectOut(i, input);
        }
        if (strcmp("<", input->args[i]) == 0) {
          redirectIn(i, input);
        }
      }

      segment = segment->next;
    }
    switch(fork()) {
      case 0:
        execvp(segment->args[0], input->args);
      default:
        wait(NULL);
    }


  }
}
