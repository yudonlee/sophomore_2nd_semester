#include <stdio.h>
#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
int bottom_up_cut_rod(int*p ,int n,int *cutIndex);
int main(){
  int n;
  scanf("%d",&n);
  int* arr = new int[n+1];
  int* cutIndex = new int[n+1];
  arr[0] =0;
  cutIndex[0] = 0;
  for(int i=1;i<=n;i++)
    scanf("%d",&arr[i]);
  int result=bottom_up_cut_rod(arr,n,cutIndex);
  printf("%d\n",result);
  if(cutIndex[n]==n){
    printf("%d\n",cutIndex[n]);
    return 0;
  }
  while(cutIndex[n]!=n){
    printf("%d ",cutIndex[n]);
    n = n-cutIndex[n];
  }
  if(n==cutIndex[n])
    printf("%d\n",cutIndex[n]);
  return 0;
}
int bottom_up_cut_rod(int* p,int n,int *cutIndex){
  int* right = new int[n+1];
  int q;
  for(int j=0;j<=n;j++){
    q =p[j];
    cutIndex[j] = j;
    for(int i=1;i<j/2+1;i++){
      int a = max(q,p[i]+right[j-i]);
      if(a!=q){
	q=a;
	cutIndex[j] = i;
      }
	
       }
    right[j] =q;
  }
  return right[n];
}
