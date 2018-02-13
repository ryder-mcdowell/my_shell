#include <my_shell.h>

int main() {
  char line[80];


  while (fgets(line, 1000, stdin) != NULL) {

    InputLine *line = parseInput(line);
    Segment *segment = parseSegments(line);

    fprintf(stderr, "line count = %d\n", line->count);

    while (segment->next != NULL) {
      fprintf(stderr, "segment count = %d\n", segment->count);
      for (int i = 0; i < segment->count; i++) {

      }


      segment = segment->next;
    }
    fprintf(stderr, "segment count = %d\n", segment->count);


  }
}
