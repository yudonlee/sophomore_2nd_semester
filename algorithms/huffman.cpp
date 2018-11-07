#include <stdio.h>
#include <stdlib.h>
#define Parent(a) (a)/2;
#define Left(a)  2*a;
#define Right(a) 2*a+1;
int length; //length of heap
int sum;
typedef struct node{
	int freq;
	char ch[5];
	struct node* left;
	struct node* right; 
	
}Node;
void Swap(Node** leaf1,Node** leaf2);
Node* leaf_merge(Node** heap,Node* leaf1,Node* leaf2);
void huffman(Node** heap);
void insert(Node** heap,Node* leaf);
Node* Extract(Node** heap);
void min_Heapify(Node** heap,int parent);
void order(Node* leaf,int height);
int main(){
	int N,S;
	scanf("%d",&N);
	Node* heap[300000];
	Node* leaf = (Node*)malloc(sizeof(Node)*N);
	for(int i=1;i<=N;i++){
		scanf("%s",leaf[i].ch);
		scanf("%d",&leaf[i].freq);
		leaf[i].left = NULL;
		leaf[i].right = NULL;
		insert(heap,&leaf[i]);
	}
	scanf("%d",&S);
	huffman(heap);
	order(heap[1],0);
	int count =0;
	for(int i=1;i<=N;i*=2)
		count++;
	if(N==1){
		printf("%d\n",S);
		printf("%d\n",S);
	}
	else{
		printf("%d\n",S*count);
		printf("%d\n",sum);
	}
	free(leaf);

	return 0;
}
Node* leaf_merge(Node** heap,Node* leaf1,Node* leaf2){
	Node* output = (Node*)malloc(sizeof(Node));
	if(leaf1->freq < leaf2->freq){
		output->left = leaf1;
		output->right = leaf2;
	}
	else{
		output->left = leaf2;
		output->right = leaf1;
	}
	output->freq = leaf1->freq+leaf2->freq;
	output->ch[0] = '\0';
	return output;
}
void Swap(Node** a,Node** b){
  Node* tmp;
  tmp = *a;
  *a=*b;
  *b=tmp;
}
void huffman(Node** heap){
	Node* parent = (Node*)malloc(sizeof(Node));	
	Node* leaf1 = (Node*)malloc(sizeof(Node));
	Node* leaf2 = (Node*)malloc(sizeof(Node));
	while(length>1){
		leaf1 = Extract(heap);    
		leaf2 = Extract(heap);
	    parent = leaf_merge(heap,leaf1,leaf2);
		insert(heap,parent);
	}
}
void min_Heapify(Node** heap,int parent){
  int l,r,large;
  while (parent <= length / 2)
    {
		l = Left(parent);
		large = l;
		r = Right(parent);
		if (length >= r){
            if ((*heap[r]).freq <= (*heap[l]).freq)
                large = r;
            else
                large = l;
        }
        else{
            large = l;
        }
        if ((*heap[parent]).freq > (*heap[large]).freq)
        {
            Swap(&heap[parent], &heap[large]);
            parent = large;
        }
        else
            break;
    }
}
void insert(Node** heap,Node* leaf){
	int i= ++length;
	while(i != 1 && leaf->freq < heap[i/2]->freq){
		heap[i] = heap[i/2];
		i /= 2;
	}
	heap[i] = leaf;
}
Node* Extract(Node** heap){
	Node* output;
	output = heap[1];
	Swap(&heap[1],&heap[length--]);
	min_Heapify(heap,1);
	return output;
} 
void order(Node* leaf,int level){
 	if(leaf){
		if(leaf->ch[0] != '\0')
			sum+=leaf->freq*level;
		order(leaf->left,level+1);
		order(leaf->right,level+1);
	}
}
