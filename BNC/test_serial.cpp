
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  FILE* fp = fopen("/dev/ttyS0", "w");

  if (!fp) {
    printf("Cannot open file.\n");
    exit(1);
  }

  const char* msg = "message\n";

  while (true) {
    int nb = fprintf(fp, msg, sizeof(msg));
    fflush(fp);
    printf("written %d\n", nb);
    sleep(1);
  }

  return 0;
}
