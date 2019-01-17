/**
 * @author ktwice@mail.ru
 *
 * The program copies the contents of the log-file into separate files by year.
 * The log-file name is specified in the first parameter (default: access_log).
 * Files are created next to the log-file.
 * Files are named with postfix: Year-after-dot.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define BUF_SIZE 1024
#define NBUF_SIZE 1024

typedef struct {
  unsigned long long parts, lines, empties, bytes, seconds, tails;
} CNT_T;
CNT_T reads, writes;

int getYear(const char* s) {
  int y;
  if((s = strchr(s, '[')) != NULL)
    if((s = strchr(++s, '/')) != NULL)
      if((s = strchr(++s, '/')) != NULL) 
        if((y = atoi(++s)) != 0)
          return y;
  printf("\n%llu: Year not found in the line.", reads.lines);
  exit(2);
}

FILE* openFile(char* fname, char* mode, CNT_T* p, char* sf) {
  FILE* f = fopen(fname, mode);
  if(f != NULL) {
    printf(sf, fname);
    memset(p, 0, sizeof(CNT_T));
    return f;
  }
  perror(fname);
  exit(2);
}

int readBytes(char* buf, FILE* f) {
  int bytes;
  if(fgets(buf, BUF_SIZE, f) != NULL) {
    reads.bytes += (bytes = strlen(buf));
    return bytes;
  }
  if(!ferror(f)) return 0;
  perror("fgets()");
  exit(2);
}

int writeLine(char* buf, int bytes, FILE* fout) {
  writes.bytes += bytes;
  if(fwrite(buf, bytes, 1, fout) != -1) return(buf[bytes-1] == '\n');
  perror("fwrite()");
  exit(2);
}

void closeFile(FILE* fout) {
  fclose(fout);
  printf(" %llu lines (%llu bytes).", writes.lines, writes.bytes);
}

int main(int argc, char* argv[]) {
  time_t t = time(NULL);
  int bytes, y, yout = 0;
  char buf[BUF_SIZE], fnameout[NBUF_SIZE];
  char* fname = argc>1 ? argv[1] : "access_log";
  FILE* fout = NULL;
  FILE* f = openFile(fname, "r", &reads, "\nRead from <%s>");
  while((bytes = readBytes(buf, f)) != 0) {
    reads.lines++;
    if(bytes == 1) {
      reads.empties++;
//      printf("\n%llu: Empty line ignored.", reads.lines);
      continue;
    }
    if((y = getYear(buf)) != yout) {
      if(fout != NULL) {
        closeFile(fout);
        if(++yout != y) {
          printf("\n%llu: Next Year must be %d not %d.", reads.lines, yout, y);
          exit(2);
        }
      } else yout = y;
      sprintf(fnameout, "%s.%d", fname, y);
      fout = openFile(fnameout, "w", &writes, "\nWrite to <%s>");
      reads.parts++;
    }
    writes.lines++;
    while(!writeLine(buf, bytes, fout)) {
      reads.tails++;
      if((bytes = readBytes(buf, f)) != 0) continue;
      printf("\nWARNING: There is no LineFeed in the last line.");
      break;
    }
  }
  if(fout != NULL) closeFile(fout);
  fclose(f);
  
  reads.seconds = (unsigned long long)round(difftime(time(NULL), t));
  if(reads.seconds == 0) reads.seconds = 1;
  printf("\nSUCCESSFULLY distributed %llu lines into %llu files per %llu seconds"
    , reads.lines, reads.parts, reads.seconds);
  if(reads.empties > 0) printf("\nWARNING: %llu empty line(s) ignored"
    , reads.empties);
//  printf("\n%llu tails(over %d bytes) readed", reads.tails, BUF_SIZE);
//  printf("\n%llu bytes total read at %llu lines/second"
//    , reads.bytes, (unsigned long long)round(reads.lines / reads.seconds));
  printf("\n");
  
  return 0;
}
