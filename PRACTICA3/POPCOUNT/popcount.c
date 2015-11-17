//  según la versión de gcc y opciones de optimización usadas, tal vez haga falta
//  usar gcc –fno-omit-frame-pointer si gcc quitara el marco pila (%ebp)

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval

#define TEST		0
#define COPY_PASTE_CALC 1

#if ! TEST
  #define NBITS 20
  #define SIZE (1<<NBITS) //tamaño suficiente para tiempo apreciable
  unsigned lista[SIZE]; //unsigned para que desplazamiento derecha sea lógico
  #define RESULT (NBITS*(1<<NBITS-1)) //nueva fórmula
#else
/*--------------------------------------------------------------------------*
  #define SIZE 4
  unsigned lista[SIZE]={0x80000000,0x00100000,0x00000800,0x00000001};
  #define RESULT 4
/*--------------------------------------------------------------------------*
  #define SIZE 
  unsigned lista[SIZE]={0x7fffffff,0xffefffff,0xfffff7ff,0xfffffffe,0x01000024,0x00356700,0x8900ac00,0x00bd00ef};
  #define RESULT 156
/*--------------------------------------------------------------------------*
  #define SIZE 8
  unsigned lista[SIZE]={0x0,0x10204080,0x3590ac06,0x70b0d0e0,0xffffffff,0x12345678,0x9abcdef0,0xcafebeef};
  #define RESULT 116
/*--------------------------------------------------------------------------*/
  #endif
  int resultado=0;

int popcount1(int* array, int len)
{
    int  i,res=0,j;
    unsigned x;
    for (i=0; i<len; i++)
	x=array[i];
	for(j=0;j<8*sizeof(unsigned);j++)
	 res+= x & 0x1;
	 x>>=1;
    return res;
}

  
int popcount2(int* array, int len)
{
    int  i,   res=0;
    unsigned x;
    for (i=0; i<len; i++)
	x=array[i];
	do{
	 res+= x & 0x1;
	 x>>=1;
	}while(x);
    return res;
} 

int popcount3(int* array, int len)
{
   int i,res=0;
   unsigned x;
   for(i=0;i<len;i++){
    x=array[i];
    asm(
 	"ini3:			\n\t"
	"shr       %[x] 	\n\t"
	"adc $0x0, %[r]		\n\t"
	"test %[x],%[x] 	\n\t"
	"jnz ini3 "
     	: [r] "+r" (res) // e/s -> inicialmente 0
	: [x] "r" (x)
    );
   }
   return res;
}

int popcount4(unsigned* array,int len)
{
	int i,j,val,res=0;
	unsigned x;
	for(i=0;i<len;i++){ 
	 x=array[i];
	 for(j=0;j<8;j++){
	 	val+= x & 0x01010101;
		x>>=1;
	 }
	 val += (val >> 16);
	 val += (val >> 8);
	 res += (val & 0xff);
	  	
	}
	
	return res;
}

