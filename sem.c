# include <stdio.h>
# include "cc.h"
# include "semutil.h"
# include "sem.h"
# include "sym.h"

extern int formalnum;
extern char formaltypes[];
extern int localnum;
extern char localtypes[];
extern int localwidths[];

int numlabels = 0;                      /* total labels in file */
int numblabels = 0;                     /* toal backpatch labels in file */

/*
 * backpatch - backpatch list of quadruples starting at p with k
 */
void backpatch(struct sem_rec *p, int k)
{
	//iterate through all sem_rec, andd lable to them.
	//sem_refc is a list of labels.
	//k is the label num
	struct sem_rec* iterator = p;
	while (iterator != NULL)
	{
		//bmode = lk
		printf("B%d = L%d\n", iterator->s_place, k );
		iterator = iterator->back.s_link;
	}


  // fprintf(stderr, "sem: backpatch not implemented\n");
}

/*
 * bgnstmt - encountered the beginning of a statement
 */
void bgnstmt()
{
  extern int lineno;

  printf("bgnstmt %d\n", lineno);
  //   fprintf(stderr, "sem: bgnstmt not implemented\n");
}

/*
 * call - procedure invocation
 */
struct sem_rec *call(char *f, struct sem_rec *args)
{
   //fprintf(stderr, "sem: call not implemented\n");
   //return ((struct sem_rec *) NULL);

	struct sem_rec* current = args;
	int num_arg=0;
	while(current->back.s_link != NULL)
	{
		if(current->s_mode == T_INT)
		{
			printf("argi t%d\n", current->s_place);
			num_arg++;
		}
		else if(current->s_mode == T_DOUBLE)
		{
			printf("argf t%d\n", current->s_place);
			num_arg++;
		}

		current = current->back.s_link;
	}
	//Now on last
	if(current != NULL)
	{
		if(current->s_mode == T_INT)
		{
			printf("argi t%d\n", current->s_place);
			num_arg++;
		}
		else if(current->s_mode == T_DOUBLE)
		{
			printf("argf t%d\n", current->s_place);
			num_arg++;
		}
	}
	//use lookup to find function id_entry, build sem_rec from that.
	struct id_entry* func = lookup(f,0);
	if(func != NULL)
	{
		current = node(0,0,0,0);

		current->s_place = nexttemp();

		current->s_mode = func->i_type;

	}
	else
	{
		current = node(0,0,0,0);
		current->s_place = nexttemp();
		current->s_mode = T_INT; //Default assumption.



	}

	printf("t%d := global %s\n", current->s_place, f);


	//need a print for resturring result, and return should be storing result.

	struct sem_rec* result = node(0,0,0,0);

	result->s_place = nexttemp();
	result->s_mode = current->s_mode;

	if(result->s_mode == T_INT)
	{
		//new temp num,  last temp num, and num of args
		printf("t%d := fi t%d %d\n", result->s_place, current->s_place, num_arg);
	}
	else
	{
		//Assume float
		printf("t%d := ff t%d %d\n", result->s_place, current->s_place, num_arg);
	}
	return(result);

}

/*
 * ccand - logical and
 */
struct sem_rec *ccand(struct sem_rec *e1, int m, struct sem_rec *e2)
{
	//both false lists go to given label.
	backpatch(e1->s_false, m);
	backpatch(e2->s_false, m);
}

/*
 * ccexpr - convert arithmetic expression to logical expression
 */
struct sem_rec *ccexpr(struct sem_rec *e)
{
   struct sem_rec *t1;

   if(e){

     t1 = gen("!=", e, cast(con("0"), e->s_mode), e->s_mode);

     printf("bt t%d B%d\n", t1->s_place, ++numblabels);
     printf("br B%d\n", ++numblabels);
     return (node(0, 0,
		  node(numblabels-1, 0, (struct sem_rec *) NULL,
		       (struct sem_rec *) NULL),
		  node(numblabels, 0, (struct sem_rec *) NULL,
		       (struct sem_rec *) NULL)));
   }
   else
     fprintf(stderr, "Argument sem_rec is NULL\n");
}

/*
 * ccnot - logical not
 */
struct sem_rec *ccnot(struct sem_rec *e)
{
	//Generate a new sem_rec with reverse of the list of e
	struct sem_rec* ret = node(0,0,0,0);
	ret->back.s_true = e->s_false;
	ret->s_false = e->back.s_true;
	ret->s_mode = e->s_mode;
	ret->s_place = e->s_place; //This might be wrong.
	return(ret);
	
}

