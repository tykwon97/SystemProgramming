#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //sysV
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <unistd.h> //fork()
#include <stdlib.h> // sleep()



void makeMatrix(int ** matrix, int X, int Y);

int main (int argc, char** argv) 
{
	int n = atoi(argv[1]);
	int X = n;
	int Y = n;
	
	//count_total = fork횟수
	int count = n -3 + 1;
	int count_total =count * count;

	//filter배열 생성
	int filter[3][3] = {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};
	
	//2차원 행렬 Memory Allocation
	int **sample1;
	sample1 = (int**)malloc(sizeof(int*) * X);
	for(int i = 0; i<X;i++){
		sample1[i] = (int*)malloc(sizeof(int)*Y);
	}

	//2차원 행렬 Memory Allocation
	int **matrix1;
	matrix1=(int**)malloc(sizeof(int*)*count);
	for(int i =0;i<count;i++){
		matrix1[i]=(int*)malloc(sizeof(int)*count);
	}
	
	int m=count/2;//maxpooling
	//2차원 행렬 Memory Allocation
	int **matrix2;
	matrix2=(int**)malloc(sizeof(int*)*m);
	for(int i =0;i<m;i++){
		matrix2[i]=(int*)malloc(sizeof(int)*m);
	}

	//2차원 행렬 데이터 생성
	makeMatrix(sample1,X,Y);
	//for(int i = 0;i<X;i++)
	//{
	//	for(int j=0;j<X;j++)
	//	{
	//		printf("%d ",sample1[i][j]);
	//	}
	//	printf("\n");
	//}

		
	if(fork()==0)//자식프로세스
	{
		
		key_t ipckey;
		int mqdes,i;
		size_t buf_len;
		int MAX_ID=count_total;
		int x,y;//배열 위치
		int total=0;//총합
		
		struct
		{
			long id;
			int value;
		} mymsg; //struct
		
		buf_len=sizeof(mymsg.value);


		ipckey=ftok("./tmp/foo",1946);
		mqdes=msgget(ipckey,IPC_CREAT|0600);
		if(mqdes<0)
		{
			perror("msgget()");
			exit(0);
		}//if
		for(i=0;i<MAX_ID;i++)
		{
			if(fork()==0)//자식프로로세스
			{		
				mymsg.id=i+1;
				x=i/count;
				y=i%count;
				for(int j=0;j<3;j++)
				{
					for(int k=0;k<3;k++)
					{
						//printf("%d :filter[%d][%d] : %d ",i,j,k,filter[j][k]);
						//printf("%d :sample1[%d][%d] :%d ",i,x+j,y+k,sample1[x+j][y+k]);
						
						total+=(filter[j][k]*sample1[x+j][y+k]);	
						//printf("%d 계산값 : %d ",i,filter[j][k]*sample1[x+j][y+k]);
						//printf("total : %d\n",total);
					}
				}//for
				mymsg.value=total;
				//printf("%d mymsg.value ==============%d\n",i,mymsg.value);
				if(msgsnd(mqdes,&mymsg,buf_len,0)==-1)
				{
					perror("msgsnd()");
					exit(0);
				}//if
				//printf("자식프로세스 send성공%ld\n",mymsg.id);
				exit(0);
			}//if
			else//부모프로세스
			{
				//exit(0);	
			}//else
			
		}//for
		//return 0;
		exit(0);
	}//if
	else//부모프로세스
	{
		key_t ipckey;
		int mqdes,i;
		size_t buf_len;
		int MAX_ID=count_total;
		int x,y;//배열 위치
		
		struct
		{
			long id;
			int value;
		} mymsg;

		buf_len=sizeof(mymsg.value);
		ipckey=ftok("./tmp/foo",IPC_CREAT|0600);
		mqdes=msgget(ipckey,IPC_CREAT|0600);
		if(mqdes<0)
		{
			perror("msgget()");
			exit(0);
		}//if
		
		for (i=0;i<MAX_ID;i++)
		{
			if(msgrcv(mqdes,&mymsg,buf_len,i+1,0)== -1)
			{
				perror("msgrcv()");
				exit(0);
			}
			else
			{
				//printf("val : %d, id : %ld\n",mymsg.value,mymsg.id);
				x=(mymsg.id-1)/count;
				y=(mymsg.id-1)%count;
				matrix1[x][y]=mymsg.value;
			}	
		}
		//return 0;
	}//else
	
	//matrix1결과
	//printf("matrix1\n");
	//for (int i=0;i<count;i++)
	//{
	//	for(int j=0;j<count;j++)
	//	{
	//		printf("%d ",matrix1[i][j]);
	//	}
	//	printf("\n");
	//}
	
	if(fork()==0)//자식프로세스
	{
		key_t ipckey;
		int mqdes,i;
		size_t buf_len;
		int MAX_ID=count_total/4;
		int x,y;//배열위치
		int total=0;//총합
		struct
		{
			long id;
			int value;
		} mymsg; //struct

		buf_len=sizeof(mymsg.value);

		ipckey=ftok("./tmp/kwon",1997);
		mqdes=msgget(ipckey,IPC_CREAT|0600);
		if(mqdes<0)
		{
			perror("msgget()");
			exit(0);
		}//if
		for(i=0;i<MAX_ID;i++)
		{
			if(fork()==0)
			{
				int tmp;
				int max=0;
				mymsg.id=i+1;
				x=(i/m)*2;
				y=(i%m)*2;
				
				for(int j=0;j<2;j++)
				{
					for(int k=0;k<2;k++)
					{
						if(j==0 && k==0)
						{
							tmp=matrix1[x+j][y+k];
							max=matrix1[x+j][y+k];
						}
						tmp=matrix1[x+j][y+k];
						if(tmp>max)
							max=tmp;	
					}//for
				}//for
				mymsg.value=max;
				if(msgsnd(mqdes,&mymsg,buf_len,0)==-1)
				{
					perror("msgsnd()");
					exit(0);
				}//if
				exit(0);
			}//if
			else
			{
				//exit(0);
			}//else	
		}//for
		exit(0);
	}//if
	else
	{
		key_t ipckey;
		int mqdes,i;
		size_t buf_len;
		int MAX_ID;
		int x,y;
		MAX_ID=count_total/4;

		struct
		{
			long id;
			int value;
		}mymsg;

		buf_len=sizeof(mymsg.value);
		ipckey=ftok("./tmp/foo",IPC_CREAT|0600);
		mqdes=msgget(ipckey,IPC_CREAT|0600);
		if(mqdes<0)
		{
			perror("msgget()");
			exit(0);
		}//if
		for(i=0;i<MAX_ID;i++)
		{
			if(msgrcv(mqdes,&mymsg,buf_len,i+1,0)==-1)
			{
				perror("msgrcv()");
				exit(0);
			}
			else
			{
				//printf("receive val : %d, id : %ld\n",mymsg.value,mymsg.id);
				x=(mymsg.id-1)/m;
				y=(mymsg.id-1)%m;
				matrix2[x][y]=mymsg.value;
			}
		}
	}//else
	
	//printf("matrix2\n");
	for(int i=0;i<m;i++)
	{

		for(int j=0;j<m;j++)
		{
			printf("%d ",matrix2[i][j]);
		}
	}

	//2차원 행렬Free
	for(int i=0;i<X;i++){
		free(sample1[i]);
	}
	free(sample1);

	for(int i=0;i<count;i++){
		free(matrix1[i]);
	}
	free(matrix1);

	for(int i=0;i<m;i++){
		free(matrix2[i]);
	}
	free(matrix2);


	//sleep(2)//자식프로세스 활동하는동안
	return 0;;

} //main문
