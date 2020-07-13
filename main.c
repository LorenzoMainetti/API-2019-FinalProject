#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************
    DATA STRUCTURES && GLOBAL VARIABLES
 *****************************************/

// 1) tree node for a RB-tree of entities
struct treenode1 {
 char *data;                 /*key (entity/origin)*/
 char color;                 /*color*/
 struct treenode1 *p;        /*pointer to father*/
 struct treenode1 *left;     /*pointer to left subtree*/
 struct treenode1 *right;    /*pointer to right subtree*/
};
typedef struct treenode1 treeNode1;


// 2) tree node for a RB-tree of recipients
struct treenode2 {
 char *data;                 /*key (dest)*/
 char color;                 /*color*/
 int counter;                /*number of relations*/
 treeNode1 *origin;          /*pointer to the root of the origins tree*/
 struct treenode2 *p;        /*pointer to father*/
 struct treenode2 *left;     /*pointer to left subtree*/
 struct treenode2 *right;    /*pointer to right subtree*/
};
typedef struct treenode2 treeNode2;


// 3) list node for a list of relations
struct listNode {
 char *rel_name;             /*key (type of relation)*/
 int max;                    /*max number of relations received*/
 treeNode1 *report;          /*pointer to the root of the report tree*/
 treeNode2 *dest;            /*pointer to the root of the recipients tree*/
 struct listNode *next;      /*pointer to next node*/
};
typedef struct listNode list;


// 4) list node for a buffer
struct listNode2 {
 char *data;                 /*key*/
 struct listNode2 *next;     /*pointer to next node*/
};
typedef struct listNode2 list2;


// NIL and NUL nodes for RB-trees
#define NIL (&sentinel)     
treeNode1 sentinel = {NULL, 'B', NIL, NIL, NIL};

treeNode1 *T = NIL;   /*root, global variable*/

#define NUL (&sentinella)   
treeNode2 sentinella = {NULL, 'B', 0, NIL, NUL, NUL, NUL}; 


list *L = NULL;          /*initialize list*/
list2 *buffer = NULL;    /*initialize buffer*/   


// function prototypes for the commands:
void Addent(char name[]);
void Delent(char name[]);
void Addrel(char origin[], char dest[], char type[]);
void Delrel(char origin[], char dest[], char type[]);
void Report();

// RB-tree function prototypes:
void left_rotate(treeNode1 **T, treeNode1 *x);
void right_rotate(treeNode1 **T, treeNode1 *y);
void RB_insert(treeNode1 **T, char el[]);
void RB_insert_fixup(treeNode1 **T, treeNode1 *z);
void RB_delete(treeNode1 **T, treeNode1 *z);
void RB_delete_fixup(treeNode1 **T, treeNode1 *x);
int RB_search(treeNode1 *x, char el[]);
void RB_clean(treeNode1 *x);
treeNode1 *RB_find_node(treeNode1 *x, char el[]);
treeNode1 *RB_successor(treeNode1 *x);
treeNode1 *RB_minimum(treeNode1 *x);

void left_rotate2(treeNode2 **T, treeNode2 *x);
void right_rotate2(treeNode2 **T, treeNode2 *y);
void RB_insert2(treeNode2 **T, char el[]);
void RB_insert_fixup2(treeNode2 **T, treeNode2 *z);
void RB_delete2(treeNode2 **T, treeNode2 *z);
void RB_delete_fixup2(treeNode2 **T, treeNode2 *x);
int RB_search2(treeNode2 *x, char el[]);
treeNode2 *RB_find_node2(treeNode2 *x, char el[]);
treeNode2 *RB_successor2(treeNode2 *x);
treeNode2 *RB_minimum2(treeNode2 *x);

void origin_delete(treeNode2 *x, list *l, char el[]);
void insert_max(treeNode2 *x, list *l);

// function prototypes to handle the report command
int find_max(treeNode2 *x);
void print_max(treeNode1 *x);

// list function prototypes:
void insert(list **L, char el[]);
void delete(list **L, char el[]);
int search(list *L, char el[]);

void insert2(char el[]);
void delete2();


/***************************************
           COMMANDS FUNCTIONS
 ***************************************/

void Addent(char name[]) {   
 if(RB_search(T, name) == 1)        
   return; 
  else   
     RB_insert(&T, name); 
}