/*
 * ccor - logical or
 */
struct sem_rec *ccor(struct sem_rec *e1, int m, struct sem_rec *e2)
{
	//Short circuiting, both trues go to same label.
	backpatch(e1->back.s_true, m);
	backpatch(e2->back.s_true, m);
}

/*
 * con - constant reference in an expression
 */
struct sem_rec *con(char *x)
{
  struct id_entry *p;

  if((p = lookup(x, 0)) == NULL) {
    p = install(x, 0);
    p->i_type = T_INT;
    p->i_scope = GLOBAL;
    p->i_defined = 1;
  }

  /* print the quad t%d = const */
  printf("t%d = %s\n", nexttemp(), x);

  /* construct a new node corresponding to this constant generation
     into a temporary. This will allow this temporary to be referenced
     in an expression later */
  return(node(currtemp(), p->i_type, (struct sem_rec *) NULL,
	      (struct sem_rec *) NULL));
}

/*
 * dobreak - break statement
 */
void dobreak()
{
   fprintf(stderr, "sem: dobreak not implemented\n");
}

/*
 * docontinue - continue statement
 */
void docontinue()
{
   fprintf(stderr, "sem: docontinue not implemented\n");
}

/*
 * dodo - do statement
 */
void dodo(int m1, int m2, struct sem_rec *e, int m3)
{
	backpatch(e->back.s_true, m1);
	backpatch(e->s_false, m3);
	
}

/*
 * dofor - for statement
 */
void dofor(int m1, struct sem_rec *e2, int m2, struct sem_rec *n1,
           int m3, struct sem_rec *n2, int m4)
{
	//m1 e true = goto m3, false goto m4.
	//after m3 part goto n2, then m1.

	//backpatch takes a sem_Rec and and int.
	backpatch(e2->back.s_true, m3); //if true, perform loop
	backpatch(e2->s_false, m4); //if false, skip loop.

	backpatch(n1, m1); //iterator to check
	backpatch(n2, m2); //bottom to iterator


}

/*
 * dogoto - goto statement
 */
void dogoto(char *id)
{
   fprintf(stderr, "sem: dogoto not implemented\n");
}

/*
 * doif - one-arm if statement
 */
void doif(struct sem_rec *e, int m1, int m2)
{

	 backpatch(e->back.s_true, m1); //if true enter blcok
	 backpatch(e->s_false, m2);//else skip block.
}

/*
 * doifelse - if then else statement
 */
void doifelse(struct sem_rec *e, int m1, struct sem_rec *n,
                         int m2, int m3)
{
	backpatch(n, m3);
	backpatch(e->back.s_true,m1);
	backpatch(e->s_false,m2);
	
	
   //fprintf(stderr, "sem: doifelse not implemented\n");
}

/*
 * doret - return statement
 */
void doret(struct sem_rec *e)
{
	if(e == NULL)
	{
		//IDK
		//just...generate a new temp w/ value 0, and return that?
		//int num = nexttemp();
		//printf("")
		printf("reti \n");
	}
	else
	{
		if(e->s_mode == T_INT)
		{
			printf("reti t%d\n",e->s_place);
		}
		else
		{
			//It's a float
			printf("retf t%d\n",e->s_place);

		}
	}
	//return();
    //fprintf(stderr, "sem: doret not implemented\n");
}

/*
 * dowhile - while statement
 */
void dowhile(int m1, struct sem_rec *e, int m2, struct sem_rec *n,
             int m3)
{
	backpatch(n,m1);
	backpatch(e->back.s_true, m2);
	backpatch(e->s_false,m3);
	
   //fprintf(stderr, "sem: dowhile not implemented\n");
}

/*
 * endloopscope - end the scope for a loop
 */
void endloopscope(int m)
{
   //fprintf(stderr, "sem: endloopscope not implemented\n");
	 leaveblock();
}

/*
 * exprs - form a list of expressions
 */
struct sem_rec *exprs(struct sem_rec *l, struct sem_rec *e)
{
   //fprintf(stderr, "sem: exprs not implemented\n");

   //l->back.s_link = e;

   return(merge(l,e));
   //return ((struct sem_rec *) NULL);
}

/*
 * fhead - beginning of function body
 */
