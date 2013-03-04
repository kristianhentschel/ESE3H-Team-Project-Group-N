#include "tree.h"
#include <pthread.h>
#include <stdlib.h>
#include "iterator.h"
#include <stdio.h>

typedef struct tree_node {
	struct tree_node *parent;
	struct tree_node *left;
	struct tree_node *right;
	void *value;
} tree_node;

struct tree {
	pthread_mutex_t mutex;
	struct tree_node *root;
	int (*cmp)(void *, void *);
	long size;
};


static tree_node *node_create( void *value );
static void node_insert( tree *tree, tree_node *node, void *value );
static void inorder(tree_node *n, void **elements, long i);

/*
 * alloc a tree structure without a root node, and set the comparison function
 * used for insertions.
 */
tree *tree_create(int (*cmp)(void *, void *)) {
	tree *t;

	if((t = (tree *) malloc (sizeof(tree))) == NULL)
		return NULL;

	t->root = NULL;
	t->cmp = cmp;
	t->mutex = PTHREAD_MUTEX_INITIALIZER;

	return t;
}

void tree_insert( tree *tree, void *value ) {
	pthread_mutex_lock(&tree->mutex);
	fprintf(stderr, "inserting %s\n", (char *) value);
	if(tree->root == NULL)
		tree->root = node_create(value);
	else
		node_insert( tree, tree->root, value );
	tree->size++;
	pthread_mutex_unlock(&tree->mutex);
}

static void node_insert( tree *tree, tree_node *node, void *value ){
	if( tree->cmp(value, node->value) > 0 ){
		//value is greater, so insert right.
		if(node->right == NULL) {
			node->right = node_create(value);
			node->right->parent = node;
		} else {
			node_insert( tree, node->right, value );
		}
	} else {
		//insert left
		if(node->left == NULL) {
			node->left = node_create(value);
			node->left->parent = node;
		} else {
			node_insert( tree, node->left, value );
		}

	}
}

static tree_node *node_create( void *value ){
	tree_node *n;
	if((n = (tree_node *) malloc(sizeof(tree_node))) == NULL)
		return NULL;
	
	n->parent = NULL;
	n->left = NULL;
	n->right = NULL;
	n->value = value;
	return n;
}

/*
 * does not free the node value.
 */
static void node_destroy( tree_node *n ) {
	if(n == NULL)
		return;
	node_destroy(n->left);
	node_destroy(n->right);
	free(n);
}

void tree_destroy( tree *tree ) {
	pthread_mutex_lock(&tree->mutex);
	node_destroy( tree->root );

	pthread_mutex_unlock(&tree->mutex);
	pthread_mutex_destroy(&tree->mutex);
	free(tree);
}


iterator *tree_iterator_inorder( tree *tree ){
	void *elements[tree->size];

	inorder(tree->root, elements, 0);

	return iterator_create(tree->size, elements);
}

/* traverse the tree in-order, starting at node n.
 * put each element into element, incrementing it as we go.
 * assumes that enough space has been allocated.
 */
static void inorder(tree_node *n, void **elements, long i){
	if(n->left != NULL){
		inorder(n->left, elements, i++);
	}

	fprintf(stderr, "in-order: %s\n", (char *) n->value);
	elements[i] = n->value;

	if(n->right != NULL){
		inorder(n->right, elements, i++);
	}
}

void tree_iterator_destroy( iterator *it, tree *tree ){
	iterator_destroy(it);
	pthread_mutex_unlock(&tree->mutex);
}
