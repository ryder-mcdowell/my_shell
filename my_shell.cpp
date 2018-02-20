#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
using namespace std;

#define TRUE 1
#define FALSE 0
#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct InputLine {
  char **args;
  int count;
  int segments_count;
} InputLine;

typedef struct Segment {
  char **args;
  int count;
  int hasRedirect;
  struct Segment *next;
} Segment;

InputLine *parseInput(char line[255]) {
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

Segment *createNewSegment(vector<char *> argsVector, Segment *previous, int hasRedirect) {
  //create segment
  Segment *segment = (Segment *) malloc(sizeof(Segment));

  if (!segment) {
    perror("MALLOC ERROR:");
    exit(1);
  }

  //set count
  segment->count = argsVector.size();

  //set hasRedirect
  segment->hasRedirect = hasRedirect;

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

Segment *parseSegments(char line[255], InputLine *input) {
  char *ptr;
  vector <char *> argsVector;
  Segment *first = NULL;
  Segment *current = NULL;
  int hasRedirect = FALSE;

  //remove newline
  line[strlen(line) - 1] = '\0';

  //get line
  ptr = strtok(line, " ");
  while (ptr != NULL) {
    argsVector.push_back(ptr);
    if (strcmp(ptr, "<" ) == 0 || strcmp(ptr, ">" ) == 0 || strcmp(ptr, "2>" ) == 0 || strcmp(ptr, ">>" ) == 0) {
      hasRedirect = TRUE;
    }
    if (strcmp(ptr, "|" ) == 0) {
      input->segments_count += 1;
      argsVector.pop_back();
      if (first == NULL) {
        //first segment
        current = createNewSegment(argsVector, NULL, hasRedirect);
        first = current;
      } else {
        //next segment
        current = createNewSegment(argsVector, current, hasRedirect);
      }
      argsVector.clear();
    }
    ptr = strtok(NULL, " ");
  }
  if (first == NULL) {
    //only segment
    first = createNewSegment(argsVector, NULL, hasRedirect);
    input->segments_count += 1;
  } else {
    //last segment
    createNewSegment(argsVector, current, hasRedirect);
    input->segments_count += 1;
  }

  argsVector.clear();
  return first;
}

void redirectOut(int i, Segment *segment) {
  if (i == 0) {
    if (segment->count == 1) {
      //ONLY ARG
      fprintf(stderr, ">: file redirection\n");
    } else {
      //FIRST
      fprintf(stderr, "Missing program or utilty to redirect from\n");
    }
  } else if (i == segment->count - 1) {
    //LAST ARG
    fprintf(stderr, "Missing filename for redirect\n");
  } else {
    //MIDDLE ARG
    const char *filename = segment->args[i + 1];
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
        segment->args[i] = NULL;
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectIn(int i, Segment *segment) {
  if (i == 0) {
    if (segment->count == 1) {
      //ONLY ARG
      fprintf(stderr, "<: file redirection\n");;
    } else {
      //FIRST ARG
      fprintf(stderr, "Missing filename for redirect\n");
    }
  } else if (i == segment->count - 1) {
    //last arg
    fprintf(stderr, "Missing program or utilty to redirect from\n");
  } else {
    //MIDDLE ARG
    const char *filename = segment->args[i - 1];
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (fd < 0) {
      perror("ERROR");
      exit(1);
    }

    char **args = &segment->args[i + 1];

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
        execvp(args[0], args);
        fprintf(stderr, "-my_shell: %s: command not found\n", args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectOutAppend(int i, Segment *segment) {
  if (i == 0) {
    if (segment->count == 1) {
      //only arg
      fprintf(stderr, ">>: file redirection (append)\n");
    } else {
      //first arg
      fprintf(stderr, "Missing program or utilty to redirect from\n");
    }
  } else if (i == segment->count - 1) {
    //last arg
    fprintf(stderr, "Missing filename for redirect\n");
  } else {
    //MIDDLE ARG
    const char *filename = segment->args[i + 1];
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
        segment->args[i] = NULL;
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

void redirectOutError(int i, Segment *segment) {
  if (i == 0) {
    if (segment->count == 1) {
      //ONLY ARG
      fprintf(stderr, "2>: file redirection (stderr)\n");
    } else {
      //FIRST ARG
      fprintf(stderr, "Missing program or utilty to redirect from\n");
    }
  } else if (i == segment->count - 1) {
    //LAST ARG
    fprintf(stderr, "Missing filename for redirect\n");
  } else {
    //MIDDLE ARG
    const char *filename = segment->args[i + 1];
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
        segment->args[i] = NULL;
        execvp(segment->args[0], segment->args);
        fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
      default:
        wait(NULL);
    }
  }
}

int handleRedirects(Segment *segment) {
  for (int i = 0; i < segment->count; i++) {
    if (strcmp(">", segment->args[i]) == 0) {
      redirectOut(i, segment);
      segment->args[i] = NULL;
      return TRUE;
    }
    else if (strcmp(">>", segment->args[i]) == 0) {
      redirectOutAppend(i, segment);
      segment->args[i] = NULL;
      return TRUE;
    }
    else if (strcmp("2>", segment->args[i]) == 0) {
      redirectOutError(i, segment);
      segment->args[i] = NULL;
      return TRUE;
    }
    else if (strcmp("<", segment->args[i]) == 0) {
      redirectIn(i, segment);
      segment->args[i] = NULL;
      return TRUE;
    }
  }
  return FALSE;
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
  char line[255];
  int check_int;

  //CMD Prompt
  if (argc == 2) {
    printf("%s>", argv[1]);
  } else if (argc > 2) {
    fprintf(stderr, "ERROR: Too many command line arguments.\nUSAGE: ./my_shell [CMDPrompt]\n");
    exit(1);
  } else {
    printf("my_shell>");
  }

  while (fgets(line, 1000, stdin) != NULL) {
    int redirect = FALSE;

    //avoid only newline entry
    if (strlen(line) > 1) {

      //create structs from input line
      char* line_dup = strdup(line);
      InputLine *input = parseInput(line);
      Segment *segment = parseSegments(line_dup, input);

      //save first for freeing memory later
      Segment *first = segment;

      //free duplicated line
      free(line_dup);

      //exit
      if (strcmp("exit", segment->args[0]) == 0) {
        freeStructMemory(input, first);
        exit(0);
      }

      //print CMD Prompt
      if (argc == 2) {
        printf("%s>", argv[1]);
      } else {
        printf("my_shell>");
      }

      //NO PIPING
      if (input->segments_count == 1) {
        redirect = handleRedirects(segment);
        if (redirect == FALSE) {
          switch(fork()) {
            case 0:
              execvp(segment->args[0], segment->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
              exit(1);
            default:
              wait(NULL);
          }
        }

      //PIPING
      } else if (input->segments_count > 1 && input->segments_count < 4) {
        int pipe[2], Npipe[2];
        pipe2(pipe, 0);
        pipe2(Npipe, 0);

        //-ONE PIPE
        if (input->segments_count == 2) {
          switch(fork()) {
            case 0:
              //write end of pipe <-- stdout
              if (dup2(pipe[1], STDOUT) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(pipe[0]);
              execvp(segment->args[0], segment->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
              exit(1);
            default:
              close(pipe[1]);
              wait(NULL);
          }

          switch(fork()) {
            case 0:
              //read end of pipe <-- stdin
              if (dup2(pipe[0], STDIN) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(pipe[1]);
              handleRedirects(segment->next);
              execvp(segment->next->args[0], segment->next->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->next->args[0]);
              exit(1);
            default:
              close(pipe[0]);
              wait(NULL);
          }
        }

        //-TWO PIPES
        if (input->segments_count == 3) {
          switch(fork()) {
            case 0:
              //write end of pipe <-- stdout
              if (dup2(pipe[1], STDOUT) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(pipe[0]);
              close(Npipe[0]);
              close(Npipe[1]);
              execvp(segment->args[0], segment->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->args[0]);
              exit(1);
            default:
              close(pipe[1]);
              wait(NULL);
          }

          switch(fork()) {
            case 0:
              //read end of pipe <-- stdin
              if (dup2(pipe[0], STDIN) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(pipe[1]);
              //write end of next pipe <-- stout
              if (dup2(Npipe[1], STDOUT) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(Npipe[0]);
              execvp(segment->next->args[0], segment->next->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->next->args[0]);
              exit(1);
            default:
              close(pipe[0]);
              close(Npipe[1]);
              wait(NULL);
          }

          segment = segment->next;
          switch(fork()) {
            case 0:
              //read end of next pipe <-- stdin
              if (dup2(Npipe[0], STDIN) < 0) {
                perror("ERROR");
                exit(1);
              }
              close(Npipe[1]);
              close(pipe[0]);
              close(pipe[1]);
              handleRedirects(segment->next);
              execvp(segment->next->args[0], segment->next->args);
              fprintf(stderr, "-my_shell: %s: command not found\n", segment->next->args[0]);
              exit(1);
            default:
              close(Npipe[0]);
              wait(NULL);
          }

        }
      //MORE THAN 2 PIPES
      } else {
        fprintf(stderr, "ERROR: Sorry, my_shell only supports maximum of two pipes\n");
      }

      freeStructMemory(input, first);

    } else {
      //nothing entered, print CMD prompt
      if (argc == 2) {
        printf("%s>", argv[1]);
      } else {
        printf("my_shell>");
      }
    }
  }
}
