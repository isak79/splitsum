/*#include <_stdio.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/file.h> //flock
#include <sys/wait.h> //wait
#include <unistd.h>

/*
 * Function: split_sum_file
 * ----------------------------
 *   Computes the sum of all the numbers in a file
 *   by dividing the workload among n child processes.
 */

int countNumberOfLines(const char *filepath) {
  FILE *file = fopen(filepath, "r");

  if (file == NULL) {
    perror("Error opening file");
    return -1;
  }

  int lineCount = 0;
  char c;
  while ((c = fgetc(file)) != EOF) {
    if (c == '\n') {
      lineCount++;
    }
  }
  fseek(file, -1, SEEK_END);
  if (fgetc(file) != '\n') {
    lineCount++;
  }

  fclose(file);
  return lineCount;
}

int sum_file(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    printf("Could not open file %s\n", filepath);
    return -1;
  }

  char buffer[256];
  int sum = 0;
  while (fgets(buffer, sizeof(buffer), file)) {
    sum += atoi(buffer);
  }

  fclose(file);
  return sum;
}

int split_sum_file(const char *filepath, int n) {
  int id;
  char line[100];
  int i;

  char fileWriteName[100];
  sprintf(fileWriteName, "partial_sum_%d.txt", n);

  int numberOfLines = countNumberOfLines(filepath);
  int parts = numberOfLines / n;
  int rest = numberOfLines % n;

  for (i = 0; i < n; i++) {
    id = fork();
    if (id == 0) {
      /*printf("%d\n", i + 1);*/

      FILE *fileRead = fopen(filepath, "r");

      int startline = i * parts;
      int endline = startline + parts;
      if (i == n - 1) {
        endline += rest;
      }

      int currentline = 0;
      int sum = 0;
      while (fgets(line, sizeof(line), fileRead)) {
        if (startline <= currentline && endline > currentline) {
          /*printf("%s\n", line);*/
          int value = atoi(line);
          sum += value;
        }
        currentline++;
      }

      FILE *fileWrite = fopen(fileWriteName, "a");
      char toWrite[100];
      sprintf(toWrite, "%d\n", sum);
      flock(fileno(fileWrite), LOCK_EX);
      fputs(toWrite, fileWrite);
      flock(fileno(fileWrite), LOCK_UN);

      fclose(fileWrite);
      fclose(fileRead);
      exit(0);
    }
  }
  for (int i = 0; i < n; i++) {
    wait(NULL);
  }
  return sum_file(fileWriteName);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <filepath> <n_split>\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[2]);

  int sum = split_sum_file(argv[1], n);

  printf("%d\n", sum);

  return 0;
}
