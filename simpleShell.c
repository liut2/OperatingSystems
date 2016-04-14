// Author: Tao Liu and Xi Chen
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
char** readLineOfWords();

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
int redirectionHelper(int *fre, char** words) {
  //first pass to call open and dup2
  char* cur;
  int curInFd;
  int curOutFd;
  int i = 0;
  while (words[i] != NULL && (fre[0] != 0 || fre[1] != 0)) {
    if (strcmp(words[i], ">") == 0) {
      cur = words[i + 1];
      curOutFd = open(cur, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      if (curOutFd > 0){
        dup2(curOutFd, 1);
        close(curOutFd);
      }else{
        printf("Fail to create the file. \n");
        fflush(stdout);
        return -1;
      }
      fre[0]--;
    }
    if (strcmp(words[i], "<") == 0) {
      cur = words[i + 1];
      curInFd = open(cur, O_RDONLY);
      if (curInFd > 0){
        dup2(curInFd, 0);
        close(curInFd);
      }else{
        printf("Fail to open the file. \n");
        fflush(stdout);
        return -1;
      }
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
  return 1;
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
    int e = execvp(cmd[0], cmd);

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
  int z = execvp(myCommands[fre[2]].cmd[0], myCommands[fre[2]].cmd);
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
    int valid = 1;
    pid = fork();
    //the parent process split into two
    if (pid == 0) {
      if (fre[2] != 0 && (fre[0] != 0 || fre[1] != 0)) {
        valid = redirectionHelper(fre, words);
        //if pipe is present, then handle it
        if (valid == 1){
          pipeHelper(fre, words);
        }
      } else if (fre[2] != 0){
       //if pipe is present, then handle it
        pipeHelper(fre, words);
      }else{
        if (fre[0] != 0 || fre[1] != 0) {
          valid = redirectionHelper(fre, words);
        }
        if (valid == 1){
          //it's child process's turn
          int e = execvp(words[0], words);
          if (e == -1){
            printf("invalid token. An error has occurred. \n");
            fflush(stdout);
          }
        }
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

/*
 * reads a single line from terminal and parses it into an array of tokens/words by
 * splitting the line on spaces.  Adds NULL as final token
 */
char** readLineOfWords() {

  // A line may be at most 100 characters long, which means longest word is 100 chars,
  // and max possible tokens is 51 as must be space between each
  size_t MAX_WORD_LENGTH = 100;
  size_t MAX_NUM_WORDS = 51;

  // allocate memory for array of array of characters (list of words)
  char** words = (char**) malloc( MAX_NUM_WORDS * sizeof(char*) );
  int i;
  for (i=0; i<MAX_NUM_WORDS; i++) {
    words[i] = (char*) malloc( MAX_WORD_LENGTH );
  }

  // read actual line of input from terminal
  int bytes_read;
  char *buf;
  buf = (char*) malloc( MAX_WORD_LENGTH+1 );
  bytes_read = getline(&buf, &MAX_WORD_LENGTH, stdin);

  // take each word from line and add it to next spot in list of words
  i=0;
  char* word = (char*) malloc( MAX_WORD_LENGTH );
  word = strtok(buf, " \n");
  while (word != NULL && i<MAX_NUM_WORDS) {
    strcpy(words[i++], word);
    word = strtok(NULL, " \n");
  }

  // check if we quit because of going over allowed word limit
  if (i == MAX_NUM_WORDS) {
    printf( "WARNING: line contains more than %d words!\n", (int)MAX_NUM_WORDS );
  }
  else
    words[i] = NULL;

  // return the list of words
  return words;
}
