/* ///////////////////////////////////////////////////////////////////////////

  ============
  Graph Module
  ============
                             
  An API for GP2 graphs. Defines structures for graphs, nodes and edges.  

/////////////////////////////////////////////////////////////////////////// */

#ifndef INC_GRAPH_H
#define INC_GRAPH_H

#define MAX_INCIDENT_EDGES 16

#include <glib.h>
#include "error.h"
#include "globals.h"
#include "label.h"
#include "stack.h"

typedef struct LabelClassTable {
   int pool_size;
   int index;
   int *items;
} LabelClassTable;

/* Responsible for allocating heap memory to a LabelClassTable. 
 * Only called by addNode, relabelNode, addEdge and relabelEdge. 
 * initial_size is either the graph's node pool size or the graph's edge pool 
 * size. initial_size / 4 items are allocated to the table's items array
 * for the first allocation. */
void addLabelClassIndex(LabelClassTable *table, int index, int initial_size);
/* Only called by removeNode and removeEdge. */
void removeLabelClassIndex(LabelClassTable *table, int index);

typedef struct RootNodes {
   int index;
   struct RootNodes *next;
} RootNodes;

typedef struct Graph 
{
   struct Node *nodes;
   int node_pool_size, node_index;

   struct Edge *edges;
   int edge_pool_size, edge_index;

   /* Integer arrays to record the holes in the node and edge arrays caused by
    * node and edge deletion. The first free_node_index elements of the 
    * free node slots array are indices of holes in the graph's node array.
    * When a node is added to the graph, the free node slots array is consulted
    * first, since filling a hole is better than inserting a node at the end
    * of the array in which gaps exist. */
   int *free_node_slots, *free_edge_slots;
   int free_node_index, free_edge_index;

   /* The number of non-dummy items in the graph's nodes/edges array.
    * Do NOT use these as a bound for an iterator over the arrays. Instead use
    * node_index and edge_index. 
    * The equations below are invariant properties of this data structure.
    * number_of_nodes + free_node_index = node_index. 
    * number_of_edges + free_edge_index = edge_index. */
   int number_of_nodes, number_of_edges;

   /* Arrays of LabelClassTable indexed by label class. 
    * Each LabelClassTable is a dynamically-allocated array of node or edge
    * indices. Initially these arrays are NULL; memory is allocated on demand
    * since not all label classes are likely to be represented in a single
    * GP 2 program. */
   struct LabelClassTable nodes_by_label[LABEL_CLASSES];
   struct LabelClassTable edges_by_label[LABEL_CLASSES];

   /* Root nodes referenced in a linked list for fast access. */
   struct RootNodes *root_nodes;
} Graph;

void addRootNode(Graph *graph, int index);
void removeRootNode(Graph *graph, int index);

typedef struct Node {
   int index;
   bool root;
   LabelClass label_class;
   Label *label;

   /* Fixed-size arrays for the node's outgoing and incoming edges. */
   int out_edges[MAX_INCIDENT_EDGES];
   int in_edges[MAX_INCIDENT_EDGES];
   
   /* Pointers to extra incident edge index storage in case the array's
    * bounds are exceeded. Initially, these are NULL pointers. */
   int *extra_out_edges, *extra_in_edges;
   /* The size of the extra_out_edges and extra_in_edges arrays respectively. */
   int out_pool_size, in_pool_size;

   /* If extra edge arrays have been allocated, you must subtract
    * MAX_INCIDENT_EDGES from this number to get the correct index into the
    * extra edge array. */
   int out_index, in_index;

   /* Bidirectional edges, and hence bidegrees, exist only in rule graphs.
    * A bidirectional edge is internally represented as either a single outedge
    * or a single inedge, but it contributes only to the node's bidegree. In
    * other words, adding a bidirectional edge increments the bidegree but does
    * not change the indegree or the outdegree.
    *
    * For host graphs, and for rule graphs with bidegree 0, the out(in)degree
    * is the number of non-negative indices in the node's out(in)edge arrays.
    * For rule graphs with bidegree > 0, the invariant is less strict, since 
    * a bidirectional edge may lie in either the outedge array or the inedge
    * array. All that can be said for certain is that the sum of the three
    * degrees is the number of non-negative indices in all of the node's edge
    * arrays. */
   int outdegree, indegree, bidegree;
} Node;

