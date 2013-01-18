#ifndef _TREE_H_
#define _TREE_H_

typedef struct tree_node tree_node;
typedef struct tree tree;

tree *tree_create();

void tree_insert( tree *tree, void *value );
tree_node *tree_left( tree_node *node );
tree_node *tree_right( tree_node *node );
tree_node *tree_parent( tree_node *node );
int tree_isleaf( tree_node *node );
void tree_destroy( tree *tree );

#endif /* _TREE_H_ */
