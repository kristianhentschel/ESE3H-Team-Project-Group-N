#ifndef _TREE_H_
#define _TREE_H_

#include "iterator.h"

typedef struct tree tree;

tree *tree_create( int (*cmp)(void *, void *) );
void tree_insert( tree *tree, void *value );
void tree_destroy( tree *tree );


iterator *tree_iterator_inorder( tree *tree );
/* ts_iterator *tree_iterator_preorder( tree *tree ); */
/* ts_iterator *tree_iterator_postorder( tree *tree ); */
void tree_iterator_destroy( iterator *it, tree *tree );
#endif /* _TREE_H_ */
