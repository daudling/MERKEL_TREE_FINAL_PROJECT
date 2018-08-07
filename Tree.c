#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "md5.h"
//set everything
typedef int(*Hash_Function) (unsigned char *, unsigned int, unsigned char *);

typedef struct{
	char *hash;
	char *data;
}treeNode;

typedef struct{
	size_t n;
	size_t height;
	size_t sizeOfHash;
	size_t sizeOfBlock;
	size_t blocks;
	Hash_Function hash_function;
	treeNode *nodes;
} tree;
int makeTree(tree *tree, char **data);
int compare(tree *a, tree *b, size_t i);
int set(tree *tree, size_t i, char *data);
void freeTree(tree *tree);
int hash(tree *tree, size_t i);
void print(tree *tree);

//begin functions
//build tree
int makeTree(tree *tree, char **data){
	if(tree->blocks > (1<<(tree->height-1))){
		return -1;
	}
	int start, i;
	start = (1<<(tree->height-1));
	tree->n = start + tree->blocks-1;
	tree->nodes = (treeNode *)malloc(sizeof(treeNode) * (tree->n +1));
	for(i = start;i<=tree->n;i++){
		tree->nodes[i].data = data[i-start];
		tree->nodes[i].hash = NULL;
		if(hash(tree,i) == -1){
			return -1;
		}
	}
	for(i=start-1;i>0;i--){
		tree->nodes[i].hash = NULL;
		if(hash(tree,i) == -1){
			return -1;
		}
	}
	return 0;
}

//compares two trees
//must be same size
int compare(tree *a, tree *b, size_t i){
	int cmp;
	if(i > (1<<a->height)-1){
		return -1;
	}
	if(memcmp(a->nodes[i].hash, b->nodes[i].hash, a->sizeOfHash) !=0){
		if(i<<1 > (1<<(a->height-1))){
			return i - (1<<(a->height-1)) +1;
		}
		else{
			cmp = compare(a,b,i<<1);
			if(cmp == 0){
				return compare(a,b,(i<<1)+1);
			}
			else{
				return cmp;
			}
		}
	}
	else{
		return 0;
	}
}

//set tree data
int set(tree *tree, size_t block_num, char *data){
	if(block_num > tree->blocks){
		return -1;
	}
	size_t i = (1<<(tree->height -1)) + block_num -1;
	if(tree->nodes[i].data){
		free(tree->nodes[i].data);
	}
	tree->nodes[i].data = data;
	if(hash(tree, i) == -1){
		return -1;
	}
	for(i>>=1; i>0; i>>=1){
		if(hash(tree, i) == -1){
			return -1;
		}
	}
	return 0;
}

//free the tree
void freeTree(tree *tree){
	int i;
	if(!tree){
		return;
	}
	if(tree->nodes){
		for(i=1;i<=tree->n;i++){
			if(tree->nodes[i].hash){
				free(tree->nodes[i].hash);
			}
		}
		free(tree->nodes);
	}
	return;
}

//hash node
int hash(tree *tree, size_t i){
	if(i>(1<<tree->height)-1){
		return -1;
	}
	if(i<(1<<tree->height-1)){
		if(2*i+1 <= tree->n && tree->nodes[2*i].hash && tree->nodes[2*i+1].hash){
			char *buffer = (char *)malloc(sizeof(char *) * (2 * tree->sizeOfHash+1));
			memcpy(buffer, tree->nodes[2*i+1].hash, tree->sizeOfHash);
			memcpy(buffer+tree->sizeOfHash, tree->nodes[2*i+1].hash, tree->sizeOfHash);
			if(!tree->nodes[i].hash){
				tree->nodes[i].hash = (char *)malloc(sizeof(char *) * tree->sizeOfHash);
			}
			tree->hash_function(buffer,2*tree->sizeOfHash, tree->nodes[i].hash);
			free(buffer);
		}
		else if(2*i<=tree->n && tree->nodes[2*i].hash){
			if(!tree->nodes[i].hash){
				tree->nodes[i].hash = (char *)malloc(sizeof(char *) * tree->sizeOfHash);
			}
		}
	}
	else{
		if(tree->nodes[i].data){
			if(!tree->nodes[i].hash){
				tree->nodes[i].hash = (char *)malloc(sizeof(char *) * tree->sizeOfHash);
			}
			tree->hash_function(tree->nodes[i].data, tree->sizeOfBlock, tree->nodes[i].hash);
		}
		else{
			return -1;
		}
	}
	return 0;
}

//print tree
void print(tree *tree){
	int i;
	printf("-------------------------\n");
	for(i=1;i<=tree->n;i++){
		MD5Print(tree->nodes[i].hash);
	}
	printf("-------------------------\n");
	return;
}

//merkele tree testing
#define TREE_HEIGHT 4
#define BLOCK_SIZE 1024
#define DATA_BLOCKS 8

int main(){
	int i;
	char *data[DATA_BLOCKS], *data_copy[DATA_BLOCKS], buffer[BLOCK_SIZE];

	tree tree_a = {0, TREE_HEIGHT, MD5_DIGEST_LENGTH, BLOCK_SIZE, DATA_BLOCKS, MD5One, NULL};
	tree tree_b = {0, TREE_HEIGHT, MD5_DIGEST_LENGTH, BLOCK_SIZE, DATA_BLOCKS, MD5One, NULL};

	for(i=0; i<BLOCK_SIZE;i++)
		buffer[i] = 'A';
	for(i=0; i<DATA_BLOCKS;i++){
		data[i] = (char *)malloc(sizeof(char) * BLOCK_SIZE);
		data_copy[i] = (char *)malloc(sizeof(char) * BLOCK_SIZE);
		memcpy(data[i], buffer, BLOCK_SIZE);
		memcpy(data_copy[i], buffer, BLOCK_SIZE);
	}
	makeTree(&tree_a, data);
	data_copy[7][0]='B';
	makeTree(&tree_b, data_copy);

	print(&tree_a);
	print(&tree_b);

	printf("print 0 for no difference: %d\n", compare(&tree_a, &tree_b, 1));

	set(&tree_b, 8, buffer);

	print(&tree_a);
	print(&tree_b);

	printf("print 0 for no difference: %d\n", compare(&tree_a, &tree_b, 1));

	freeTree(&tree_a);
	freeTree(&tree_b);
	return 0;
}













