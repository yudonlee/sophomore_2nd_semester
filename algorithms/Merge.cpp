#include <stdio.h>
#include <stdlib.h>
void Merge(int* A,int l,int m,int r){
  int* tmp = new int[r-l+1];
  int i =l,j=m+1;
  int k=0;
  while(i<=m&&j<=r){
    if(A[i] <=A[j])
      tmp[k++]=A[j++];
    else
      tmp[k++]=A[i++];
  }
  while(i<=m)
    tmp[k++]=A[i++];
  while(j<=r)
    tmp[k++]=A[j++];
  for(k=0,i=l;i<=r;++i,++k)
    A[i] = tmp[k];
  delete []tmp;
}
void MSort(int* A,int left,int right){
  int center;
  if(left<right){
    center = (left+right)/2;
    MSort(A,left,center);
    MSort(A,center+1,right);
    Merge(A,left,center,right);
  }
}
int main(){
  int n;
  scanf("%d",&n);
  int tmp[100000];
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  MSort(tmp,0,n-1);
  for(int j=0;j<n;j++)
    printf("%d\n",tmp[j]);
  return 0;
}
