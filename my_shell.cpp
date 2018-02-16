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
  int segments_count;
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

  if (!input) {
    perror("MALLOC ERROR:");
    exit(1);
  }

  //set segments_count for parseSegments to add to
  input->segments_count = 0;

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
  if (!input->args) {
    perror("MALLOC ERROR:");
    exit(1);
  }
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

  if (!segment) {
    perror("MALLOC ERROR:");
    exit(1);
  }

  //set count
  segment->count = argsVector.size();

  //set args char-style
  segment->args = (char **) malloc(sizeof(char *) * (segment->count + 1));
  if (!segment->args) {
    perror("MALLOC ERROR:");
    exit(1);
  }
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

Segment *parseSegments(char line[80], InputLine *input) {
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
      input->segments_count += 1;
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
    input->segments_count += 1;
  } else {
    //last segment
    createNewSegment(argsVector, current);
    input->segments_count += 1;
  }

  argsVector.clear();
  return first;
}

void redirectOut(int i, InputLine *input, Segment *segment) {
  //int check_int;

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
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
      perror("ERROR");
      exit(1);
    }

    switch(fork()) {
      case 0:
        if (dup2(fd, 1) < 0) {
          perror("ERROR");
          exit(1);
        }
        if (close(fd) < 0) {
          perror("ERROR");
          exit(1);
        }
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_bash: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectIn(int i, InputLine *input, Segment *segment) {
  //int check_int;

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
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
      perror("ERROR");
      exit(1);
    }

    switch(fork()) {
      case 0:
        if (dup2(fd, 1) < 0) {
          perror("ERROR");
          exit(1);
        }
        if (close(fd) < 0) {
          perror("ERROR");
          exit(1);
        }
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_bash: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectOutAppend(int i, InputLine *input, Segment *segment) {
  //int check_int;

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
    int fd = open(filename, O_WRONLY | O_APPEND);
    if (fd < 0) {
      perror("ERROR");
      exit(1);
    }

    switch(fork()) {
      case 0:
        if (dup2(fd, 1) < 0) {
          perror("ERROR");
          exit(1);
        }
        if (close(fd) < 0) {
          perror("ERROR");
          exit(1);
        }
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_bash: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectOutError(int i, InputLine *input, Segment *segment) {
  //int check_int;

  if (i == 0) {
    if (input->count == 1) {
      //only arg
      fprintf(stderr, "2>: file redirection (stderr)\n");
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
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
      perror("ERROR");
      exit(1);
    }

    switch(fork()) {
      case 0:
        if (dup2(fd, 2) < 0) {
          perror("ERROR");
          exit(1);
        }
        if (close(fd) < 0) {
          perror("ERROR");
          exit(1);
        }
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_bash: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void freeStructMemory(InputLine *input, Segment *segment) {
  //free segments
  Segment *tmp;
  while (segment != NULL) {
    for (int i = 0; i < segment->count + 1; i++) {
      free(segment->args[i]);
    }
    free(segment->args);
    tmp = segment;
    segment = segment->next;
    free(tmp);
  }

  //free input line
  for (int i = 0; i < input->count + 1; i++) {
    free(input->args[i]);
  }
  free(input->args);
  free(input);
}

int main(int argc, char **argv) {
  char line[80];           //what should line's size be?
  int onlyArg, check_int;

  //CMD Prompt
  if (argc == 2) {
    printf("%s>", argv[1]);
  } else if (argc > 2) {
    fprintf(stderr, "ERROR: Too many command line arguments.\nUSAGE: ./my_shell [CMDPrompt]\n");
    exit(1);
  } else {
    printf("my_shell>");
  }

  while (fgets(line, 1000, stdin) != NULL) {      //check?
    onlyArg = 1;
    char* line_dup = strdup(line);

    //CMD Prompt
    if (argc == 2) {
      printf("%s>", argv[1]);
    } else {
      printf("my_shell>");
    }

    //create structs from input line
    InputLine *input = parseInput(line);
    Segment *segment = parseSegments(line_dup, input);

    //save first for freeing memory later
    Segment *first = segment;

    //free duplicated line
    free(line_dup);

    //exit
    if (strcmp("exit", input->args[0]) == 0) {
      freeStructMemory(input, first);
      exit(0);
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
        if (strcmp(">>", input->args[i]) == 0) {
          redirectOutAppend(i, input, segment);
          onlyArg = 0;
        }
        if (strcmp("2>", input->args[i]) == 0) {
          redirectOutError(i, input, segment);
          onlyArg = 0;
        }
      }

      segment = segment->next;
    }
    if (onlyArg == 1) {
      switch(fork()) {
        case 0:
          execvp(segment->args[0], segment->args);
          fprintf(stderr, "-my_bash: %s: command not found\n", segment->args[0]);
        default:
          wait(NULL);
      }
    }
    freeStructMemory(input, first);
  }
}
