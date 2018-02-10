#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
using namespace std;

vector <char *> parseInput(char line[80]) {
  char *ptr;
  vector <char *> argsVector;

  ptr = strtok(line, " ");
  while (ptr != NULL) {
    argsVector.push_back(ptr);
    ptr = strtok(NULL, " ");
  }

  return argsVector;
}



void redirectOut(int i, int args_count) {
  if (i == 0) {
    if (args_count == 1) {
      //only arg
      fprintf(stderr, ">: only arg\n");
    } else {
      //first arg
      fprintf(stderr, ">: first arg\n");
    }
  } else if (i == args_count - 1) {
    //last arg
    fprintf(stderr, ">: last arg\n");
  } else {
    //middle arg
    fprintf(stderr, ">: middle arg\n");
  }
}

void redirectIn(int i, int args_count) {
  if (i == 0) {
    if (args_count == 1) {
      //only arg
      fprintf(stderr, "<: only arg\n");
    } else {
      //first arg
      fprintf(stderr, "<: first arg\n");
    }
  } else if (i == args_count - 1) {
    //last arg
    fprintf(stderr, "<: last arg\n");
  } else {
    //middle arg
    fprintf(stderr, "<: middle arg\n");
  }
}

int main() {
  char line[80];
  vector <char *> argsVector;
  int args_count;
  char **args;

  int i;


  while (gets(line) != NULL) {

    argsVector = parseInput(line);

    args_count = argsVector.size();

    //store arguments
    args = (char **) malloc(sizeof(char *) * (args_count + 1));
    for (i = 0; i < args_count; i++) {
      args[i] = argsVector[i];
    }
    args[args_count] = NULL;

    //print arguments
    //fprintf(stderr, "->%s\n", args[args_count - 1]);
    //fprintf(stderr, "%d\n", args_count);
    //**********************//

    //exit
    if (strcmp("exit", args[0]) == 0) {
      exit(1);
    }

    for (i = 0; i < args_count; i++) {
      if (strcmp(">", args[i]) == 0) {
        redirectOut(i, args_count);
      }
      if (strcmp("<", args[i]) == 0) {
        redirectIn(i, args_count);
      }

    }

    argsVector.clear();
    free(args);
  }
}
