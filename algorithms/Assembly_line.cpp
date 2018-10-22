#include <stdio.h>
#define MIN(x,y) (((x)<=(y)) ? (x):(y))
int main(){
	int table[3][3];
	int n;
	scanf("%d",&n);
	int e1,e2;
	scanf("%d %d",&e1,&e2);
	int x1,x2;
	scanf("%d %d",&x1,&x2);
	int*  a1 = new int[n+1];
	for(int i=1;i<n+1;i++)
		scanf("%d",&a1[i]);
	int*  a2 = new int[n+1];
	for(int i=1;i<n+1;i++)
		scanf("%d",&a2[i]);
	int*  t1 = new int[n];
	for(int i=1;i<n;i++)
		scanf("%d",&t1[i]);	
	int*  t2 = new int[n];
	for(int i=1;i<n;i++)
		scanf("%d",&t2[i]);
	int* tableL1 = new int[n+1];
	int* tableL2 = new int[n+1];
	table[1][1]=a1[1]+e1;
	table[2][1]=a2[1]+e2;
	for(int i=1;i<n;i++){
		if(i!=1){
			table[1][1]=table[1][2];
			table[2][1]=table[2][2];
			
		}
		if(table[1][1]+a1[i+1]<=table[2][1]+t2[i]+a1[i+1])
			tableL1[i+1]=1;
		else
			tableL1[i+1]=2;
		if(table[2][1]+a2[i+1]<=table[1][1]+t1[i]+a2[i+1])
			tableL2[i+1]=2;
		else
			tableL2[i+1]=1;
		table[1][2]=MIN(table[1][1]+a1[i+1],table[2][1]+t2[i]+a1[i+1]);
		table[2][2]=MIN(table[2][1]+a2[i+1],table[1][1]+t1[i]+a2[i+1]);
	}
	
	printf("%d\n",MIN(table[1][2]+x1,table[2][2]+x2));
	int* result = new int[n+1];
	int start;
	if(table[1][2]+x1<=table[2][2]+x2)
		start=1;
	else
		start=2;
	result[n]=start;
	for(int i=n;i>1;i--){
		if(result[i]==1){
			result[i-1]= tableL1[i];
		}
		else{
			result[i-1] = tableL2[i];
		}
	}
	for(int i=1;i<=n;i++){
		printf("%d %d\n",result[i],i);
	}
	return 0;
}
