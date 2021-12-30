#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* define inteiros de tamanho específico */
#include <omp.h>

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

typedef struct parametros PARAMETROS;

RGB matrizI[M][N];  
RGB matrizO[M][N];

// ---------------------------------------------------------------------//


int comparador(const void *a, const void *b) {

    unsigned char f = *((unsigned char *)a);
    unsigned char s = *((unsigned char *)b);
    if (f > s)
        return 1;
    if (f < s)
        return -1;
    return 0;
}


void filter_median(int tm, int v,int nthr, int alt, int larg){


    int tamanho = tm * tm;
    int id;

    unsigned char vetmedr[tamanho];
    unsigned char vetmedg[tamanho];
    unsigned char vetmedb[tamanho];

    int cvetmed, cma, cml;


    int i = 0, j = 0;

    omp_set_num_threads(nthr);

	#pragma omp parallel private (i,j,cma, cml,cvetmed,vetmedr, vetmedg, vetmedb)
    {
        id = omp_get_thread_num();

        for (i = (id+v); i < (alt - v); i+=nthr){
            for ((j = v); j < (larg - v); j++)
            {
                cvetmed = 0;
                cma = 0;
                cml = 0;
                for(cma = i-v; cma <= i+v ; cma++)
                {
                    for(cml= j-v ; cml <= j+v; cml++)
                    {
                        vetmedr[cvetmed] = matrizI[cma][cml].red;
                        vetmedg[cvetmed] = matrizI[cma][cml].green;
                        vetmedb[cvetmed] = matrizI[cma][cml].blue;
                        cvetmed++;                    
                    }
                }

                qsort(vetmedr, sizeof(vetmedr) / sizeof(*vetmedr), sizeof(*vetmedr), comparador);
                qsort(vetmedg, sizeof(vetmedg) / sizeof(*vetmedg), sizeof(*vetmedg), comparador);
                qsort(vetmedb, sizeof(vetmedb) / sizeof(*vetmedb), sizeof(*vetmedb), comparador);

                int mediana = tamanho / 2 + tamanho % 2;
                matrizO[i][j].red = vetmedr[mediana];
                matrizO[i][j].green = vetmedg[mediana];
                matrizO[i][j].blue = vetmedb[mediana];
            }
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
    int alt;
    int larg;
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
    else if (tm == 9) {
        v = 4;
    }
    else if (tm == 11) {
        v = 5;
    }
    else if (tm == 13) {
        v = 6;
    }

    for (x = 0; x < bH.altura; x++) 
    {
        for (y = 0; y < bH.largura; y++) 
        {
            fread(&matrizI[x][y],sizeof(RGB),1,imagemI);
        }
    }
    fclose(imagemI);
    
    alt = bH.altura;
    larg = bH.largura;

    filter_median(tm,v,nthr,alt,larg);
 
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