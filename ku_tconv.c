#include <stdio.h>
#include <string.h>//strcpy
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>//atoii

pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;


typedef struct matrix
{
	int** sample; //계산 전 matrix
	int** matrix; //계산 후 matrix
	int filter[3][3];
	int i; //순서
	int count; //배열 크기
}matrix;


void* convolution(void* m)
{
	int x;
	int y;
	int count;
	int total=0;
	pthread_mutex_lock(&mutex);

	matrix* ma=(matrix*)m;
	count=ma->i;
	x=ma->i/ma->count;
	y=ma->i%ma->count;
	for(int i=0;i<3;i++) //convolution 계산
		for(int j=0;j<3;j++)
			total+=ma->sample[i+x][j+y]*ma->filter[i][j];
	ma->matrix[x][y]=total; //계산 값 저장
	ma->i = ma->i +1; // 다음 배열 계산
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void* maxpooling(void* m)
{
	int x;
	int y;
	int count;
	int max=0;
	pthread_mutex_lock(&mutex);

	matrix* ma=(matrix*)m;
	count=ma->i;
	x=(ma->i/ma->count)*2;
	y=(ma->i%ma->count)*2;
	for(int i=0; i<2;i++) //maxpooling 계산
	{
		for(int j=0;j<2;j++)
		{
			if(i==0 & j==0)
				max=ma->sample[x][y];
			if(max<=ma->sample[i+x][j+y])
				max=ma->sample[i+x][j+y];
		}

	}
	ma->i = ma->i +1; //다음 배열 계산
	ma->matrix[x/2][y/2]=max; //계산 값 저장
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main(int argc, char** argv)
{	
	int fd;
	char buffer[2];
	int n1=0;
	int n2=0;
	int n=0;
	int num1=0;
	int num2=0;
	int x=0;
	int y=0;


	fd=open(argv[1],O_RDONLY);//dataset불러오기	
	
	read(fd,buffer,1);
	n1=atoi(buffer);
	read(fd,buffer,1);
	n2=atoi(buffer);

	n=n1*10+n2;//배열 크기 계산
	//printf("num = %d\n",n);	

	int filter[3][3] = {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};
	
	//2차원 행렬 Memory Allocation
	int **sample;
        sample=(int**)malloc(sizeof(int*)*n);
        for(int i=0;i<n;i++)
        	sample[i]=(int*)malloc(sizeof(int)*n);

	if(n>=10) 
		lseek(fd,3,SEEK_SET);
	else //한자리인경우
		lseek(fd,2,SEEK_SET);

	for(int i=0;i<n*n;i++)
	{	
		read(fd,buffer,1);//두자리 불러오기
		num1=atoi(buffer);
		read(fd,buffer,1);
		num2=atoi(buffer);
			
		x=i/n;
		y=i%n;

		sample[x][y]=(num1*10+num2);
		lseek(fd,1,SEEK_CUR);//빈칸 계산
	}
	close(fd);

	//for(int i=0;i<n;i++){
	//	for(int j=0;j<n;j++){
	//		printf("%d ",sample[i][j]);
	//	}
	//	printf("\n");
	//}
	
	n=n-2;//convolution layer배열(크기 2 작아짐)

	int** matrix1;
	matrix1=(int**)malloc(sizeof(int*)*n);
	for(int i=0;i<n;i++)
		matrix1[i]=(int*)malloc(sizeof(int)*n);
	int count;
	count=n;
	n=n*n;
	pthread_t thread_id[n];
	int status;
	
	struct matrix m;
	m.sample=sample;
	m.matrix=matrix1;
	m.count=count;
	m.i=0;

	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			m.filter[i][j]=filter[i][j];

	for(int i=0;i<n;i++)//쓰레드 생성
		status=pthread_create(&thread_id[i],NULL,convolution,&m);	
	
	for(int i=0;i<n;i++)//쓰레드 wait
		pthread_join(thread_id[i],NULL);
	
	//printf("\n");
	//for(int i=0;i<count;i++){
	//	for(int j=0;j<count;j++){
	//		printf("%d ",matrix1[i][j]);
	//	}
	//	printf("\n");
	//}
	
	count=count/2;
	int** matrix2;//maxpooling배열 (크기 반으로 줄어듬)
	matrix2=(int**)malloc(sizeof(int*)*count);
	for(int i=0;i<count;i++)
		matrix2[i]=(int*)malloc(sizeof(int)*count);

	m.sample=matrix1;
	m.matrix=matrix2;
	m.count=count;
	m.i=0;
	
	n=count*count;
	pthread_t thread[n];

	for(int i=0;i<n;i++)//쓰레드 생성
		status=pthread_create(&thread[i],NULL,maxpooling,&m);
	
	for(int i=0;i<n;i++)//쓰레드 wait
		pthread_join(thread[i],NULL);

	fd=open(argv[2],O_WRONLY|O_CREAT);//저장할장소 불러오기
	char buf[10];//버퍼
	char number[10];//정수를 char형으로 변환하기 위한 배열
	int total;
	for(int i=0;i<count;i++)
	{
		for(int j=0;j<count;j++)
		{
			total=matrix2[i][j];
			if(-1000<total && total<-99){
				//printf("%d ",matrix2[i][j]);
				total=total*-1;//음수인경우
				sprintf(number,"-%d ",total);
				strcpy(buf,number);
				write(fd,buf,5);//파일에 write
			}
			else if(-100<total && total<-9){
				//printf(" %d ",matrix2[i][j]);
				total=total*-1;
				sprintf(number," -%d ",total);
				strcpy(buf,number);
				write(fd,buf,5);
			}
			else if(-10<total && total<0){
				//printf("  %d ",matrix2[i][j]);
				total=total*-1;
				sprintf(number,"  -%d ",total);
				strcpy(buf,number);
				write(fd,buf,5);
			}
			else if(0<=total && total<10){
				//printf("   %d ",matrix2[i][j]);
				sprintf(number,"   %d ",total);
				strcpy(buf,number);
				write(fd,buf,5);
			}
			else if(9<total && total<100){
				//printf("  %d ",matrix2[i][j]);
				sprintf(number,"  %d ",total);
				strcpy(buf,number);
				write(fd,buf,5);
			}
			else{
				//printf(" %d ",matrix2[i][j]);	
				sprintf(number," %d ",total);
				strcpy(buf,number);
				write(fd,buf,5);
			}
		
			if(i==count-1 && j==count-1)
				break;
		}
		//printf("\n");
		sprintf(buf,"%s","\n");//한칸 띄어서 저장
		write(fd,buf,2);
	}
	return 0;
}//main문
