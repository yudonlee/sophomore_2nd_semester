//Counting.cpp
//2018-2 algorithms Assignment
//Created by 2017030119_Leeyudon
#include <stdio.h>
#include <stdlib.h>
int main(){
  int N,M,K;
  int A[100001],B[100001];
  int input[1000001];
  int output[100001] = {0,};
  scanf("%d %d %d",&N,&M,&K);
  for(int i=1;i<=K;i++){
      scanf("%d %d",&A[i],&B[i]);
    }
  for(int i=1;i<=N;i++)
    scanf("%d",&input[i]);
  for(int i=1;i<=N;i++)
    output[input[i]]++;
  for(int i=2;i<=M;i++)
    output[i]=output[i]+output[i-1];
  for(int i=1;i<=K;i++)
    printf("%d\n",output[B[i]]-output[A[i]-1]);
  return 0;
}
