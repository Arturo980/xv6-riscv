#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  // Probando getpid y getppid
  int pid = getpid();
  int ppid = getppid();

  printf("Mi PID es %d y el de mi padre es %d\n", pid, ppid);
  for(int i = 0; i <= 3; i++) {
    printf("Ancestor %d : %d\n", i, getancestor(i));
  }
  printf("Ancestor 10: %d\n", getancestor(10));

  exit(0);
}