extern struct Node dummy_node;

typedef struct Edge {
   int index;
   bool bidirectional;
   LabelClass label_class;
   Label *label;
   int source, target;
} Edge;

extern struct Edge dummy_edge;



/* The arguments nodes and edges are the initial sizes of the node array and the
 * edge array respectively. */
Graph *newGraph(int nodes, int edges);

/* Nodes and edges are created and added to the graph with the addNode and addEdge
 * functions. They take the necessary construction data as their arguments and 
 * return their index in the graph. 
 *
 * To assign the empty label to a node or edge, pass NULL as the label 
 * argument. This also applies to the relabelling functions. */
int addNode(Graph *graph, bool root, Label *label);
int addEdge(Graph *graph, bool bidirectional, Label *label, int source_index, 
            int target_index);
void removeNode(Graph *graph, int index);
void removeEdge(Graph *graph, int index);

/* The relabel functions take boolean arguments to control if the label is 
 * updated and if the boolean flag of the item should be changed. For nodes, 
 * this is the root flag. For edges, this is the bidirectional flag. */
void relabelNode(Graph *graph, Node *node, Label *new_label, bool change_label, 
                 bool change_root); 
void relabelEdge(Graph *graph, Edge *edge, Label *new_label, bool change_label, 
                 bool change_bidirectional);


extern Stack *graph_stack;
/* Creates a memory copy of the passed graph and pushes it to graph_stack. */
void copyGraph(Graph *graph);
/* restoreGraph is passed the current graph. It frees its argument before 
 * returning the top graph from graph_stack. */
Graph *restoreGraph(Graph *graph);
void freeGraphStack(Stack *graph_stack);


Node *getNode(Graph *graph, int index);
Edge *getEdge(Graph *graph, int index);
RootNodes *getRootNodeList(Graph *graph);
LabelClassTable getNodesByLabel(Graph *graph, LabelClass label_class);
LabelClassTable getEdgesByLabel(Graph *graph, LabelClass label_class);
/* The following four functions return indices. To get pointers to the items,
 * pass the return value to getNode or getEdge with the appropriate graph. */
int getInEdge(Node *node, int index);
int getOutEdge(Node *node, int index);
int getSource(Edge *edge);
int getTarget(Edge *edge);
Label *getNodeLabel(Node *node);
Label *getEdgeLabel(Edge *edge);
int getIndegree(Node *node);
int getOutdegree(Node *node);

void printGraph(Graph *graph);
void freeGraph(Graph *graph);

/* structs placed on the graph change stack. It contains sufficient
 * information to undo any of the six types of graph change.
 *
 * Add Node + Add Edge
 * ===================
 * The undo operations are removeNode and removeEdge which only require the 
 * name of the item's since the pointer can be quickly accessed via a hash
 * lookup. Storing the pointer to the item itself is dangerous because the 
 * added item could be deleted before restoreGraph is called.
 *
 * Remove Node + Remove Edge
 * =========================
 * The undo operations are addNode and addEdge. The structs contain the 
 * arguments to these functions. In the case of addEdge, called with pointers
 * to its source and target, these are obtained by hasing the names in
 * the GraphChange removed_edge struct. The label pointers are heap copies
 * since labels are freed when an item is deleted.
 *
 * Relabel Node + Relabel Edge
 * ===========================
 * As above, the structs contain the arguments to the relabelNode and 
 * relabelEdge functions. Labels do not have to be copied to heap in this
 * case as the pointers to the old labels of the items are used.
 */

/* typedef enum { ADD_NODE = 0, ADD_EDGE, REMOVE_NODE, REMOVE_EDGE, 
	       RELABEL_NODE, RELABEL_EDGE } GraphChangeType; 

typedef struct GraphChange 
{
   GraphChangeType type;
   union
   {
      int added_node; 
      int added_edge;

      struct {
         bool root;
         Label *label;
      } removed_node;

      struct {
         Label *label;
         bool bidirectional;
         int source_name;
         int target_name;
      } removed_edge;

      struct {
         int index;
         Label *old_label;
         bool change_root;
      } relabelled_node;
   
      struct {
         int index;
         Label *old_label;
         bool change_bidirectional;
      } relabelled_edge;
   } data;
} GraphChange; 
void freeGraphChange(GraphChange *change); */

#endif /* INC_GRAPH_H */
