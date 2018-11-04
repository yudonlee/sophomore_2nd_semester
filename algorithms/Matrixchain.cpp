#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void printParenthesis(int **s,int i,int j,int* element){
	if(i==j){
		printf("%d",element[i]);
		element++;
		return;
	}
	printf("(");
	printParenthesis(s,i,s[i][j],element);
	printParenthesis(s,s[i][j]+1,j,element);
	printf(")");
}
int min_number(int i,int j,int** m,int** s,int* p);
int make_matrix_chain(int n,int** m,int **s,int* p);
int main(){
	int n;
	scanf("%d",&n);
	int *p = (int*)malloc(sizeof(int)*n+1);
	for(int i=0;i<=n;i++){
		scanf("%d",&p[i]);
	}
	
	int **m = (int**)malloc(sizeof(int*)*n+1);
	for(int i=0;i<n+1;i++){
		m[i] = 	(int*)malloc(sizeof(int)*n+1);
		memset(m[i],0,sizeof(int)*n+1);
	}
	int **s = (int**)malloc(sizeof(int*)*n+1);
	for(int i=0;i<n+1;i++){
		s[i] = 	(int*)malloc(sizeof(int)*n+1);
		memset(s[i],0,sizeof(int)*n+1);
	}
	int minNum =make_matrix_chain(n,m,s,p);
	printf("%d\n",minNum);
	
	int* result = (int*)malloc(sizeof(int)*n+1);
	for(int i=1;i<=n;i++){
		result[i] = i;
	}
	printParenthesis(s,1,n,result); 
	return 0;	

}
int smallest_element_index(int work_array[], int max_j){
	int index = 0;
    int i;
   	for( i = 1; i < max_j; i++){
     	if(work_array[i] < work_array[index])
            index = i;
        }
        return index;
}
int min_number(int i,int j,int** m,int** s,int* p){
	int* result = (int*)malloc(sizeof(int)*(j-i));
	int k,count=0;
	for(k=i;k<j;k++){
		result[count]=m[i][k]+m[k+1][j]+p[i-1]*p[k]*p[j];
		count++;
	}
	count = smallest_element_index(result,j-i);
	s[i][j]=i+count;
	return result[count];
}
int make_matrix_chain(int n,int **m,int **s,int* p){
	int i=1;
	while(i<n){
		for(int k=1;k<n&&i+k<=n;k++){
			m[k][k+i]=min_number(k,i+k,m,s,p);
		}
		i++;
	}
	return m[1][n];
}