int popcount5(unsigned* array,int len)
{
	int i, val, res = 0;
	int SSE_mask[]  = {0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f};
	int SSE_LUTb[]  = {0x02010100, 0x03020201, 0x03020201, 0x04030302};
			 // 	 3 2 1 0      7 6 5 4   1110 9 8    15141312
	if(len & 0x3) printf("leyendo 128b pero len no múltiplo de 4?\n");
	for (i=0; i<len; i+=4){
		asm("movdqu        %[x], %%xmm0 \n\t"
                                "movdqa  %%xmm0, %%xmm1 \n\t" // dos copias de x
                                "movdqu    %[m], %%xmm6 \n\t"// máscara
                                "psrlw           $4, %%xmm1 \n\t"
                                "pand    %%xmm6, %%xmm0 \n\t"//; xmm0 – nibbles inferiores
                                "pand    %%xmm6, %%xmm1 \n\t"//; xmm1 – nibbles superiores

                                "movdqu    %[l], %%xmm2 \n\t"//; ...como pshufb sobrescribe LUT
                                "movdqa  %%xmm2, %%xmm3 \n\t"//; ...queremos 2 copias
                                "pshufb  %%xmm0, %%xmm2 \n\t"//; xmm2 = vector popcount inferiores
                                "pshufb  %%xmm1, %%xmm3 \n\t"//; xmm3 = vector popcount superiores

                                "paddb   %%xmm2, %%xmm3 \n\t"//; xmm3 - vector popcount bytes
                                "pxor    %%xmm0, %%xmm0 \n\t"//; xmm0 = 0,0,0,0
                                "psadbw  %%xmm0, %%xmm3 \n\t"//;xmm3 = [pcnt bytes0..7|pcnt bytes8..15]
                                "movhlps %%xmm3, %%xmm0 \n\t"//;xmm3 = [             0           |pcnt bytes0..7 ]
                                "paddd   %%xmm3, %%xmm0 \n\t"//;xmm0 = [ no usado        |pcnt bytes0..15]
                                "movd    %%xmm0, %[val] \n\t"
                                : [val]"=r" (val)
                                : [x] "m" (array[i]),
                                [m] "m" (SSE_mask[0]),
                                [l] "m" (SSE_LUTb[0])
                );


		   res += val;
		}

	return res;
}

//Versión SSE4.2 (popcount)
int popcount6(unsigned* array, int len)
{
	int i,val,result=0;
	unsigned x;
	for(i=0;i<len;i++){
	  x=array[i];
	  asm("popcnt %[x], %[val]"
	   : [val] "=r" (val)
           :   [x]  "r" (x)	
	  );
	  result+=val;
	}
        return result;
}

//popcount 64bit p/mejorar prestaciones
int popcount7(unsigned* array,int len)
{
	int i,val,result=0;
	unsigned x1,x2;
	if(len & 0x1)
	   printf("leer 64b y len impar=\n");
    	for(i=0;i<len;i+=2){
	  x1=array[i];
	  x2=array[i+1];
	  asm("popcnt %[x1], %[val] \n\t"
	      "popcnt %[x2], %[val] \n\t"
	      "add    %%edi, %[val] \n\t"
	   : [val] "=&r" (val)
	   :  [x1]   "r" (x1),
	      [x2]   "r" (x2)
 	   : "edi");
	  result+=val;
	}
	return result;
}

//popcount 4 -> libro hallaron problema 3.49 resuelto p.364



void crono(int (*func)(), char* msg){
    struct timeval tv1,tv2;	// gettimeofday() secs-usecs
    long           tv_usecs;	// y sus cuentas

    gettimeofday(&tv1,NULL);
    resultado = func(lista, SIZE);
    gettimeofday(&tv2,NULL);

    tv_usecs=(tv2.tv_sec -tv1.tv_sec )*1E6+
             (tv2.tv_usec-tv1.tv_usec);
#if COPY_PASTE_CALC				//Cuando hagamos c-p a Calc
    printf(	"%ld" "\n",	tv_usecs);	// querremos sólo los números
#else
    printf("resultado= %d\t", resultado); 	//durante desarrollo
    printf("%s:%9ld us\n", msg, tv_usecs);	// comprobar resultado OK
#endif
}

int main()
{
#if !TEST
    int i;			// inicializar array
    for (i=0; i<SIZE; i++)	// se queda en cache
	 lista[i]=i;
#endif
    crono(popcount1, "popcount1 (lenguaje C -	      for)"); 
    crono(popcount2, "popcount2 (lenguaje C -	    while)"); 
    crono(popcount3, "popcount3 (leng.ASM   -cuerpo while)");
    crono(popcount4, "popcount4 (l.S:APP 3.49-group 8b   )");
    crono(popcount5, "popcount5 (asm SSE3 - pshufb 128b  )");
    crono(popcount6,"popcount6 (asm SSE4 - popcount 32b  )");
    crono(popcount7,"popcount7 (asm SSE4 - popcnt 2x32b  )");
 
#if ! COPY_PASTE_CALC
    printf("Calculando= %d\n", RESULT); 
#endif
    exit(0);
}
