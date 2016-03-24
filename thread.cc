#include "utils.h"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

std::string utils::exec(const char* cmd, int *status, int timeout) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid;
  pid = fork();
  switch (pid) {
    case -1: perror("fork"); exit(1);
    case 0: {
      // child
      if (timeout>0) alarm(timeout);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      execl("/bin/sh", "sh", "-c", cmd, (char *) NULL);
      perror("execl");
      exit(1);
    }
    default: {
      // parent
      close(pipefd[1]);
      // fdopen(pipefd[0], "r");
      char buf[BUFSIZ];
      int nread;
      std::string result;
      while ((nread = read(pipefd[0], buf, sizeof(buf))) != 0) {
        if (nread == -1) {
          perror("read");
        } else {
          result.append(buf, nread);
        }
      }
      close(pipefd[0]);
      int _status;
      waitpid(pid, &_status, 0);
      if (WIFEXITED(_status) && status != NULL) {
        *status = WEXITSTATUS(_status);
      }
      return result;
    }
  }
}

static int split(const char *scon, char** &argv) {
  char *s = strdup(scon);
  char *tok = strtok(s, " ");
  argv = (char**)malloc(10*sizeof(char*));
  int argc = 0;
  while (tok) {
    argv[argc] = (char*)malloc(strlen(tok)+1);
    strcpy(argv[argc], tok);
    tok = strtok(NULL, " ");
    argc++;
  }
  argv[argc] = NULL;
  return argc;
}

std::string utils::exec_in(const char* cmd, const char* input, int *status, unsigned int timeout) {
  // cmd should not exceed BUFSIZ
  char cmd_buf[BUFSIZ];
  strcpy(cmd_buf, cmd);
  std::string result;
  // create pipe
  int pipein[2], pipeout[2];
  if (pipe(pipein) == -1 || pipe(pipeout) == -1) {
    perror("pipe");
    exit(1);
  }
  pid_t pid;
  pid = fork();
  if (pid<0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    // children
    if (timeout>0) alarm(timeout);
    if (dup2(pipein[0], 0) == -1 || dup2(pipeout[1], 1) == -1) {
      perror("dup error");
      exit(1);
    }
    close(pipein[0]);
    close(pipein[1]);
    close(pipeout[0]);
    close(pipeout[1]);
    // prepare the command
    char **argv = NULL;
    // the argv is malloc-ed, but anyway the process will exit, it will be released
    ::split(cmd, argv);
    execvp(argv[0], argv);
    perror("execvp");
    exit(1);
  }
  // parent
  close(pipein[0]);
  close(pipeout[1]);
  // write to child's input
  write(pipein[1], input, strlen(input));
  close(pipein[1]);
  char buf[BUFSIZ];
  int nread;
  while ((nread = read(pipeout[0], buf, sizeof(buf))) != 0) {
    if (nread == -1) {
      perror("read");
    } else {
      result.append(buf, nread);
    }
  }
  close(pipeout[0]);
  int _status;
  waitpid(pid, &_status, 0);
  if (WIFEXITED(_status) && status != NULL) {
    *status = WEXITSTATUS(_status);
  }
  return result;
  return result;
}

