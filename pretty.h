/* ///////////////////////////////////////////////////////////////////////////

  ======================
  Pretty Printing Module
  ======================

  Module for pretty printing the abstract syntax tree and the symbol table.
  Contains several macros to free the source file from clutter and prototypes
  for printing functions.                         
                      
/////////////////////////////////////////////////////////////////////////// */

#ifndef INC_PRETTY_H
#define INC_PRETTY_H

#include <glib.h>
#include "ast.h"
#include "error.h"
#include "globals.h"
#include "seman.h" 

#define print_to_dot_file(text, ...)                  \
  do { fprintf(dot_file, text, ## __VA_ARGS__); }     \
  while(0) 

#define print_to_symtab_file(text, ...)                        \
  do { fprintf(symbol_table_file, text, ## __VA_ARGS__); }     \
  while(0) 


/* prettyPrint is a macro that calls the appropriate print provided the first
 * argument is not a null pointer. 
 *
 * POINTER_ARG is a member of the current structure pointing to an AST node 
 * that is to be printed.
 *
 * TYPE corresponds to the print functions in this file. For example, calling
 * prettyPrint with second argument 'List' will call printList on POINTER_ARG
 * if POINTER_ARG is not NULL. Otherwise an error node is created in the 
 * appropriate place and an error message is printed to stderr. 
 *
 * Be aware that prettyPrint does not write an edge to point to any nodes
 * written by prettyPrint. The edges must be explicitly written directly 
 * before calling this macro. This is done in the .c file.
 */ 

#define prettyPrint(POINTER_ARG,TYPE)                                         \
  do { 									      \
       if(POINTER_ARG != NULL)                                                \
         printAST ## TYPE (POINTER_ARG, dot_file);                            \
       else {                                                                 \
         print_to_dot_file("node%d[shape=plaintext,label=\"%d ERROR\"]\n",    \
                           next_node_id, next_node_id);                       \
         print_to_log("Error: Unexpected NULL pointer at AST node %d\n",      \
                           next_node_id);                                     \
       }            							      \
     }                                                                        \
  while (0)


/* prettyPrintList is used to process members of AST structs that point
 * to a struct List. It should only be used when a NULL pointer is valid in
 * the GP AST. For example, this macro is called to print the node 
 * and edge lists of a graph, which may be NULL, but not for the list
 * component of a label, which should not point to NULL. Use printList
 * or prettyPrint for the latter cases.
 * 
 * The NODE_TYPE parameter is the name of the argument of the calling function:
 * the name of the structure that stores the node_id.
 *
 * If POINTER_ARG is NULL, a NULL node is written to the .dot file and an edge
 * is created from the current node to the NULL node with the label EDGE_LABEL. 
 *
 * Otherwise an edge is written with label EDGE_LABEL, pointing from the 
 * current node to the node that will be created by the printList call 
 * immediately following this edge creation. The use of the global node counter
 * next_node_id ensures that the edge points to the correct node.
 */

#define prettyPrintList(POINTER_ARG, NODE_TYPE, EDGE_LABEL)                 \
   do {                                                                     \
        if(POINTER_ARG == NULL) {                                           \
          print_to_dot_file("node%d[shape=plaintext,label=\"%d NULL\"]\n",  \
                            next_node_id, next_node_id);                    \
          print_to_dot_file("node%d->node%d[label=\"" #EDGE_LABEL "\"]\n",  \
                            NODE_TYPE->node_id, next_node_id);              \
          next_node_id += 1;                                                \
        }							            \
        else {                                                              \
          print_to_dot_file("node%d->node%d[label=\"" #EDGE_LABEL "\"]\n",  \
                            NODE_TYPE->node_id, next_node_id);              \
          printASTList(POINTER_ARG, dot_file);                              \
        }                                                                   \
      } 			                                            \
    while (0)

/* LOCATION_ARGS(LOC) is shorthand for the components of the location structure
 * that are required for calls to fprintf. LOC is a variable of type struct 
 * YYLTYPE, which occurs in the location field of every AST node struct.
 */

#define LOCATION_ARGS(LOC)    \
   LOC.first_line, LOC.first_column, LOC.last_line, LOC.last_column


#define printListNode(NODE_LABEL)                                     \
  do                                                                  \
  {                                                                   \
     print_to_dot_file("node%d[shape=box,label=\"%d\\n%d.%d-%d.%d\\n" \
                       #NODE_LABEL "\"]\n",                           \
                       list->node_id, list->node_id,                  \
                       LOCATION_ARGS(list->location));                \
     print_to_dot_file("node%d->node%d[label=\"value\"]\n",           \
                       list->node_id, next_node_id);                  \
  }                                                                   \
  while(0)              

#define printDeclarationNode(NODE_LABEL_1, NODE_LABEL_2)                \
  do                                                                    \
  {                                                                     \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"             \
                       #NODE_LABEL_1 "\"]\n",                           \
                       decl->node_id, decl->node_id,                    \
                       LOCATION_ARGS(decl->location));                  \
     print_to_dot_file("node%d->node%d[label=\"" #NODE_LABEL_2 "\"]\n", \
                       decl->node_id, next_node_id);                    \
  }                                                                     \
  while(0)     

#define printConditionalNode(NODE_LABEL)                                  \
  do                                                                      \
  {                                                                       \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"               \
                       #NODE_LABEL "\\n Restore Point = %d\\n"            \
                       "Roll Back Point = %d\\n"                          \
                       "Copy Point = %d\"]\n",                            \
                       stmt->node_id, stmt->node_id,                      \
                       LOCATION_ARGS(stmt->location),                     \
                       stmt->value.cond_branch.restore_point,             \
                       stmt->value.cond_branch.roll_back_point,           \
                       stmt->value.cond_branch.copy_point);               \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"condition\"]\n",           \
                       stmt->node_id, next_node_id);                      \
     prettyPrint(stmt->value.cond_branch.condition, Statement);           \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"then\"]\n",                \
                       stmt->node_id, next_node_id);                      \
     prettyPrint(stmt->value.cond_branch.then_stmt, Statement);           \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"else\"]\n",                \
                       stmt->node_id, next_node_id);                      \
     prettyPrint(stmt->value.cond_branch.else_stmt, Statement);           \
  }                                                                       \
  while(0)     

