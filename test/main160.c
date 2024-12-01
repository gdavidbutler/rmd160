#include <stdio.h>
#include <stdlib.h>
#include "rmd160.h"

int
main(
  void
){
  rmd160_t *c;
  size_t i;
  unsigned char h[20];
  unsigned char b[40];

  if (!(c = malloc(rmd160tsize())))
    return (1);
  rmd160init(c);
  while ((i = fread(b, 1, sizeof (b), stdin)) == sizeof (b))
    rmd160update(c, b, i);
  if (i > 0)
    rmd160update(c, b, i);
  rmd160final(c, h);
  free(c);
  rmd160hex(h, (char *)b);
  printf("%.40s\n", (char *)b);
  return (0);
}
