#include <stdio.h>
#include <stdlib.h>
#define Left(a)  2*a;
#define Right(a) 2*a+1;
int Queue_size;
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
void Build_MaxHeap(int* arr,int n){
  for (int j=n/2;j> 0;j--){
     Heapify(arr,n,j);
  }
}
//Extract max num and Heapsort.
void Extract(int* arr){
  if(Queue_size<1)
    return;
  Swap(arr[1],arr[Queue_size]);
  Queue_size--;
  Heapify(arr,Queue_size,1);
}

int main(){
  int tmp[100000];
  int n;
  int j=0;
  int count=0;
  Queue_size=0;
  while(true){
    scanf("%d",&n);
    if(n==0){
      int *p = tmp+count;
      while(p>(tmp+Queue_size)){
	printf("%d ",*p--);
      }
      printf("\n");
      for(int i=1;i<=Queue_size;i++)
	printf("%d ",tmp[i]);
      printf("\n");
      return 0;
    }
    if(n==1){
      count++;
      Queue_size++;
      scanf("%d\n",&tmp[Queue_size]);
    }
    else if(n==2){
      Extract(tmp);
    }
    else if(n==3){
      int second,third;
      scanf("%d %d",&second,&third);
      tmp[second] = third;
      Build_MaxHeap(tmp,Queue_size);
    }
    else
      return 0;
  }
  return 0;
}
