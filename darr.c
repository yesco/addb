//
//   darr.c - Dynamic ARRay in C
//
// (>) CC-BY 2021 Jonas S Karlsson
//         jsk@yesco.org
//

// Dynamic array
// =============
// This is a dynamic growable managed array of dint:s (a long, or pointer!).
// It grows as need at put.
// Notice that all update functions take and return the array
// (potentially new realloced!)
//
//   dint v;
//   darr* d= NULL;
//   d= dput(d, index, value); // this reallocs!
//   v= dget(d, index);
//   printf("len=%d\n", dlen(d));
//
// 
// Interally, it keeps track allocated memory, the first index, and last index.
// This allows it to also act as a stack, and a queue:
//
//   darr* d= NULL;
//   d= dpush(d, value);
//   ...
//   v= dpop(d);
//   printf("len=%d\n", dlen(d));
//
// And as a queue:
//   d= dpush(d, value);
//   v= dshift(d, value);
// or
//   d= dunshift(d, value);
//   v= dpop(d):
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// long or pointer
typedef intptr_t dint;

#define DARR_STEP 64

typedef struct darr {
  int first, next, max;
  dint data[0];
} darr;

darr* darray(darr* d, int capacity) {
  int max= d ? d->max : 0;
  if (capacity < max) return d;
  capacity+= DARR_STEP;
  int bytes= sizeof(darr)+ capacity*sizeof(dint);
  d = realloc(d, bytes);
  // TODO: clean additional allocated
  if (!max) memset(d, 0, bytes);
  d->max = capacity;
  return d;
}

// get length of active items in D array
int dlen(darr* d) {
  return !d ? 0 : d->next - d->first;
}

// don't use directly; unsafe
dint* _dptr(darr* d, int i) {
  if (i<0) i= d->next+i; // from end
  return i<0 ? NULL : &d->data[i];
}

// in D array, put DATA at index I 
// -n means n:th from the end
// Usage: d= dput(d, 42, 4711);
darr* dput(darr* d, int i, dint data) {
  d= darray(d, i);
  *(_dptr(d, i))= data;
  if (i >= d->next) d->next= i+1;
  return d;
}

// from D array at index I get data
// -n means n:th from the end
// Usage: v= dget(d, 42); // => 4711
dint dget(darr* d, int i) {
  dint* p= _dptr(d, i);
  return p ? *p : 0;
}

// stack using: dpush, dpop
// circular with: dunshift, dshift

// WARNING: dput/dget indices are not what you expect (0th at offset d->first)

// Usage: d= dpush(d, 99);
darr* dpush(darr* d, dint data) {
  d= darray(d, d?d->next:0);
  return dput(d, d->next, data);
}

// Usage: v= dpop(d); // => 99
dint dpop(darr* d) {
  if (d->first >= d->next) return 0;
  dint* p= _dptr(d, --d->next);
  dint v= *p; *p= 0;
  return v;
}

// Usage: d= dunshift(d, 42);
darr* dunshift(darr* d, dint data) {
  if (--d->first < 0) {
    d= darray(d, d->max + DARR_STEP);
    memcpy(d->data+DARR_STEP, d->data, (d->max-DARR_STEP)*sizeof(dint));
    memset(d->data, 0, DARR_STEP*sizeof(dint));
    d->first+= DARR_STEP;
    d->next+= DARR_STEP;
  }
  return dput(d, d->first, data);
}

// Usage: v= dnshift(d) // ==> 42
dint dshift(darr* d) {
  if (d->first >= d->next) return 0;
  dint* p= _dptr(d, d->first++);
  dint v= *p; *p= 0;
  // TODO: shrink if used as circular...
  // if (d->first > 2*DARR_STEP) ...
  return v;
}

/* ENDWCOUNT */
