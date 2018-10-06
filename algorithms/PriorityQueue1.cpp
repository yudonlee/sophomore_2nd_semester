#include <stdio.h>
#include <stdlib.h>
#define Left(a)  2*a;
#define Right(a) 2*a+1;
void Swap(int& a,int& b){
  int t;
  t=a;
  a=b;
  b=t;
}
void Heapify(int* arr,int size,int parent){
  int max = parent;
  int l =Left(parent);
  int r= Right(parent);
  if (size < l) return;
  else if (size == l) {
    if (arr[parent] < arr[l]) {
      max = l;
    }
  }
  else {
    if (arr[l] > arr[r]) {
      if (arr[parent] < arr[l]) {
	max = l;
      }
		}
    else {
      if (arr[parent] < arr[r]) {
	max =r;
      }
    }
  }
 
  if (max != parent) {
    Swap(arr[max], arr[parent]);
    Heapify(arr,size ,max);
  }
  
}
void Build_MaxHeap(int* arr,int *m){
  for (int j=*m/2;j> 0;j--){
    Heapify(arr,*m,j);
  }
}
int main(){
  int tmp[100000];
  int n;
  int Queue_size=0;
  while(true){
    scanf("%d",&n);
    if(n==0){
      for(int i=1;i<=Queue_size;i++)
	printf("%d ",tmp[i]);
      break;
    }
    if(n==1){
      Queue_size++;
      scanf("%d\n",&tmp[Queue_size]);
      Build_MaxHeap(tmp,&Queue_size);
    }
    else if(n==2){
      
      printf("%d\n",tmp[1]);
      if(Queue_size>1)
	Swap(tmp[1],tmp[Queue_size]);
      Queue_size--;
      Heapify(tmp,Queue_size,1);
    }
    else if(n==3){
      int second,third;
      scanf("%d %d",&second,&third);
      tmp[second] = third;
      int parent,child;
      parent = second/2;
      child =second;
      while(parent>0){
	if(tmp[parent]<tmp[child]){
	  Swap(tmp[parent],tmp[child]);
	  parent /=2;
	  child /=2;
	}
	else break;
      }
      Heapify(tmp,Queue_size,child);	    
    }
    else
      return 0;
  }
  return 0;
}
