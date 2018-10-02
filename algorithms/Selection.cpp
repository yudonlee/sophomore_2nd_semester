//2017030119_Leeyudon Selection.cpp
#include <stdio.h>
#include <stdlib.h>
void Swap(int& A,int& B){
  int t;
  t =A;
  A=B;
  B=t;
}
int main(){
  int n,m;
  scanf("%d %d",&n,&m);
  int* tmp = new int[n];
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  for(int i=0;i<m;i++){
    int rad=i;
    for(int j=i+1;j<n;j++){
      if(tmp[j]<tmp[rad])
	rad=j;
    }
    Swap(tmp[i],tmp[rad]);
  }
  for(int j=0;j<n;j++)
    printf("%d\n",tmp[j]);
  free(tmp);
  return 0;
}
