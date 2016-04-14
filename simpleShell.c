#include <stdio.h>
#include <stdlib.h>
#include "execexample.c"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

//this function checks to see which operator is present
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
    i++;
  }
  return freq;
}
//this function handles I/O redirection
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
//this struct holds a single command from a pipe operation
struct Command{
  char** cmd;
};
//fork a new process to deal with the next command
void subProcess(int curInFd, int curOutFd, char** cmd){
  int pid = fork();
  if (pid == 0) {
    //if the read file descriptor isn't 0, then it means we are reading not from std input but from the pipe of last command
    if (curInFd != 0) {
      dup2(curInFd, 0);
      close(curInFd);
    }
    //if the write file descriptor isn't 1, then it means we are writing to the write end of a pipe
    if (curOutFd != 1) {
      dup2(curOutFd, 1);
      close(curOutFd);
    }
    execvp(cmd[0], cmd);
  }else {
    //parent does nothing here
  }
}
//this function handles pipe operation
void pipeHelper(int *fre, char** words) {
  struct Command myCommands[(fre[2] + 1)];
  //i to index input words
  int i = 0;
  //j to index each individual command
  int j = 0;
  //k to index struct array
  int k = 0;
  //parse the input into groups of commands
  char** curCmd = malloc(sizeof(char*) * 50);
  while (words[i] != NULL) {
    if (strcmp(words[i], "|") == 0) {
      curCmd[j] = NULL;
      myCommands[k].cmd = curCmd;
      k++;
      curCmd = malloc(sizeof(char*) * 50);
      j = 0;
    }else {
      curCmd[j] = words[i];
      j++;
    }
    i++;
  }
  curCmd[j] = NULL;
  myCommands[k].cmd = curCmd;
  //for loop to deal with each command
  int curInFd = 0;
  int curOutFd;
  int fds[2];
  for (int i = 0; i < fre[2]; i++) {
    //call pipe to create two file descriptors, one for read and one for write
    pipe(fds);
    //fork a new process to handle the next command
    subProcess(curInFd, fds[1], myCommands[i].cmd);
    //we don't need to output to stdoutput, so close it
    close(fds[1]);
    //carry the input file descriptor to next command, since the new command will read input from here
    curInFd = fds[0];
  }
  if (curInFd != 0) {
    dup2(curInFd, 0);
  }
  //since the output of the last command will go to std output, we handle it outside the for loop
  execvp(myCommands[fre[2]].cmd[0], myCommands[fre[2]].cmd);
}
//main function
int main(void) {
  int pid;
  int status;
  while (1) {
    //get user input
    printf("Please enter a shell command: ");
    fflush(stdout);
    char** words = readLineOfWords();
    //check to see which operator is present and count their frequencies
    int *fre = checkFreq(words);
    pid = fork();
    //the parent process split into two
    if (pid == 0) {
      if (fre[2] != 0) {
        //if pipe is present, then handle it
        pipeHelper(fre, words);
      } else {
        if (fre[0] != 0 || fre[1] != 0) {
          //if > or < is present, handle redirection operators
          redirectionHelper(fre, words);
        }
        //it's child process's turn
        execvp(words[0], words);
      }
    } else {
      //it's parent process's turn
      if (fre[3] == 0) {
        //if & is present, then handle & operator
        int returnPid = waitpid(pid, &status, 0);
      }
    }
  }
  return 0;
}