void Delent(char name[]) {
 if(RB_search(T, name) == 0)
   return;
  else { 
    treeNode1 *s = RB_find_node(T, name);
    RB_delete(&T, s);

    list *curr = L;
    char *type;

    while (curr != NULL) {
      treeNode2 *temp = RB_find_node2(curr->dest, name);
      if(temp != NUL) {                
        temp->counter = 0;
        RB_clean(temp->origin);   
        temp->origin = NIL;             
       } 
     
      origin_delete(curr->dest, curr, name);
            
      while (buffer != NULL) { 
        treeNode2 *t = RB_find_node2(curr->dest, buffer->data);
        treeNode1 *n = RB_find_node(curr->report, buffer->data); 
        RB_delete2(&(curr->dest), t);  
        RB_delete(&(curr->report), n);         
        delete2();
       }        

      if(curr->dest == NUL) {
       type = curr->rel_name;
       curr = curr->next;
       delete(&L, type);
      }   
      else {
        int i = find_max(curr->dest); 
        if(i < curr->max) {          //max changes
           curr->max = i;
           RB_clean(curr->report);
           curr->report = NIL;
           insert_max(curr->dest, curr);
          }  
        curr = curr->next;
       }
     }          
  }
}

void Addrel(char origin[], char dest[], char type[]) {
 if(RB_search(T, origin) == 0 || RB_search(T, dest) == 0) 
   return;

 if(search(L, type) == 0)
   insert(&L, type);
 list *curr = L;
 while (strcmp(curr->rel_name, type) != 0)
   curr = curr->next;

 if(RB_search2(curr->dest, dest) == 0)       //could not find the given dest
   RB_insert2(&(curr->dest), dest);
 treeNode2 *temp = RB_find_node2(curr->dest, dest);

 if(RB_search(temp->origin, origin) == 0) {  //could not find the given origin
   temp->counter++;
   RB_insert(&(temp->origin), origin);    

   if(temp->counter > curr->max) {      //max changes
    curr->max = temp->counter; 
    RB_clean(curr->report);
    curr->report = NIL;
    RB_insert(&(curr->report), dest);
   }
   else if(temp->counter == curr->max)  //max does not change
     RB_insert(&(curr->report), dest);
  }
  else
    return;                                           
}

void Delrel(char origin[], char dest[], char type[]) {
 if(RB_search(T, origin) == 0 || RB_search(T, dest) == 0)
   return;
 
 if(search(L, type) == 1) {
   list *curr = L;
   while (strcmp(curr->rel_name, type) != 0)  
     curr = curr->next; 

   treeNode2 *temp = RB_find_node2(curr->dest, dest);
   if(temp != NUL) { 
     treeNode1 *t = RB_find_node(temp->origin, origin);   
     if(t != NIL) {
       temp->counter--;

       if(temp->counter < curr->max) {       
         int i = find_max(curr->dest);
         if(i < curr->max) {           //max changes
           curr->max = i;
           RB_clean(curr->report);
           curr->report = NIL;
           insert_max(curr->dest, curr);
          }
          else if (i == curr->max) {   //max does not change
            treeNode1 *v = RB_find_node(curr->report, dest);
            RB_delete(&(curr->report), v);      
           } 
         }

       RB_delete(&(temp->origin), t);      
       if(temp->counter == 0)         
         RB_delete2(&(curr->dest), temp);                 
      }
     if(curr->dest == NUL)
       delete(&L, type); 
   }
   else
     return;
 }
  else
    return;
}

void Report() {   
 list *curr = L;
 
 if(curr == NULL)
   fputs_unlocked("none", stdout);
 else {
   while (curr != NULL) {
     fputs_unlocked(curr->rel_name, stdout);
     fputs_unlocked(" ", stdout);    
     print_max(curr->report);       
     printf("%d", curr->max);
     fputs_unlocked("; ", stdout);
     curr = curr->next;
    }
  }   
 fputs_unlocked("\n", stdout);
}


/***************************************
        RED-BLACK TREES FUNCTIONS
 ***************************************/

void left_rotate(treeNode1 **T, treeNode1 *x) {
 treeNode1 *y = x->right;
 x->right = y->left;

 if (y->left != NIL)
   y->left->p = x;
 
   y->p = x->p;

 if (x->p == NIL)
   (*T) = y;
   else {
       if (x == x->p->left)
         x->p->left = y;
        else
           x->p->right = y;
       }
 y->left = x;
 x->p = y;
}