#define printTypeCheckNode(NODE_LABEL_1, NODE_LABEL_2)                        \
  do                                                                          \
  {                                                                           \
     if(cond->value.var != NULL)                                              \
        print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"                \
                          #NODE_LABEL_1 "\\n Variable: %s\"]\n",              \
                          cond->node_id, cond->node_id,                       \
                          LOCATION_ARGS(cond->location), cond->value.var);    \
     else                                                                     \
     {                                                                        \
        print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"                \
                          "Variable: \\n UNDEFINED\"]\n",                     \
                          cond->node_id, cond->node_id,                       \
                          LOCATION_ARGS(cond->location));                     \
        print_to_dot_file("Error (printASTCondition." #NODE_LABEL_2           \
                          "): Undefined name at AST node %d", cond->node_id); \
     }                                                                        \
  }                                                                           \
  while(0)                                                                    \

#define printListEqualityNode(NODE_LABEL)                                 \
  do                                                                      \
  {                                                                       \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"               \
                       #NODE_LABEL "\"]\n", cond->node_id, cond->node_id, \
                       LOCATION_ARGS(cond->location));                    \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"left list\"]\n",           \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.list_cmp.left_list, List);                   \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"right list\"]\n",          \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.list_cmp.right_list, List);                  \
  }                                                                       \
  while(0)  

#define printRelationalNode(NODE_LABEL)                                   \
  do                                                                      \
  {                                                                       \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"               \
                       #NODE_LABEL "\"]\n", cond->node_id, cond->node_id, \
                       LOCATION_ARGS(cond->location));                    \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"left exp\"]\n",            \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.atom_cmp.left_exp, Atom);                    \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"right exp\"]\n",           \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.atom_cmp.right_exp, Atom);                   \
  }                                                                       \
  while(0)  

#define printBinaryBooleanNode(NODE_LABEL)                                \
  do                                                                      \
  {                                                                       \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"               \
                       #NODE_LABEL "\"]\n", cond->node_id, cond->node_id, \
                       LOCATION_ARGS(cond->location));                    \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"left exp\"]\n",            \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.bin_exp.left_exp, Condition);                \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"right exp\"]\n",           \
                       cond->node_id, next_node_id);                      \
     prettyPrint(cond->value.bin_exp.right_exp, Condition);               \
  }                                                                       \
  while(0)  

