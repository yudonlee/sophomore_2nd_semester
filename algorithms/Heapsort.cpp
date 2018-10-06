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

int main(){
  int n,k;
  scanf("%d %d",&n,&k);
  int* tmp = (int *)malloc(sizeof(int)*n);
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  for (int j=n/2-1;j>= 0;j--){
    Heapify(tmp,n,j);
  }
  int count = 0;
  int j = n-1;
  int l = n- k-1;
  for (j; j > l; --j) {
    count++;
    printf("%d ", tmp[0]);
    Swap(tmp[0], tmp[j]);
    n--;
    Heapify(tmp,n, 0);
  }
  
  printf("\n");
  for (int i = 0; i <n; ++i) {
    printf("%d ", tmp[i]);
  }
  

  return 0;
}
