#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
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
  for (i = 0; i < input->count; i++) {
    input->args[i] = strdup(argsVector[i]);    //need strdup?
  }
  input->args[input->count] = NULL;


  argsVector.clear();
  return input;
}

void redirectOut(int i, InputData *input) {
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

void redirectIn(int i, InputData *input) {
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

void freeMemory(InputData *input) {
  for (int i = 0; i < input->count + 1; i++) {
    free(input->args[i]);
  }
  free(input->args);
  free(input);
}


int main() {
  char line[80];
  int i;
  int isNewSegment = 1;

  while (fgets(line, 1000, stdin) != NULL) {

    InputData *input = parseInput(line);

    if (input->count != 0) {

      //exit
      if (strcmp("exit", input->args[0]) == 0) {
        freeMemory(input);
        exit(1);
      }

      //exec
      for (int i = 0; i < input->count; i++) {
        fprintf(stderr, "LOOP %d\n", i);
        switch(fork()) {
          case 0:
            execvp(input->args[i], input->args);
            default:
            wait(NULL);
          }
        }

        //redirection
        if (strcmp(">", input->args[i]) == 0) {
          redirectOut(i, input);
        }
        if (strcmp("<", input->args[i]) == 0) {
          redirectIn(i, input);
        }

      }

    }


    freeMemory(input);
  }
}