#define printBinaryOperatorNode(NODE_LABEL)                               \
  do                                                                      \
  {                                                                       \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n"               \
                       #NODE_LABEL "\"]\n", atom->node_id, atom->node_id, \
                       LOCATION_ARGS(atom->location));                    \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"left exp\"]\n",            \
                       atom->node_id, next_node_id);                      \
     prettyPrint(atom->value.bin_op.left_exp, Atom);                      \
                                                                          \
     print_to_dot_file("node%d->node%d[label=\"right exp\"]\n",           \
                       atom->node_id, next_node_id);                      \
     prettyPrint(atom->value.bin_op.right_exp, Atom);                     \
  }                                                                       \
  while(0)  

#define printDegreeOperatorNode(NODE_LABEL_1, NODE_LABEL_2)                 \
  do                                                                        \
  {                                                                         \
     if(atom->value.name != NULL)                                           \
     print_to_dot_file("node%d[label=\"%d\\n%d.%d-%d.%d\\n" #NODE_LABEL_1   \
                       "(%s)\"]\n", atom->node_id, atom->node_id,           \
                       LOCATION_ARGS(atom->location), atom->value.node_id); \
     else                                                                   \
     {                                                                      \
        print_to_dot_file("node%d[shape=box,label=\"%d\\n%d.%d-%d.%d\\n"    \
                          #NODE_LABEL_1": \\n UNDEFINED\"]\n",              \
                          atom->node_id, atom->node_id,                     \
                          LOCATION_ARGS(atom->location));                   \
        print_to_log("Error (printASTAtom." #NODE_LABEL_2 "): Undefined "   \
                     "node name at AST node %d", atom->node_id);            \
     }                                                                      \
  }                                                                         \
  while(0)  

/* printSymbolTable creates the file <program>.tab, where <program> is the
 * name of the GP2 program file, and pretty prints the symbol table to that 
 * file. 
 *
 * Argument 1: The symbol table.
 * Argument 2: The name of the GP2 program file.*/

void printSymbolTable(GHashTable *table, string const file_name);


/* printSymbolList is an auxiliary function called by printSymbolTable. It
 * iterates over a symbol list, pretty printing each symbol in the list. */

void printSymbolList(gpointer key, gpointer value, gpointer user_data);

/* printDotAST creates a new file <source_name>.dot. It generates
 * a DOT text file that can be used to draw a picture of the AST via
 * graphviz. 
 * 
 * Argument 1: A pointer to the AST to be pretty printed.
 * Argument 2: The name of the GP2 program file. */

void printDotAST(List * const gp_ast, string file_name);


/* Identical to printDotAST, except it calls printGraph instead of
 * printList.
 *
 * Argument 1: A pointer to the AST of the host graph to be pretty printed.
 * Argument 2: The name of the GP2 host graph file. */

void printDotHostGraph(GPGraph * const host_graph_ast, string file_name);

/* printList is called by printDotAST. It walks through the AST, outputting
 * lines to dot_file and recursively calling printing functions at the
 * appropriate places. 
 * 
 * Argument 1: A pointer to a List in the AST.
 * Argument 2: The .dot file created by printDotAST.
 * Argument 3: A variable to keep track of the current node number. Used
               to uniquely identify nodes in the output. */

void printASTList(List * const list, FILE *dot_file);
void printASTDeclaration(GPDeclaration * const decl, FILE *dot_file);
void printASTStatement(GPStatement * const stmt, FILE *dot_file);
void printASTCondition(GPCondExp * const cond, FILE *dot_file);
void printASTAtom(GPAtomicExp * const atom, FILE *dot_file);
void printASTProcedure(GPProcedure * const proc, FILE *dot_file);
void printASTRule(GPRule * const rule, FILE *dot_file);
void printASTGraph(GPGraph * const graph, FILE *dot_file);
void printASTNode(GPNode * const node, FILE *dot_file);
void printASTEdge(GPEdge * const edge, FILE *dot_file);
void printASTLabel(GPLabel * const label, FILE *dot_file);
/* void printASTPosition(GPPos * const pos, FILE *dot_file); */

#endif /* INC_PRETTY_H */
