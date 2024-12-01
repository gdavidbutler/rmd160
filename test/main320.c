#include <stdio.h>
#include <stdlib.h>
#include "rmd320.h"

int
main(
  void
){
  rmd320_t *c;
  size_t i;
  unsigned char h[40];
  unsigned char b[80];

  if (!(c = malloc(rmd320tsize())))
    return (1);
  rmd320init(c);
  while ((i = fread(b, 1, sizeof (b), stdin)) == sizeof (b))
    rmd320update(c, b, i);
  if (i > 0)
    rmd320update(c, b, i);
  rmd320final(c, h);
  free(c);
  rmd320hex(h, (char *)b);
  printf("%.80s\n", (char *)b);
  return (0);
}
