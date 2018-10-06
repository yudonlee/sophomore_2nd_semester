/* 2017030119_Leeyudon Heapsort */
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
    if (arr[l] > arr[r] &&arr[parent]<arr[l])
      max = l;
    else if(arr[r]>arr[l] && arr[parent]< arr[parent]<arr[r])
      max =r;
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
void Heapsort(int* arr,int m,int n){
  Build_MaxHeap(arr,n);
  int j = n-m;
  for (int i= n;i>j;i--){  
    Swap(arr[1],arr[i]);
    n--;
    Heapify(arr,n,1);
    }
}

int main(){
  int n,m;
  scanf("%d %d",&n,&m);
  int* tmp = (int *)malloc(sizeof(int)*n+1);
  for(int i=1;i<n+1;i++)
    scanf("%d",&tmp[i]);
  Heapsort(tmp,m,n);
  int *p = tmp+n;
  while(p>(tmp+n-m)){
    printf("%d ",*p--);
  }
  printf("\n");
  for(int i=1;i<=n-m;i++)
    printf("%d ",tmp[i]);
  
  printf("\n");

  free(tmp);
  return 0;
}
