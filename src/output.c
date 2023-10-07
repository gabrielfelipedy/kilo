#include "globconst.h"
#include "input.h"
#include "output.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int editorRowCyToRx(rowOfText* row, int cy) {
  int rx = 0;
  for(int j = 0; j < cy; j++) {
    if(row->text[j] == '\t')
      rx += (TAB_STOP - 1) - (rx % TAB_STOP);

    rx++;
  }
  return rx;
}

void editorScroll(void) {
  E.rx = 0;
  if(E.cx < E.numRows) {
    E.rx = editorRowCyToRx(&E.rows[E.cx], E.cy);
  }

  if(E.cx < E.rowOffset)
    E.rowOffset = E.cx;

  if(E.cx >= E.rowOffset + E.screenrows)
    E.rowOffset = E.cx - E.screenrows + 1;

  if(E.rx < E.colOffset)
    E.colOffset = E.rx;

  if(E.rx >= E.colOffset + E.screencols)
    E.colOffset = E.rx - E.screencols + 1;
}

void setStatusMsg(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.statusMsg, sizeof(E.statusMsg), fmt, ap);
  va_end(ap);
  E.statusMsg_time = time(NULL);
}

void drawMessageBar(struct abuf* ab) {
  abAppend(ab, "\x1b[K", 3);
  int len = strlen(E.statusMsg);
  if(len > E.screencols) len = E.screencols;
  if(len && time(NULL) - E.statusMsg_time < 5)
    abAppend(ab, E.statusMsg, len);

}

void refreshScreen(void) {
  editorScroll();

  struct abuf ab = ABUF_INIT;

  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);

  drawRows(&ab);
  drawStatusBar(&ab);
  drawMessageBar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cx - E.rowOffset) + 1, (E.rx - E.colOffset) + 1);
  
  abAppend(&ab, buf, strlen(buf));

  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.buf, ab.len);
  abFree(&ab);
}

void drawStatusBar(struct abuf* ab) {
  abAppend(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines", E.filename ? E.filename : "[No name]", E.numRows);

  int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", E.cx + 1, E.numRows);

  if(len > E.screencols) len = E.screencols;
  abAppend(ab, status, len);

  while(len < E.screencols) {
    if(E.screencols - len == rlen) {
      abAppend(ab, rstatus, rlen);
      break;
    }
    else {
      abAppend(ab, " ", 1);
      len++;
    }
  }
  abAppend(ab, "\x1b[m", 3);
  abAppend(ab, "\r\n", 2);
}

void drawRows(struct abuf* ab) {
  for(int x = 0; x < E.screenrows; x++) {

    int fileRow = x + E.rowOffset;

    if(fileRow >= E.numRows) {

      if(E.numRows == 0 && x == E.screenrows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version 1.0");

        if(welcomelen > E.screencols) welcomelen = E.screencols;
        int padding = (E.screencols - welcomelen) / 2;

        if(padding--) {
          abAppend(ab, "~", 1);
        }

        while(padding--) abAppend(ab, " ", 1);
        abAppend(ab, welcome, welcomelen);
      }
      else {
        abAppend(ab, "~", 1);
      }
    }
    else { //parte que printa o arquivo de texto
      int len = E.rows[fileRow].rsize - E.colOffset;
      if(len < 0) len = 0;
      if(len > E.screencols) len = E.screencols;
      abAppend(ab, &E.rows[fileRow].render[E.colOffset], len);
    } //até aqui

    abAppend(ab, "\x1b[K", 3); //limpa a linha de baixo
    abAppend(ab, "\r\n", 2); //quebra de linha
  }
}
