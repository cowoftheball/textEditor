/* includes */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* data */
struct termios orig_termios;

/* terminal */
void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  // tcsetattr applies attributes to terminal
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  // tgetattr reads terminal attributes and puts them in raw
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    die("tcgetattr");
  }

  // at exit is used to automatically call function when program exits
  // in this case, we want to disable raw mode after the program exits
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  // c_lflag is for local flags
  // ICANON was added to turn off canonical mode and read byte-by-byte instead
  // of line-by-line
  // ISIG was added to turn of ctrl + C and ctrl + Z signals
  // IEXTEN was added to turn off ctrl + V
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  /* IXON was added to trun off ctrl + S and ctrl + Q
   * ICRNL was added to fix ctrl + M issues. The I stand for input flag, CR
   * stands for carriage \ return, and NL stands for new line*/
  raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);

  /* OPOST was added to fix issue with terminal translating each newline into a
   * carriage return \ plus a new line. We use OPOST to turn off all output
   * processing features. As an aside, carriage returns are used by the terminal
   * \ to return the cursor the beginning of the line on each new line*/
  raw.c_oflag &= ~(OPOST);

  raw.c_cflag |= (CS8);

  /* turned off misceallaneous characters above, including BRKINT, INPCK,
   * ISTRIP, and CS8. \
   * This aren't too important anymore, but once upon a time turning these off
   * were considered \ part of enabling raw mode, so it's tradition. */

  // sets terminal to raw mode while program is running
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

/* init */
int main() {
  enableRawMode();

  while (1) {
    char c = '\0';
    // read() and STDIN_FILENO come from <unistd.h>
    // We are asking read() to read 1 byte from the standard input into the
    // variable `c` until there are no more bytes to read. read() returns the
    // number of bytes read and returns 0 when it reaches the end of a file
    /* iscntrol tests if a character is a control character. control characters
    are non \
    printable characters such as ctrl + A. So if a character is control
    character, then \
    we print out the byte value of that character. If the character isn't a
    control character \ then we print out the byte value and then the character
    */
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
      die("read");
    }
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q')
      break;
  }
  return 0;
}
