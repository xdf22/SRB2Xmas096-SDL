#include <stdlib.h>
#include <ctype.h>

#ifndef __EMSCRIPTEN__
int strupr(char *n) {
      int i;
      for (i=0;n[i];i++) {
	       n[i] = toupper(n[i]);
      }
      return 1;
}

int strlwr(char *n) {
      int i;
      for (i=0;n[i];i++) {
	       n[i] = tolower(n[i]);
      }
      return 1;
}
#endif
