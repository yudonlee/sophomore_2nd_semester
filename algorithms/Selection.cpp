//2017030119_Leeyudon Selection.cpp
#include <stdio.h>
#include <stdlib.h>
void Swap(int& A,int& B){
  int t;
  t =A;
  A=B;
  B=t;
}
void SelectionSort(int* input,int n,int m){
  for(int i=0;i<m;i++){
    int rad=i;
    for(int j=i+1;j<n;j++){
      if(input[j]<input[rad])
	rad=j;
    }
    Swap(input[i],input[rad]);
  }
  
  /*for(int i=m;i<n;i++){
    int rad=i;
    for(int j=i+1;j<n;j++){
      if(input[j]<input[rad])
	rad=j;
    }
    Swap(input[i],input[rad]);
    }*/
}

int main(){
  int n,m;
  scanf("%d %d",&n,&m);
  int* tmp = (int *)malloc(sizeof(int)*n);
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  SelectionSort(tmp,n,m);
  for(int j=0;j<n;j++)
    printf("%d\n",tmp[j]);
  free(tmp);
  return 0;
}
