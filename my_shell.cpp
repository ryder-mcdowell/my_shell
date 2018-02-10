#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
using namespace std;

typedef struct {
  char **args;
  int count;
} InputData;

InputData *parseInput(char line[80]) {
  char *ptr;
  vector <char *> argsVector;
  int i;
  InputData *input;

  input = (InputData *) malloc(sizeof(InputData));

  //get line
  ptr = strtok(line, " ");
  while (ptr != NULL) {
    argsVector.push_back(ptr);
    ptr = strtok(NULL, " ");
  }
  input->count = argsVector.size();

  //convert to char-style
  input->args = (char **) malloc(sizeof(char *) * (input->count + 1));
  for (i = 0; i < input->count; i++) {
    input->args[i] = strdup(argsVector[i]);
  }
  input->args[input->count] = NULL;


  argsVector.clear();
  return input;
}

void redirectOut(int i, int count) {
  if (i == 0) {
    if (count == 1) {
      //only arg
      fprintf(stderr, ">: only arg\n");
    } else {
      //first arg
      fprintf(stderr, ">: first arg\n");
    }
  } else if (i == count - 1) {
    //last arg
    fprintf(stderr, ">: last arg\n");
  } else {
    //middle arg
    fprintf(stderr, ">: middle arg\n");
  }
}

void redirectIn(int i, int count) {
  if (i == 0) {
    if (count == 1) {
      //only arg
      fprintf(stderr, "<: only arg\n");
    } else {
      //first arg
      fprintf(stderr, "<: first arg\n");
    }
  } else if (i == count - 1) {
    //last arg
    fprintf(stderr, "<: last arg\n");
  } else {
    //middle arg
    fprintf(stderr, "<: middle arg\n");
  }
}

void freeMemory(InputData *input) {
  for (int i = 0; i < input->count; i++) {
    fprintf(stderr, "free%d\n", i);
    free(input->args[i]);
  }
  free(input);
}

int main() {
  char line[80];
  int i;


  while (gets(line) != NULL) {

    InputData *input = parseInput(line);


    //exit
    if (strcmp("exit", input->args[0]) == 0) {
      exit(1);
    }

    for (i = 0; i < input->count; i++) {
      if (strcmp(">", input->args[i]) == 0) {
        redirectOut(i, input->count);
      }
      if (strcmp("<", input->args[i]) == 0) {
        redirectIn(i, input->count);
      }

    }

    freeMemory(input);
  }
}
