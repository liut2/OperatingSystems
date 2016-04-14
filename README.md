# Simple Shell
This is a quick-and-dirty simple shell program written in C. It supports three main operators:
* | (pipe), which directs the output of the command before it to the input of the command after it.
* > < (I/O redirection), which replace the std input and output with named files.
* & (non-blocking), which sets parent process not to wait for child process to finish.

Note: This version of shell program doesn't handle invalid input error checking yet.
