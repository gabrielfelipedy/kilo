#include "globconst.h"
#include "output.h"
#include <unistd.h>

void refreshScreen(void) {
  write(STDOUT_FILENO, "\x1b[2J\x1b[H", 7);

  drawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void drawRows(void) {
  for(int x = 0; x < E.screenrows; x++) {
    write(STDOUT_FILENO, "~", 1);

    if(x < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}