void fhead(struct id_entry *p)
{	//print "func \name\ " ?
	//also does formal and localloc? Likely with a for loop or two of
	//those global numbers and the type arrays.
	//Problem: How to initalize those values?
	//Called after name and arg list is parsed, entering funciton body
	//

	int i;
	for (i = 0; i < formalnum; i++)
	{
		//Print out "formal [bytenum]"
		if(formaltypes[i] == 'i')
		{
			printf("formal %d\n", 4);
		}
		else
		{
			printf("formal %d\n", 8);
		}

	}
	for (i = 0; i < localnum; i++)
	{
		//print "localloc [bytenum]"
		if(localtypes[i] == 'i')
		{
			printf("localloc %d\n", 4);
		}
		else
		{
			printf("localloc %d\n", 8);
		}

	}

	//reset them?
	localnum = 0;
	formalnum = 0;

   //fprintf(stderr, "sem: fhead not implemented\n");
}

/*
 * fname - function declaration
 */
struct id_entry *fname(int t, char *id)
{ //t is type, id is name?
	//type of what? Return argument? Args?
	//Puts function name into symbol table, initialize formalnum and localnum
	//Returns an id_entry, which is a symbol table entry.
	printf("func %s \n", id); //TODO: IDK

	struct id_entry* p;
	if((p = lookup(id, 0)) == NULL)
	{
		//make a new thing for the function, install in table.
		//node's arguments: , , link (c), false list (d)
		//oor just do 0,0,0,0 and manually assign everything.
		//we want an id_entry
		enterblock();
		//use install.
		p = install(id,0); //makes entry with name of id* and at global level.
		p->i_type = t;
		p->i_scope = GLOBAL;
		p->i_defined = 1;

		return(p);


	}
	else
	{
		printf("Function already exists, error\n");
	}
   //fprintf(stderr, "sem: fname not implemented\n");
   //return ((struct id_entry *) NULL);
}

/*
 * ftail - end of function body
 */
void ftail()
{	//print "fend", what else?
	leaveblock();
	leaveblock();
	printf("fend\n");

   //fprintf(stderr, "sem: ftail not implemented\n");
}

/*
 * id - variable reference
 */
struct sem_rec *id(char *x)
{
   struct id_entry *p;

   if ((p = lookup(x, 0)) == NULL) {
      yyerror("undeclared identifier");
      p = install(x, -1);
      p->i_type = T_INT;
      p->i_scope = LOCAL;
      p->i_defined = 1;
   }
   if (p->i_scope == GLOBAL)
      printf("t%d := global %s\n", nexttemp(), x);
   else if (p->i_scope == LOCAL)
      printf("t%d := local %d\n", nexttemp(), p->i_offset);
   else if (p->i_scope == PARAM) {
      printf("t%d := param %d\n", nexttemp(), p->i_offset);
      if (p->i_type & T_ARRAY) {
         (void) nexttemp();
         printf("t%d := @i t%d\n", currtemp(), currtemp()-1);
      }
   }

   /* add the T_ADDR to know that it is still an address */
   return (node(currtemp(), p->i_type|T_ADDR, (struct sem_rec *) NULL,
                (struct sem_rec *) NULL));
}

/*
 * index - subscript
 */
struct sem_rec *tom_index(struct sem_rec *x, struct sem_rec *i)
{
  return (gen("[]", x, cast(i, T_INT), x->s_mode&~(T_ARRAY)));
}

/*
 * labeldcl - process a label declaration
 */
void labeldcl(char *id)
{
   fprintf(stderr, "sem: labeldcl not implemented\n");
}

/*
 * m - generate label and return next temporary number
 */
int m()
{
   //fprintf(stderr, "sem: m not implemented\n");
   numlabels++;
   printf("label L%d\n", numlabels);

   return (numlabels);
}

/*
 * n - generate goto and return backpatch pointer
 */
struct sem_rec *n()
{
	//print a branch lable
	//create a sem_rec with s_mode of same value
	int dest = numblabels + 1;
	numblabels++;

	printf("br B%d\n", dest );

	struct sem_rec* ret_rec = node(0,0,0,0);
	ret_rec->s_place = dest;

	return(ret_rec);


   //fprintf(stderr, "sem: n not implemented\n");
   //return ((struct sem_rec *) NULL);
}

/*
 * op1 - unary operators
 */
struct sem_rec *op1(char *op, struct sem_rec *y)
{
  if (*op == '@' && !(y->s_mode&T_ARRAY)){
    /* get rid of T_ADDR if it is being dereferenced so can handle
       T_DOUBLE types correctly */
    y->s_mode &= ~T_ADDR;
    return (gen(op, (struct sem_rec *) NULL, y, y->s_mode));
  }
  else{
    //fprintf(stderr, "sem: op1 not implemented\n");

	//struct sem_rec* result = node(0,0,0,0);
	//result->s_place = nexttemp();

    return (gen(op, (struct sem_rec *) NULL, y, y->s_mode) );
  }
}

