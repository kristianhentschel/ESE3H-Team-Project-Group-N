#include <tree.h>
#include <stdlib.h>


struct tree_node {
	tree_node *parent;
	tree_node *left;
	tree_node *right;
	void *value;
};

struct tree {
	//TODO semaphore etc
	tree_node *root;
	int (*cmp)(void *, void *);
};


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

	return t;
}

void tree_insert( tree *tree, void *value ) {
	if(tree->root == NULL)
		tree->root = node_create(value);
	else
		node_insert( tree->root, value );
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
tree_node *tree_left( tree_node *node );
tree_node *tree_right( tree_node *node );
		node->left->parent = node;
int tree_isleaf( tree_node *node );
void tree_destroy( tree *tree );
