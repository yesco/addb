#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#define NAMELEN 64
#define VARCOUNT 256

char* ps= NULL;
int lineno= 1;

#define ZERO(z) memset(&z, 0, sizeof(z))

int parse(char* s) {
  ps= s;
  return 1;
}

int end() {
  return !ps || !*ps;
}

int gotc(char c) {
  if (end()) return 0;
  if (*ps != c) return 0;
  ps++;
  return c;
}

// OR
int gotcs(char* s) {
  if (end() || !s) return 0;
  char* f= strchr(s, *ps);
  if (!f) return 0;
  ps++;
  return *f;
}

int spcs() {
  int n= 0;
  while(gotcs(" \t\n\t")) n++;;
  return n;
}

// also removes starting/trailing spaces
int got(char* s) {
  if (end() || !s) return 0;
  spcs();
  char* p= ps;
  while(*p && *s && *p == *s) {
    p++; s++;
  }
  if (*s) return 0;
  ps= p;
  spcs();
  return 1;
}

int num(double* d) {
  spcs();
  int neg= gotc('-')? -1: 1;
  int i= strspn(ps, "0123456789.");
  if (!i) return 0;
  *d= atof(ps) * neg;
  ps+= i;
  spcs();
  return 1;
}

int str(char** s) {
  spcs();
  if (!gotc('"')) return 0;
  *s= ps;
  // TODO: handle \ quoted chars
  while(!end() && !gotc('"'))
    ps++;
  ps--;
  if (*ps != '"') return 0;
  *ps= 0; // terminate string
  ps++;
  return 1;
}

void error(char* msg) {
  fprintf(stdout, "Error: %s\n", msg);
  fprintf(stderr, "Error: %s\n", msg);
  // TODO: return 1 for shortcut?
  exit(1);
}

void expected(char* msg) {
  fprintf(stdout, "Error: expected %s\n", msg);
  fprintf(stderr, "Error: expected %s\n", msg);
  exit(1);
}

// app
typedef struct val {
  char* s;
  double d;
  int not_null;
} val;
  
void print_val(val* v) {
  if (!v->not_null) printf("NULL");
  else if (v->s) printf("\"%s\"", v->s);
  else printf("%.15lg", v->d);
}

char* varnames[VARCOUNT]= {0};
val* varvals[VARCOUNT]= {0};
int varcount= 0;

int linkval(char* name, val* v) {
  if (varcount>=VARCOUNT) error("out of vars");
  varnames[varcount]= name;
  varvals[varcount]= v;
  varcount++;
  return 1;
}

int getval(char name[NAMELEN], val* v) {
  // special names
  if (0==strcmp("$lineno", name)) {
    v->d= lineno;
    v->not_null= 1;
    return 1;
  }
  if (0==strcmp("$time", name)) {
    v->d = 42; // TODO: utime?
    v->not_null= 1;
    return 1;
  }
  // lookup variables
  for(int i=0; i<varcount; i++)
    if (0==strcmp(name, varnames[i])) {
      *v= *varvals[i];
      return 1;
    }
  // DO: v->not_null= 1;

  // TODO:

  // failed, null
  ZERO(*v);
  return 0;
}
	   
int getname(char name[NAMELEN]) {
  spcs();
  char* p= &name[0];
  while(!end() && (isalnum(*ps) || *ps=='$'|| *ps=='.')) {
    *p++= *ps;
    ps++;
  }
  return p!=&name[0];
}

int var(val* v) {
  char name[NAMELEN]= {};
  if (getname(name) && getval(name, v)) return 1;
  // not found == null
  ZERO(*v);
  return 1; // "NULL"
}

int expr(val* v);

int prim(val* v) {
  spcs();
  if (gotc('(')) {
    if (!expr(v)) expected("expr");
    spcs();
    if (!gotc(')')) expected("')'");
    return 1;
  }
  if (num(&v->d)) { v->not_null= 1; return 1; }
  if (str(&v->s)) return 1;
  if (var(v)) return 1;
  return 0;
}

int mult(val* v) {
  if (!prim(v)) return 0;
  char op;
  while ((op= gotcs("*/"))) {
    val vv= {};
    if (prim(&vv)) {
      // TODO: type checking?
      if (op == '*') {
	v->d= v->d * vv.d;
      } else if (op == '/') {
	v->d= v->d / vv.d;
      }
      v->not_null &= vv.not_null;
    }
  }
  return 1;
}

int expr(val* v) {
  spcs();
  if (!mult(v)) return 0;
  char op;
  while ((op= gotcs("+-"))) {
    val vv= {};
    if (mult(&vv)) {
      // TODO: type checking?
      if (op == '+') {
	v->d= v->d + vv.d;
      } else if (op == '-') {
	v->d= v->d - vv.d;
      }
      v->not_null &= vv.not_null;
    }
  }
  return 1;
}

// returns end pointer
char* print_expr_list(char* expression, int do_print) {
  char* old_ps= ps;
  ps= expression;
  
  spcs();
  val v= {};
  do {
    if (expr(&v)) {
      if (do_print) print_val(&v);
    } else expected("expression");
    if (do_print) printf("\t");
  } while(gotc(','));
  printf("\n");
  lineno++;
  
  expression= ps;
  ps= old_ps;
  return expression;
}

int where(char* expression) {
  val v= {};
  if (got("where")) {
    // TODO: test
  } else {
    v.not_null= 1;
  }
  
  if (v.not_null)
    print_expr_list(expression, 1);

  return 1;
}

int INT(char* expression) {
  char name[NAMELEN]= {};
  double start= 0, stop= 0, step= 1;
  // TODO: generalize
  // TODO: make it use expression
  if (gotc('(') && num(&start) && gotc(',')
      && num(&stop) && gotc(')')) {
    stop+= 0.5;
    spcs();
    if (!getname(name)) expected("name");
  } else return 0;

  val v= {};
  linkval(name, &v);
  int old_count= varcount;
  for(double i= start; i<stop; i+= step) {
    v.d= i;
    v.not_null= 1;

    where(expression);
  }
  varcount= old_count;
  return 1;
}

int from_list(char* expression) {
  char* start= ps;
  if (got("int")) INT(expression);
  else error("Unknown from-iterator");
  // restore parse position!
  ps= start;
  return 1;
}

int from(char* expression) {
  if (!got("from")) {
    where(expression);
    return 0;
  } else {
    from_list(expression);
    return 1;
  }
}

int select() {
  if (!got("select")) return 0;
  printf("SELECT =>\n");
  char* expression= ps;
  // "skip" (dummies)
  char* end= print_expr_list(expression, 0);
  if (end) ps= end;

  from(expression);
  return 1;
}

int sql() {
  int r= select();
  if (!r) return 0;
  return r;
}

int main(int argc, char** argv) {
  char* cmd= argv[1];
  printf("cmd=%s<\n", cmd);

  parse(cmd);
  int r= sql();
  printf("r=%d, ps=%s\n", r, ps?ps:"(NULL)");
  return 0;
}
