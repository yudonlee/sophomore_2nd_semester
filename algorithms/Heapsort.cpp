/* 2017030119_Leeyudon Heapsort */
#include <stdio.h>
#include <stdlib.h>
#define Parent(a) (a-1)/2;
#define Left(a)  2*a+1;
#define Right(a) 2*a+2;
void Swap(int& a,int& b){
  int t;
  t=a;
  a=b;
  b=t;
}
void Heapify(int* arr,int n,int i){
  int l = Left(i);
  int r = Right(i);
  int max =i;
  if(arr[i]<arr[l] && l<n)
    max = l;
  else
    max = i;
  if(arr[max]<arr[r] && r<n)
    max = r;
  if(max != i){
    Swap(arr[i],arr[max]);
    Heapify(arr,n,max);
  }
}
void Build_MaxHeap(int* arr,int n){
  for (int j=n/2-1;j>= 0;j--){
     Heapify(arr,n,j);
  }
}
void Heapsort(int* arr,int n){
  Build_MaxHeap(arr,n);
  for (int i= n-1;i>0;i--){  
    Swap(arr[0],arr[i]);
    n--;
    Heapify(arr,n,0);
    }
}

int main(){
  int n,m;
  scanf("%d %d",&n,&m);
  int* tmp = (int *)malloc(sizeof(int)*n);
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  Heapsort(tmp,n);
  int *p = tmp+n-1;
  while(p>=tmp){
    printf("%d ",*p--);
    if(*p==tmp[n-m-1])
       printf("\n");
  }
  free(tmp);
  return 0;
}
