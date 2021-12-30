#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define LC 8  // pois a matriz tem de ser quadrada
#define NP 4   // numero de processos
#define RND 100 // numeros randomicos da matriz

/*-----------------------------------------------*/
void gera_mat(int *mat)
{
	int l,c;
	for (l=0;l<LC;l++)
    {
        for (c=0;c<LC;c++)
        {
		    mat[(l*LC)+c] = rand()%RND;
        }
	}
}
/*-----------------------------------------------*/
void multi_mat(int *mat1, int *mat2, int *nmat,int id_seq){
	int lo,co,contmin,contmax,min, max,idp;
    int vetmin[LC];
    
    idp = id_seq;

    for (lo=idp;lo<LC;lo=(lo+NP))
    {
        for(co=0;co<LC;co++)
        {
            //------------------separação interno externo---------------//
            for (contmin=0;contmin<LC;contmin++)
            {
                if (mat1[lo*LC+contmin]> mat2[contmin*LC+co])
                //        0       0              0        0
                {
                    vetmin[contmin] = mat2[contmin*LC+co];
                }
                else
                {
                    vetmin[contmin] = mat1[lo*LC+contmin];
                }
            }    

            max = vetmin[0];

            for(contmax = 1;contmax<LC;contmax++)
            {
                if(vetmin[contmax]>max)
                {
                    max = vetmin[contmax];
                }
            }

            //------------------separação interno externo---------------//

            nmat[(lo*LC)+co] = max;

            
            /*  Tentativa 1
            for (li=0;li<LC;li++)
            {
                for (ci=0;ci<LC;ci++)
                {
                    if (mat1[(li*LC)+ci] > mat2[(ci*LC)+li])
                    {
                        min = mat2[(ci*LC)+li];
                    }
                    else 
                    {
                        min = mat1[(li*LC)+ci];
                    }
                    vetmin[ci] = min;
                }
                if (min>max)
                {
                    max = min;
                }
            }
            //------------------separação interno externo---------------//
            nmat[(lo*LC)+co] = max;*/
        }
    }
}
/*-----------------------------------------------*/
void escreve_mat(int *mat)
{
    int l,c;
	srand(time(NULL));		
	for (l=0;l<LC;l++)
    {
        for (c=0;c<LC;c++)
        {
            if((mat[(l*LC)+c])>=10)
            {
                printf("%d ", mat[(l*LC)+c]);
            }
            else
            {
                printf(" %d ", mat[(l*LC)+c]);
            }   
        }
        printf("\n");
	}
}
/*-----------------------------------------------*/
int main(){
	int *mat1=NULL,*mat2=NULL, *nmat=NULL;
    int pid, id_seq;
    int i, shmid, chave = 3;

    
    srand(time(NULL));
    mat1 = (int *)malloc((LC*LC)*sizeof(int));
    mat2 = (int *)malloc((LC*LC)*sizeof(int));

    shmid = shmget(chave, (LC*LC)*sizeof(int), 0600 | IPC_CREAT);
	nmat = (int *)shmat(shmid, 0, 0);

	gera_mat(mat1);
    gera_mat(mat2);


    id_seq = 0;
    for(i=1;i<NP;i++)
    {
        pid = fork();
        if ( pid == 0)
        {
            
            id_seq = i;
            break;
        } 
    }
	
    multi_mat(mat1,mat2,nmat,id_seq);

    if (id_seq != 0){
        shmdt(nmat);
    }
    else {
        for(i=1;i<NP;i++){
            wait(NULL);
        } 
        printf("Matriz 1--------------------------------------------------------------------------------------------------------------------------------------\n");   
        escreve_mat(mat1);
        printf("Matriz 2--------------------------------------------------------------------------------------------------------------------------------------\n");
        escreve_mat(mat2);
        printf("MAtriz R--------------------------------------------------------------------------------------------------------------------------------------\n");
        escreve_mat(nmat);
        shmdt(nmat);
        shmctl(shmid, IPC_RMID, 0);
    }
    free(mat1);
    free(mat2);  
}
/*-----------------------------------------------*/