void right_rotate(treeNode1 **T, treeNode1 *y) {
 treeNode1 *x = y->left;
 y->left = x->right;
 
 if (x->right != NIL)
   x->right->p = y;
 
 x->p = y->p;

 if (y->p == NIL)
   (*T) = x;
   else {
       if (y == y->p->left)
         y->p->left = x;
        else
           y->p->right = x;
       }
 x->right = y;
 y->p = x;
}

void RB_insert(treeNode1 **T, char el[]) {
 treeNode1 *z = (treeNode1*)malloc(sizeof(treeNode1));
 z->data = malloc((strlen(el)+1)*sizeof(char));
 strcpy(z->data, el);
 z->left = NIL;
 z->right = NIL;
 z->p = NIL;

 if((*T) == NIL) {                          
   z->color = 'B';                          
   (*T) = z;
  }
  else {
     treeNode1 *y = NIL;
     treeNode1 *x = (*T);
     while (x != NIL) {
       y = x;
       if(strcmp(z->data, x->data) < 0)
          x = x->left;
        else 
           x = x->right;     
     }
     z->p = y;
     if(strcmp(z->data, y->data) > 0)
       y->right = z;
      else
        y->left = z;
     z->color = 'R';

     RB_insert_fixup(T, z);
    }
}

void RB_insert_fixup(treeNode1 **T, treeNode1 *z) {
 while (z != (*T) && z->p->color == 'R') {
 
   if(z->p == z->p->p->left) {
      treeNode1 *y = z->p->p->right;
      if(y->color == 'R') {
         z->p->color = 'B';
         y->color = 'B';
         z->p->p->color = 'R';
         z = z->p->p;
        }
        else {
              if(z == z->p->right) {
                 z = z->p;
                 left_rotate(T, z);
                }
              z->p->color = 'B';
              z->p->p->color = 'R';
              right_rotate(T, z->p->p);
             }
   }      
    else {
          treeNode1 *y = z->p->p->left;
          if(y->color == 'R') {
             z->p->color = 'B';
             y->color = 'B';
             z->p->p->color = 'R';
             z = z->p->p;
            }
            else {
                  if(z == z->p->left) {
                     z = z->p;
                     right_rotate(T, z);
                    }
                  z->p->color = 'B';
                  z->p->p->color = 'R';
                  left_rotate(T, z->p->p);
                 }
         }  
 }
 (*T)->color = 'B';
} 

void RB_delete(treeNode1 **T, treeNode1 *z) {
 treeNode1 *x, *y;
 
 if(z == NIL)
   return;
 if(z->left == NIL || z->right == NIL)
   y = z; 
  else
    y = RB_successor(z);
 
 if(y->left != NIL)
   x = y-> left;
  else
    x = y->right;
 
 x->p = y->p;
 if(y->p == NIL)
   (*T) = x;
  else {
     if(y == y->p->left)
        y->p->left = x;
      else
         y->p->right = x;
     }
 if(y != z) {
   if(strlen(z->data) < strlen(y->data))
     z->data = realloc(z->data, strlen(y->data)+1);
   strcpy(z->data, y->data);
  }
 if(y->color == 'B')
   RB_delete_fixup(T, x);
 free(y->data);
 free(y);
}

void RB_delete_fixup(treeNode1 **T, treeNode1 *x) {
 treeNode1 *w;
 while (x != (*T) && x->color == 'B') {
   if(x == x->p->left) {
      w = x->p->right;
      if(w->color == 'R') {
         w->color = 'B';
         x->p->color = 'R';
         left_rotate(T, x->p);
         w = x->p->right;
        }
      if(w->left->color == 'B' && w->right->color == 'B') {
         w->color = 'R';
         x = x->p;
        }
        else {
              if(w->right->color == 'B') {
                 w->left->color = 'B';
                 w->color = 'R';
                 right_rotate(T, w);
                 w = x->p->right;
                }
              w->color = x->p->color;
              x->p->color = 'B';
              w->right->color = 'B';
              left_rotate(T, x->p);
              x = (*T);
             }
     }
     else {
        w = x->p->left;
        if(w->color == 'R') {
           w->color = 'B';
           x->p->color = 'R';
           right_rotate(T, x->p);
           w = x->p->left;
          }
        if(w->right->color == 'B' && w->left->color == 'B') {
           w->color = 'R';
           x = x->p;
          }
          else {
                if(w->left->color == 'B') {
                   w->right->color = 'B';
                   w->color = 'R';
                   left_rotate(T, w);
                   w = x->p->left;
                  }
                w->color = x->p->color;
                x->p->color = 'B';
                w->left->color = 'B';
                right_rotate(T, x->p);
                x = (*T);
               }
       }
 }
 x->color = 'B';
}