/*
 * op2 - arithmetic operators
 */
struct sem_rec *op2(char *op, struct sem_rec *x, struct sem_rec *y)
{
	if(x == NULL || y == NULL || op == '\0')
	{
		printf("Error, cannot perform operation.\n");
		return((struct sem_rec*) NULL);
	}
	if(x->s_mode != y->s_mode)
	{
		printf("Incompatible types\n");
		return((struct sem_rec*) NULL);
	}
	struct sem_rec* p = gen(op, x, y, x->s_mode);

	return(p);
   //fprintf(stderr, "sem: op2 not implemented\n");
   return ((struct sem_rec *) NULL);
}

/*
 * opb - bitwise operators
 */
struct sem_rec *opb(char *op, struct sem_rec *x, struct sem_rec *y)
{
	//Generate the operation.
	//Make sure they're the same type, if so gen op.
	
	if(x->s_mode == y->s_mode)
	{
		struct sem_rec* t = gen(op,x,y,x->s_mode);
		return(t);
	}
	else
	{
		printf("Bitwise operation failed: type mismatch.\n");
		return((struct sem_rec*) NULL);
	}
	
	//Returns a sem_rec pointer.
}

/*
 * rel - relational operators
 */
struct sem_rec *rel(char *op, struct sem_rec *x, struct sem_rec *y)
{
   //fprintf(stderr, "sem: rel not implemented\n");
   //need to generate branching statement.
   //rel happens at if i == 0
   //generate a temp storing that resutl,
   //a branch if true, and a new backpachting lable,
   //andother branch if false rightr after.

   //assign those labels to a true list and a false lsit.
   //make stuff w/ node.
   if(x->s_mode != y->s_mode)
	 {
		 struct sem_rec* temp = node(0,0,0,0);
		 struct sem_rec* comparer;
		 if(x->s_mode == T_INT)
		 {
			 //convert x to float
			 //generate a sem_rec for that conversion

			 int new_var = nexttemp();
			 temp->s_place = new_var;
			 temp->s_mode = T_DOUBLE;

			 printf("t%d := cvf t%d\n", temp->s_place , x->s_place );
			 comparer = gen(op,temp,y,temp->s_mode);
		 }
		 else
		 {
			 //y is an int, convert to float.
			 int new_var = nexttemp();
			 temp->s_place = new_var;
			 temp->s_mode = T_DOUBLE;

			 printf("t%d := cvf t%d\n", temp->s_place , y->s_place );
			 comparer = gen(op,x,temp,x->s_mode);
		 }
		 struct sem_rec* t_l = node(0,0,0,0); //true list.
	   struct sem_rec* f_l = node(0,0,0,0); //false list

	   int tr = numblabels + 1;
	   numblabels++;
	   int fl = numblabels + 1;
	   numblabels++;

	   printf("bt t%d B%d\n", comparer->s_place, tr );//branch if true.
	   printf("br B%d\n", fl); //branch if false.

	   t_l->s_place = tr;
	   f_l->s_place = fl;

	   comparer->back.s_true = t_l;
	   comparer->s_false = f_l;

	   return(comparer);

	 }
	 else
	 {
	   struct sem_rec* t_l = node(0,0,0,0); //true list.
	   struct sem_rec* f_l = node(0,0,0,0); //false list
	   struct sem_rec* comparer = gen(op,x,y,x->s_mode);
	   int tr = numblabels + 1;
	   numblabels++;
	   int fl = numblabels + 1;
	   numblabels++;

	   printf("bt t%d B%d\n", comparer->s_place, tr );//branch if true.
	   printf("br B%d\n", fl); //branch if false.

	   t_l->s_place = tr;
	   f_l->s_place = fl;

	   comparer->back.s_true = t_l;
	   comparer->s_false = f_l;

	   return(comparer);
 }
   //return ((struct sem_rec *) NULL);
}

/*
 * set - assignment operators
 */
struct sem_rec *set(char *op, struct sem_rec *x, struct sem_rec *y)
{
  /* assign the value of expression y to the lval x */
  struct sem_rec *p, *cast_y;

