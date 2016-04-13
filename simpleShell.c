#include <stdio.h>
#include <stdlib.h>
#include "execexample.c"
#include <unistd.h>
#include <string.h>

int * checkFreq(char** words) {
  int *freq = malloc(sizeof(int) * 4);
  for (int i = 0; i < 4; i++) {
    freq[i] = 0;
  }
  int i = 0;
  while (words[i] != NULL) {
    if (strcmp(words[i], ">") == 0) {
      freq[0]++;
    }else if (strcmp(words[i], "<") == 0) {
      freq[1]++;
    }else if (strcmp(words[i], "|") == 0) {
      freq[2]++;
    }else if (strcmp(words[i], "&") == 0) {
      freq[3]++;
      words[i] = NULL;
    }
    printf("%s and the index is %d\n", words[i], i);
    i++;
  }
  return freq;
}

int main(void) {
  int pid;
  int status;
  while (1) {
    printf("Please enter a shell command: ");
    fflush(stdout);
    printf("main pid is %d", getpid());
    char** words = readLineOfWords();
    //pre-process before exec
    int i=0;
    //these slots stand for the freq of these tokens in the input string arr
    //>, <, |, &
    int *fre = checkFreq(words);
    for (int i  =0; i < 4; i++) {
      printf("%d\n", fre[i]);
    }
    //check with or without &

    if (fre[3] == 0){
      pid = fork();
      //the parent process split into two
      if (pid == 0) {
        //it's child process's turn
        execvp(words[0], words);
      } else {
        printf("parent pid is %d\n", getpid());
        //it's parent process's turn
        int pppppid = waitpid(-1, &status, 0);
        printf("child pid is %d\n", pppppid);
      }
    }else if (fre[3] == 1){
      pid = fork();
      if (pid == 0) {
        //it's child process's turn
        execvp(words[0], words);
      } else {
        //it's parent process's turn
        //wait(&status);
      }
    }

    // execute command in words[0] with arguments in array words
    // by convention first argument is command itself, last argument must be
    printf("\n");
  }

  return 0;
}