int RB_search(treeNode1 *x, char el[]) {
 if(x == NIL)
   return 0;
 if(strcmp(el, x->data) == 0) 
   return 1; 
  else if (strcmp(el, x->data) < 0)
    return RB_search(x->left, el);
   else
     return RB_search(x->right, el);
 return 0;
} 

treeNode1 *RB_find_node(treeNode1 *x, char el[]) {
 if(x == NIL)
   return x;

 if(strcmp(el, x->data) == 0)
   return x;
  else if(strcmp(el, x->data) < 0)
      return RB_find_node(x->left, el);
     else
       return RB_find_node(x->right, el);
 return NIL;
}      

treeNode1 *RB_successor(treeNode1 *x) {
 if(x->right != NIL)
   return RB_minimum(x->right);
 treeNode1 *y = x->p;
 while (y != NIL && x == y->right) {
   x = y;
   y = y->p;
  }
 return y;
}

treeNode1 *RB_minimum(treeNode1 *x) {
 while (x->left != NIL)
   x = x->left;
 return x;
}

void RB_clean(treeNode1 *x) {
 treeNode1 *temp = x;
 if(temp == NIL)
   return;

 RB_clean(temp->left);
 RB_clean(temp->right);

 free(temp->data);
 free(temp);
}



void left_rotate2(treeNode2 **T, treeNode2 *x) {
 treeNode2 *y = x->right;
 x->right = y->left;

 if (y->left != NUL)
   y->left->p = x;
 
   y->p = x->p;

 if (x->p == NUL)
   (*T) = y;
   else {
       if (x == x->p->left)
         x->p->left = y;
        else
           x->p->right = y;
       }
 y->left = x;
 x->p = y;
}

void right_rotate2(treeNode2 **T, treeNode2 *y) {
 treeNode2 *x = y->left;
 y->left = x->right;
 
 if (x->right != NUL)
   x->right->p = y;
 
 x->p = y->p;

 if (y->p == NUL)
   (*T) = x;
   else {
       if (y == y->p->left)
         y->p->left = x;
        else
           y->p->right = x;
       }
 x->right = y;
 y->p = x;
}

void RB_insert2(treeNode2 **T, char el[]) {
 treeNode2 *z = (treeNode2*)malloc(sizeof(treeNode2));
 z->data = malloc((strlen(el)+1)*sizeof(char));
 strcpy(z->data, el);
 z->left = NUL;
 z->right = NUL;
 z->p = NUL;
 z->origin = NIL;
 z->counter = 0;

 if((*T) == NUL) {                          
   z->color = 'B';                          
   (*T) = z;
  }
  else {
     treeNode2 *y = NUL;
     treeNode2 *x = (*T);
     while (x != NUL) {
       y = x;
       if(strcmp(z->data, x->data) < 0)
          x = x->left;
        else 
           x = x->right;     
     }
     z->p = y;
     if(strcmp(z->data, y->data) > 0)
       y->right = z;
      else
        y->left = z;
     z->color = 'R';

     RB_insert_fixup2(T, z);
    }
}

void RB_insert_fixup2(treeNode2 **T, treeNode2 *z) {
 while (z != (*T) && z->p->color == 'R') {
 
   if(z->p == z->p->p->left) {
      treeNode2 *y = z->p->p->right;
      if(y->color == 'R') {
         z->p->color = 'B';
         y->color = 'B';
         z->p->p->color = 'R';
         z = z->p->p;
        }
        else {
              if(z == z->p->right) {
                 z = z->p;
                 left_rotate2(T, z);
                }
              z->p->color = 'B';
              z->p->p->color = 'R';
              right_rotate2(T, z->p->p);
             }
   }      
    else {
          treeNode2 *y = z->p->p->left;
          if(y->color == 'R') {
             z->p->color = 'B';
             y->color = 'B';
             z->p->p->color = 'R';
             z = z->p->p;
            }
            else {
                  if(z == z->p->left) {
                     z = z->p;
                     right_rotate2(T, z);
                    }
                  z->p->color = 'B';
                  z->p->p->color = 'R';
                  left_rotate2(T, z->p->p);
                 }
         }  
 }
 (*T)->color = 'B';
} 