  if(*op!='\0' || x==NULL || y==NULL){
	  //this is an op, but x or y doesn't exist.
	  //t16 and t17 in example 1.
	  //this is like i += 1 or w/e.
	  //use gen()?
	  //first sem_rec arg NULL, second as assignment?

		if(y->s_mode == T_DOUBLE)
		{
			//generate quad for grapping value of i from x,
			//in i += 1, y is the 1.
			//when genning first quad, stor it temp for usage in 2nd quad.
			struct sem_rec* temp1 = gen("@", (struct sem_rec*) NULL, x, T_DOUBLE);
			//temp 1 now holds the variable to be incremented.
			struct sem_rec* throwaway = gen(op, temp1, y, T_DOUBLE);
			y = throwaway;
		}
		else
		{
			struct sem_rec* temp1 = gen("@", (struct sem_rec*) NULL, x, T_INT);
			struct sem_rec* throwaway = gen(op, temp1, y, T_INT);
			y = throwaway;
		}

		if(x == NULL)
	  {
		  p = gen(op, NULL, y, y->s_mode);
	  }
	  else if(y == NULL)
	  {
		  p = gen(op, NULL, x, x->s_mode);
	  }
    //fprintf(stderr, "sem: set not implemented\n");
    //return(p);
  }

  /* if for type consistency of x and y */
  cast_y = y;
  if((x->s_mode & T_DOUBLE) && !(y->s_mode & T_DOUBLE)){

    /*cast y to a double*/
    printf("t%d = cvf t%d\n", nexttemp(), y->s_place);
    cast_y = node(currtemp(), T_DOUBLE, (struct sem_rec *) NULL,
		  (struct sem_rec *) NULL);
  }
  else if((x->s_mode & T_INT) && !(y->s_mode & T_INT)){

    /*convert y to integer*/
    printf("t%d = cvi t%d\n", nexttemp(), y->s_place);
    cast_y = node(currtemp(), T_INT, (struct sem_rec *) NULL,
		  (struct sem_rec *) NULL);
  }

  /*output quad for assignment*/
  if(x->s_mode & T_DOUBLE)
    printf("t%d := t%d =f t%d\n", nexttemp(),
	   x->s_place, cast_y->s_place);
  else
    printf("t%d := t%d =i t%d\n", nexttemp(),
	   x->s_place, cast_y->s_place);

  /*create a new node to allow just created temporary to be referenced later */
  return(node(currtemp(), (x->s_mode&~(T_ARRAY)),
	      (struct sem_rec *)NULL, (struct sem_rec *)NULL));
}

/*
 * startloopscope - start the scope for a loop
 */
void startloopscope()
{
		enterblock(); //Maybe?
		//Prep for a new scope.
}

/*
 * string - generate code for a string
 */
struct sem_rec *string(char *s)
{
   //fprintf(stderr, "sem: string not implemented\n");

   //node(0,0,0,0) generates a sem_rec we can fill ourselves.
   struct sem_rec* my_srec = node(0,0,0,0);

   my_srec->s_place = nexttemp();
   my_srec->s_mode = T_STR;

   printf("t%d := %s\n", my_srec->s_place, s);
   return (my_srec);
}



/************* Helper Functions **************/

/*
 * cast - force conversion of datum y to type t
 */
struct sem_rec *cast(struct sem_rec *y, int t)
{
   if (t == T_DOUBLE && y->s_mode != T_DOUBLE)
      return (gen("cv", (struct sem_rec *) NULL, y, t));
   else if (t != T_DOUBLE && y->s_mode == T_DOUBLE)
      return (gen("cv", (struct sem_rec *) NULL, y, t));
   else
      return (y);
}

/*
 * gen - generate and return quadruple "z := x op y"
 */
struct sem_rec *gen(char *op, struct sem_rec *x, struct sem_rec *y, int t)
{
   if (strncmp(op, "arg", 3) != 0 && strncmp(op, "ret", 3) != 0)
      printf("t%d := ", nexttemp());
   if (x != NULL && *op != 'f')
      printf("t%d ", x->s_place);
   printf("%s", op);
   if (t & T_DOUBLE && (!(t & T_ADDR) || (*op == '[' && *(op+1) == ']'))) {
      printf("f");
      if (*op == '%')
         yyerror("cannot %% floating-point values");
   }
   else
      printf("i");
   if (x != NULL && *op == 'f')
      printf(" t%d %d", x->s_place, y->s_place);
   else if (y != NULL)
      printf(" t%d", y->s_place);
   printf("\n");
   return (node(currtemp(), t, (struct sem_rec *) NULL,
           (struct sem_rec *) NULL));
}
