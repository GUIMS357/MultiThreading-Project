#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* define inteiros de tamanho específico */
#include <pthread.h>


#pragma pack(1) /* diz pro compilador não alterar alinhamento 
                        ou tamanho da struct */


#define M 5000
#define N 5000

    

// /---------------------------------------------------------------------/
struct cabecalho {
	unsigned short tipo;
	unsigned int tamanho_arquivo;
	unsigned short reservado1;
	unsigned short reservado2;
	unsigned int offset;
	unsigned int tamanho_image_header;
	int largura;
	int altura;
	unsigned short planos;
	unsigned short bits_por_pixel;
	unsigned int compressao;
	unsigned int tamanho_imagem;
	int largura_resolucao;
	int altura_resolucao;
	unsigned int numero_cores;
	unsigned int cores_importantes;
}; 
typedef struct cabecalho CABECALHO;

// /---------------------------------------------------------------------/
struct rgb{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};
typedef struct rgb RGB;

struct parametros{
	int id;
	int nthr;
	int tm;
    int v;
    CABECALHO *bH;
};

typedef struct parametros PARAMETROS;

RGB matrizI[M][N];  
RGB matrizO[M][N];


// ---------------------------------------------------------------------//


int valormed(int *vetmed, int tm)
{
    int aux;
    int vret;
    int n = tm*tm;
    int Pcentral = (tm*tm)%2+1;
    //printf("pré sort   ");
    
        for (int j = 0; j <= n - 1; j++) {

            if (vetmed[j] > vetmed[j + 1]) {
                aux = vetmed[j];
                vetmed[j] = vetmed[j + 1];
                vetmed[j + 1] = aux;
            }
        }
    //printf("pos sort    ");
    vret = vetmed[Pcentral];
    return vret;// aqui tem q vir o valor medio do vetor direto já pronto
}



void * filter_median(void *args) {
    PARAMETROS *par = (PARAMETROS *)args;

    int tm = par->tm;
    int v = par->v;
    int id = par->id;
    int nthr = par->nthr;
    CABECALHO *bH = par->bH;

    int vetmedr[tm*tm];
    int vetmedg[tm*tm];
    int vetmedb[tm*tm];

    int cvetmed, cma, cml;


    int i = 0, j = 0;

    //printf("inicio mediana  ");

    for (i = (id+v); i < (bH->altura - v); i+=nthr){
        for ((j = v); j < (bH->largura - v); j++)
        {
            cvetmed = 0;
            //laço interno
            //printf("passa pro interno mediana   ");

            for(cma=(i-v); cma <= (i+v); cma++)
            {
                for(cml=(j-v); cml <= (j+v); cml++)
                {
                    vetmedr[cvetmed] = matrizI[cma][cml].red;
                    vetmedg[cvetmed] = matrizI[cma][cml].green;
                    vetmedb[cvetmed] = matrizI[cma][cml].blue;
                    cvetmed++;
                    //printf("carrega vetor   ");
                    
                }
            }
            matrizO[i][j].red = valormed(vetmedr,tm);
            matrizO[i][j].green = valormed(vetmedg,tm);
            matrizO[i][j].blue = valormed(vetmedb,tm);
            //printf("printou na mat    ");
        }
    }
}

int main(int argc, char **argv){

    int x;
    int y;
    int tm; // configurar com 3 para 3x3, 5 para 5x5 e 7 para 7x7
    int mpa;
    int mpl;
    int cma;
    int cml;
    int cvetmed;
    char *imgpath;
    
    int nthr;
	pthread_t *tid = NULL;
    PARAMETROS *par = NULL;

    FILE *imagemI;
    FILE *imagemO;

    if ( argc != 4 ){
		printf("%s < YM > < T > < FILE > \n", argv[0]); 
		exit(0);
	}

    tm = atoi(argv[1]); // tamanho da mascara
	nthr = atoi(argv[2]); // Número de treads
    imgpath = (argv[3]); // Nome da IMG

    tid = (pthread_t *)malloc(nthr * sizeof(pthread_t));
    par = (PARAMETROS *)malloc(nthr * sizeof(PARAMETROS));

    imagemI = fopen(imgpath,"rb");  
    
    if ( imagemI == NULL ){
        printf("Erro ao abrir a imagem de entrada\n");
        exit(0);
    }

    imagemO = fopen("img_saida.bmp", "wb");

    if ( imagemO == NULL ){
        printf("Erro ao criar a imagem de saida\n");
        exit(0);
    }

    CABECALHO bH;

    fread(&bH, sizeof(CABECALHO), 1, imagemI);
    
    int v; // numero de casas vizinhas 1, 2, 3

    if (tm == 3){
        v = 1;
    }
    else if (tm == 5) { 
        v = 2;
    }
    else if (tm == 7) {
        v = 3;
    }

    for (x = 0; x < bH.altura; x++) 
    {
        for (y = 0; y < bH.largura; y++) 
        {
            fread(&matrizI[x][y],sizeof(RGB),1,imagemI);
        }
    }
    fclose(imagemI);
    for(int i=0; i<nthr; i++ ){
        par[i].id = i;
		par[i].nthr = nthr;
		par[i].bH = &bH;
		par[i].tm = tm;
        par[i].v = v;
		pthread_create(&tid[i], NULL, filter_median, (void *) &par[i]); 
    }
    for (int i=0; i<nthr; i++ ){
		pthread_join(tid[i], NULL);
	}
    
    fwrite(&bH, sizeof(CABECALHO), 1, imagemO);   
    for (x = 0; x < bH.altura; x++) 
    {
        for (y = 0; y < bH.largura; y++) 
        {
            fwrite(&matrizO[x][y],sizeof(RGB),1,imagemO);
        }
    }
    fclose(imagemO);
   
}