void RB_delete2(treeNode2 **T, treeNode2 *z) {
 treeNode2 *x, *y;
 
 if(z == NUL)
   return;
 if(z->left == NUL || z->right == NUL)
   y = z; 
  else
    y = RB_successor2(z);
 
 if(y->left != NUL)
   x = y-> left;
  else
    x = y->right;
 
 x->p = y->p;
 if(y->p == NUL)
   (*T) = x;
  else {
     if(y == y->p->left)
        y->p->left = x;
      else
         y->p->right = x;
     }
 if(y != z) {
   if(strlen(z->data) < strlen(y->data))
     z->data = realloc(z->data, strlen(y->data)+1);
   strcpy(z->data, y->data);
   z->counter = y->counter;
   z->origin = y->origin;
  }
 if(y->color == 'B')
   RB_delete_fixup2(T, x);
 free(y->data); 
 free(y);
}

void RB_delete_fixup2(treeNode2 **T, treeNode2 *x) {
 treeNode2 *w;
 while (x != (*T) && x->color == 'B') {
   if(x == x->p->left) {
      w = x->p->right;
      if(w->color == 'R') {
         w->color = 'B';
         x->p->color = 'R';
         left_rotate2(T, x->p);
         w = x->p->right;
        }
      if(w->left->color == 'B' && w->right->color == 'B') {
         w->color = 'R';
         x = x->p;
        }
        else {
              if(w->right->color == 'B') {
                 w->left->color = 'B';
                 w->color = 'R';
                 right_rotate2(T, w);
                 w = x->p->right;
                }
              w->color = x->p->color;
              x->p->color = 'B';
              w->right->color = 'B';
              left_rotate2(T, x->p);
              x = (*T);
             }
     }
     else {
        w = x->p->left;
        if(w->color == 'R') {
           w->color = 'B';
           x->p->color = 'R';
           right_rotate2(T, x->p);
           w = x->p->left;
          }
        if(w->right->color == 'B' && w->left->color == 'B') {
           w->color = 'R';
           x = x->p;
          }
          else {
                if(w->left->color == 'B') {
                   w->right->color = 'B';
                   w->color = 'R';
                   left_rotate2(T, w);
                   w = x->p->left;
                  }
                w->color = x->p->color;
                x->p->color = 'B';
                w->left->color = 'B';
                right_rotate2(T, x->p);
                x = (*T);
               }
       }
 }
 x->color = 'B';
}

int RB_search2(treeNode2 *x, char el[]) {
 if(x == NUL)
   return 0;
 if(strcmp(el, x->data) == 0) {
   return 1; 
  }
  else if (strcmp(el, x->data) < 0) {
    return RB_search2(x->left, el); 
   }
   else {
     return RB_search2(x->right, el); 
    }
 return 0;
} 

treeNode2 *RB_find_node2(treeNode2 *x, char el[]) {
 if(x == NUL)
   return x;

 if(strcmp(el, x->data) == 0)
   return x;
  else if(strcmp(el, x->data) < 0)
      return RB_find_node2(x->left, el);
     else
       return RB_find_node2(x->right, el);
 return NUL;
}      

treeNode2 *RB_successor2(treeNode2 *x) {
 if(x->right != NUL)
   return RB_minimum2(x->right);
 treeNode2 *y = x->p;
 while (y != NUL && x == y->right) {
   x = y;
   y = y->p;
  }
 return y;
}

treeNode2 *RB_minimum2(treeNode2 *x) {
 while (x->left != NUL)
   x = x->left;
 return x;
}

int find_max(treeNode2 *x) {
 treeNode2 *temp = x;
 int r, l;
 r = -1;
 l = -1;
 if(temp == NUL)
  return -1;
 if(temp->left == NUL && temp->right == NUL)
  return temp->counter;
 if(temp->left != NUL) 
  l = find_max(temp->left);
 if(temp->right != NUL)
  r = find_max(temp->right);
 if(r >= l && r > temp->counter)
  return r;
 if(l >= r && l > temp->counter)
  return l;

 return temp->counter;
}

void print_max(treeNode1 *x) { 
 treeNode1 *temp = x;
 if(temp != NIL) {
   print_max(temp->left);

    fputs_unlocked(temp->data, stdout);
    fputs_unlocked(" ", stdout);
 
   print_max(temp->right);
  }
}  


