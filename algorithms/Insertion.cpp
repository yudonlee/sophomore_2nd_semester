#include <stdio.h>
#include <stdlib.h>
int main(){
  int n;
  scanf("%d",&n);
  int* tmp = (int *)malloc(sizeof(int)*n);
  for(int i=0;i<n;i++)
    scanf("%d",&tmp[i]);
  for(int i=1;i<n;i++){
    int temp =tmp[i];
    int j=i-1;
    while(temp>tmp[j] && j>=0){
      tmp[j+1] =tmp[j];
      --j;
    }
    tmp[j+1]=temp;
  }
  for(int j=0;j<n;j++)
    printf("%d\n",tmp[j]);
  free(tmp);
  return 0;
}
