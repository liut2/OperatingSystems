#include <stdio.h>
#include <stdlib.h>
#include "execexample.c"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

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
    //printf("%s and the index is %d\n", words[i], i);
    i++;
  }
  return freq;
}

void redirectionHelper(int *fre, char** words) {
  //first pass to call open and dup2
  char* cur;
  int curInFd;
  int curOutFd;
  int i = 0;
  while (words[i] != NULL && (fre[0] != 0 || fre[1] != 0)) {
    if (strcmp(words[i], ">") == 0) {
      cur = words[i + 1];
      curOutFd = open(cur, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      dup2(curOutFd, 1);
      close(curOutFd);
      fre[0]--;
    }
    if (strcmp(words[i], "<") == 0) {
      cur = words[i + 1];
      curInFd = open(cur, O_RDONLY);
      dup2(curInFd, 0);
      close(curInFd);
      fre[1]--;
    }
    i++;
  }
  //second pass to remove the tokes from string array
  i = 0;
  while (words[i] != NULL) {
    if (strcmp(words[i], ">") == 0 || strcmp(words[i], "<") == 0) {
      words[i] = NULL;
      break;
    }
    i++;
  }
}

int main(void) {
  int pid;
  int status;
  while (1) {
    //get user input
    printf("Please enter a shell command: ");
    fflush(stdout);
    char** words = readLineOfWords();
    //pre-process
    int *fre = checkFreq(words);
    for (int i = 0; i < 4; i++) {
      printf("%d ", fre[i]);
    }
    printf("\n");
    //now deal with I/O redirection

    //check with or without &
    if (fre[3] == 0){
      pid = fork();
      //the parent process split into two
      if (pid == 0) {
        if (fre[0] != 0 || fre[1] != 0) {
          //then it means we have I/O redirection to do, call open and dup2 continuously
          redirectionHelper(fre, words);
        }
        //it's child process's turn
        execvp(words[0], words);
      } else {
        //it's parent process's turn
        int returnPid = waitpid(pid, &status, 0);
      }
    }else if (fre[3] == 1){
      pid = fork();
      if (pid == 0) {
        //it's child process's turn
        if (fre[0] != 0 || fre[1] != 0) {
          //then it means we have I/O redirection to do, call open and dup2 continuously
          redirectionHelper(fre, words);
        }
        execvp(words[0], words);
      } else {
        //it's parent process's turn
        //wait(&status);
      }
    }
  }

  return 0;
}