/***************************************
           HELPER FUNCTIONS
 ***************************************/

void origin_delete(treeNode2 *x, list *l, char el[]) {
 if(x != NUL) {
  origin_delete(x->left, l, el);

  treeNode1 *temp = RB_find_node(x->origin, el);
  if(temp != NIL) {
    RB_delete(&(x->origin), temp);
    x->counter--; 
    treeNode1 *t = RB_find_node(l->report, x->data);
    if(t != NIL)
      RB_delete(&(l->report), t);   
   }

  if(x->counter == 0) {
    insert2(x->data);      
   }

  origin_delete(x->right, l, el);
 } 
}

void insert_max(treeNode2 *x, list *l) {
 if(x != NUL) {
   insert_max(x->left, l);

   if(x->counter == l->max)
    RB_insert(&(l->report), x->data);

   insert_max(x->right, l);
  }
}


/***************************************
            LISTS FUNCTIONS
 ***************************************/

void insert(list **L, char el[]) {
 list *prec, *curr, *newnode;
 prec = NULL;
 curr = (*L);
 while (curr != NULL && strcmp(el, curr->rel_name) > 0) {
   prec = curr;
   curr = curr->next;
  }
 newnode = (list*)malloc(sizeof(list));
 newnode->rel_name = malloc((strlen(el)+1)*sizeof(char));
 strcpy(newnode->rel_name, el);
 newnode->max = 0;
 newnode->report = NIL;
 newnode->dest = NUL;   

 if(prec == NULL) {
   newnode->next = (*L);
   (*L) = newnode;
  }
  else {
     prec->next = newnode;
     newnode->next = curr;
    }
}

void delete(list **L, char el[]) {
 list *prec, *curr;
 prec = NULL;
 curr = (*L);

 if(curr != NULL && strcmp(el, curr->rel_name) == 0) { 
   (*L) = curr->next;
   free(curr->rel_name);
   free(curr);
   return;
  }
 while (curr != NULL && strcmp(el, curr->rel_name) != 0) { 
   prec = curr;
   curr = curr->next;
  }
 if(curr != NULL)                     
   prec->next = curr->next;
 else
   return;
 free(curr->rel_name);
 free(curr);
}

int search(list *L, char el[]) {
 list *curr = L;
 while (curr != NULL) {
   if(strcmp(el, curr->rel_name) == 0)
     return 1;
    else 
       curr = curr->next;
  }
 return 0;
}


void insert2(char el[]) {
 list2 *newnode;

 newnode = (list2*)malloc(sizeof(list2));
 newnode->data = malloc((strlen(el)+1)*sizeof(char));
 strcpy(newnode->data, el);

 newnode->next = buffer;
 buffer = newnode;   
}

void delete2() {
 list2 *temp;

 if(buffer != NULL) {
  temp = buffer;
  buffer = buffer->next;

  free(temp->data);
  free(temp);
 }
}


/***************************************
                  MAIN
 ***************************************/

int main() {
 enum comandi {addent, delent, addrel, delrel, report, end};
 char buffer[100];
 char comando[7];
 char *input1;
 char *input2;
 char *input3;

 do {
  fgets_unlocked(buffer, 100, stdin);

  if(strlen(buffer) == 4) {    //sono in end
    exit(0);
   }
   else {
     strncpy(comando, buffer, 6);
     comando[6] = '\0';
    }

  strtok(buffer, " ");
  
  if (strcmp(comando, "addent") == 0) {
                     input1 = strtok(NULL, "\n");
                     Addent(input1);
                    }     
  else if (strcmp(comando, "delent") == 0) {
                     input1 = strtok(NULL, "\n");
                     Delent(input1);
                    }
  else if (strcmp(comando, "addrel") == 0) {
                     input1 = strtok(NULL, " ");
                     input2 = strtok(NULL, " ");
                     input3 = strtok(NULL, "\n");
                     Addrel(input1, input2, input3);
                    }
  else if (strcmp(comando, "delrel") == 0) {
                     input1 = strtok(NULL, " ");
                     input2 = strtok(NULL, " ");
                     input3 = strtok(NULL, "\n");
                     Delrel(input1, input2, input3);
                    }
  else if (strcmp(comando, "report") == 0) {
                     Report();
                    }
 } while (1);

 return 0;
}




