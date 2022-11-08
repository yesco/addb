// Minimal malloc/free counter
//   (works on CLANG)

#include <stdio.h>
#include <dlfcn.h>

long nalloc= 0, nfree= 0, nbytes= 0;

void* malloc(size_t size) {
  static void* (*omalloc)(size_t)= NULL;
  if (!omalloc) omalloc= dlsym(RTLD_NEXT, "malloc");
  void* r= omalloc(size);
  if (r) {
    nalloc++;
    nbytes+= size;
    //fprintf(stderr, "malloc(%lu)\n", size);
    static long mask= -1;
    static long stored= -1;
    if (mask == -1) {
      //    0xb400007a82eb0050 = android
      mask= 0xffff000000000000;
      stored= mask & (long)r;
    } else {
      // assume it's "not changinmg")
      assert(((long)r & mask)==stored);
    }
  }
  return r;
}

void free(void* p) {
  static void (*ofree)(void*)= NULL;
  if (!ofree) ofree= dlsym(RTLD_NEXT, "free");

  if (p) nfree++;
  ofree(p);
}  

void mallocsreset() {
  nalloc= nfree= nbytes= 0;
}

void fprintmallocs(FILE* f) {
  fprintf(f, "[%ld malloc %ld free (leak %+ld) %ld bytes]\n", nalloc, nfree, nalloc-nfree, nbytes);
}

#ifdef MAIN

int main(int argc, char** argv){
  char* s= strdup("foo");
  free(s);
  char* v= malloc(42);
  fprintmallocs(stderr);
}

#endif
