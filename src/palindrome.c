#include<stdio.h>
int main()
{
	int n;
	int num[8];
	int cnt = 0 ;
	int i,j;
	int flag = 0;
	scanf("%d",&n);
	num[cnt] = n % 10;
	n = n / 10;
	cnt ++ ;
	while(n != 0){
		num[cnt] = n % 10;
		n =  n / 10;
		cnt++; 
	}
	for(i=0,j=cnt-1;i<=j;i++,j--){
		if(num[i] != num[j]){
			flag = 1;
			break;
		}
	}
	if(flag == 0){
		printf("Y");
	}else{
		printf("N");
	}
	return 0;
}
