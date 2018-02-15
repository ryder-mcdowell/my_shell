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
    if (strcmp(ptr, "<" ) == 0 || strcmp(ptr, ">" ) == 0 || strcmp(ptr, "2>" ) == 0 || strcmp(ptr, ">>" ) == 0 || strcmp(ptr, "|" ) == 0) {
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

void redirectOut(int i, InputLine *input, Segment *segment) {
  int check_int;

  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, ">: file redirection\n");
    } else {
      //first arg
      fprintf(stderr, "Missing program or utilty to redirect from\n");
    }
  } else if (i == input->count - 1) {
    //last arg
    fprintf(stderr, "Missing filename for redirect\n");
  } else {
    //MIDDLE ARG
    const char *filename = input->args[i + 1];
    int fd = open(filename, O_CREAT|O_WRONLY, 0777);

    switch(fork()) {
      case 0:
        dup2(fd, 1);
        close(fd);
        execvp(segment->args[0], segment->args);
      default:
        wait(NULL);
    }
  }
}

void redirectIn(int i, InputLine *input, Segment *segment) {
  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, "<: file redirection\n");;
    } else {
      //first arg
      fprintf(stderr, "Missing filename for redirect\n");
    }
  } else if (i == input->count - 1) {
    //last arg
    fprintf(stderr, "Missing program or utilty to redirect from\n");
  } else {
    //MIDDLE ARG
    const char *filename = input->args[i - 1];
    int fd = open(filename, O_CREAT|O_WRONLY, 0777);

    switch(fork()) {
      case 0:
        dup2(fd, 1);
        close(fd);
        execvp(segment->args[0], segment->args);
      default:
        wait(NULL);
    }
  }
}

void redirectOutAppend(int i, InputLine *input, Segment *segment) {
  int check_int;

  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, ">>: file redirection (append)\n");
    } else {
      //first arg
      fprintf(stderr, "Missing program or utilty to redirect from\n");
    }
  } else if (i == input->count - 1) {
    //last arg
    fprintf(stderr, "Missing filename for redirect\n");
  } else {
    //MIDDLE ARG
    const char *filename = input->args[i + 1];
    int fd = open(filename, O_WRONLY|O_APPEND, 0777);

    switch(fork()) {
      case 0:
        dup2(fd, 1);
        close(fd);
        execvp(segment->args[0], segment->args);
      default:
        wait(NULL);
    }
  }
}

void freeMemory(InputLine *input, Segment *segment) {
  for (int i = 0; i < input->count + 1; i++) {
    free(input->args[i]);
  }
  free(input->args);
  free(input);

  // while (segment->next != NULL) {
  //   for (i = 0; i < segment->count + 1; i++) {
  //     free(segment->args[i]);
  //   }
  //   free(segment->args);
  //
  //   free(segment->next);
  //   free(segment);
  // }
  free(segment->next);
  free(segment->args);
  free(segment);

}

int main() {
  char line[80];
  int onlyArg;

  while (fgets(line, 1000, stdin) != NULL) {
    onlyArg = 1;

    InputLine *input = parseInput(strdup(line));
    Segment *segment = parseSegments(strdup(line));

    //exit
    if (strcmp("exit", input->args[0]) == 0) {
      freeMemory(input, segment);
      exit(1);
    }

    while (segment->next != NULL) {
      for (int i = 0; i < segment->count + 2; i++) {
        //redirection
        if (strcmp(">", input->args[i]) == 0) {
          redirectOut(i, input, segment);
          onlyArg = 0;
        }
        if (strcmp("<", input->args[i]) == 0) {
          redirectIn(i, input, segment->next);
          onlyArg = 0;
        }
      }

      segment = segment->next;
    }
    if (onlyArg == 1) {
      switch(fork()) {
        case 0:
          execvp(segment->args[0], segment->args);
        default:
          wait(NULL);
      }
    }
    freeMemory(input, segment);
  }
}
