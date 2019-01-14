/**
 * @author ktwice@mail.ru
 *
 * The program cuts the contents of the log file by year into separate files.
 * Log-file name is specified in the first parameter (default: access_log).
 * Files are created next to log-file.
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
  unsigned long long parts;
  unsigned long long lines;
  unsigned long long empties;
  unsigned long long bytes;
  unsigned long long seconds;
  unsigned long long tails;
} CNT_T;

int lineHeadYear(const char* s) {
  if((s = strchr(s, '[')) == NULL) return 0; 
  if((s = strchr(++s, '/')) == NULL) return 0; 
  if((s = strchr(++s, '/')) == NULL) return 0; 
  return atoi(++s);
}

FILE* open2(char* fname, char* mode) {
  FILE* f = fopen(fname, mode);
  if(f != NULL) return f;
  perror(fname);
  exit(2);
}

int readBytes(char* buf, FILE* f) {
  if(fgets(buf, BUF_SIZE, f) != NULL) return strlen(buf);
  if(feof(f)) return 0;
  perror("fgets()");
  exit(2);
}

int writeLine(char* buf, int bytes, FILE* fout) {
  if(fwrite(buf, bytes, 1, fout) != -1) return(buf[bytes-1] == '\n');
  perror("fwrite()");
  exit(2);
}

void close2(FILE* fout, unsigned long long lines) {
  fclose(fout);
  printf(" %llu lines.", lines);
}

int main(int argc, char* argv[]) {
  time_t t = time(NULL);
  CNT_T reads, writes;
  int bytes, y, yout = 0;
  char buf[BUF_SIZE], fnameout[NBUF_SIZE];
  char* fname = argc>1 ? argv[1] : "access_log";
  FILE* fout = NULL;
  FILE* f = open2(fname, "r");
  memset(&reads, 0, sizeof(reads));
  printf("\nRead from <%s>", fname);
  while((bytes = readBytes(buf, f)) != 0) {
    reads.bytes+=bytes;
    reads.lines++;
    if(bytes == 1) {
      reads.empties++;
//      printf("\n%llu: Empty line ignored.", reads.lines);
      continue;
    }
    y = lineHeadYear(buf);
    if(y==0) {
      printf("\n%llu: Year not found in line.", reads.lines);
      exit(2);
    }
    if(y != yout) {
      if(fout != NULL) {
        close2(fout, writes.lines);
        if(++yout != y) {
printf("\n%llu: Next Year must be %d not %d.", reads.lines, yout, y);
          exit(2);
        }
      } else yout = y;
      sprintf(fnameout, "%s.%d", fname, y);
      fout = open2(fnameout, "w");
      reads.parts++;
      memset(&writes, 0, sizeof(writes));
      printf("\nWrite to <%s>", fnameout);
    }
    while(!writeLine(buf, bytes, fout)) {
      if(!(bytes = readBytes(buf, f))) {
        close2(fout, writes.lines);
        fout = NULL;
        printf("\nWARNING: Last line without LineFeed.");
        break;
      }
      reads.bytes+=bytes;
      reads.tails++;
    }
    writes.lines++;
  }
  if(fout != NULL) close2(fout, writes.lines);
  fclose(f);
  
  reads.seconds = (unsigned long long)round(difftime(time(NULL), t));
  if(reads.seconds == 0) reads.seconds = 1;
  printf("\n<%s> SUCCESSFULLY split %llu lines into %llu files per %llu seconds"
    , fname, reads.lines, reads.parts, reads.seconds);
  if(reads.empties > 0) printf("\nWARNING: %llu empty line(s) ignored"
    , reads.empties);
//  printf("\n%llu tails(over %d bytes) readed", reads.tails, BUF_SIZE);
//  printf("\n%llu bytes total read at %llu lines/second"
//    , reads.bytes, (unsigned long long)round(reads.lines / reads.seconds));
  printf("\n");
  return 0;
}
