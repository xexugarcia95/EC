//  según la versión de gcc y opciones de optimización usadas, tal vez haga falta
//  usar gcc –fno-omit-frame-pointer si gcc quitara el marco pila (%ebp)

#include <stdio.h>	// para printf()
#include <stdlib.h>	// para exit()
#include <sys/time.h>	// para gettimeofday(), struct timeval

#define TEST		0
#define COPY_PASTE_CALC 1
//for((i=0;i<11;i++));do echo $i;./parity;done | pr -11 -l 20 -w 80
#if ! TEST
  #define NBITS 20
  #define SIZE (1<<NBITS) //tamaño suficiente para tiempo apreciable
  unsigned lista[SIZE]; //unsigned para que desplazamiento derecha sea lógico
  #define RESULT (1<<NBITS-1) //nueva fórmula
#else	
/*--------------------------------------------------------------------*/
  #define SIZE 4
  unsigned lista[SIZE]={0x80000000,0x00100000,0x00000800,0x00000001};
  #define RESULT 4
/*--------------------------------------------------------------------*

  #define SIZE 8
  unsigned lista[SIZE]={0x7fffffff,0xffefffff,0xfffff7ff,0xfffffffe,0x01000024,0x00356700,0x8900ac00,0x00bd00ef};
  #define RESULT 8
/*--------------------------------------------------------------------*

  #define SIZE 8
  unsigned lista[SIZE]={0x0,0x10204080,0x3590ac06,0x70b0d0e0,0xffffffff,0x12345678,0x9abcdef0,0xcafebeef};
  #define RESULT 2
/*--------------------------------------------------------------------*/
  #endif
  int resultado=0;

int parity1(int* array, int len)
{
    int  i,res=0,val,j;
    unsigned x;
    for (i=0; i<len; i++){
	x=array[i];
	val=0; //val libre encara iteración
	for(j=0;j<8*sizeof(unsigned);j++){
	 val^= x & 0x1; //acumular xor lateralmente
	 x>>=1;
	}
    res+=val; //acumular suma paridades
    }
    return res;
}

  
int parity2(int* array, int len)
{
    int  i,val,res=0;
    unsigned x;
    for (i=0; i<len; i++){
	x=array[i];
	val=0;
	do{
	 val^= x & 0x1;
	 x>>=1;
	}while(x);
    res+=val;
    }
    return res;
}

int parity3(int* array, int len)
{
    int  i,val,res=0;
    unsigned x;
    for (i=0; i<len; i++){
	x=array[i];
	val=0;
	while(x){
	 val^= x;
	 x>>=1;
	}
    
    res+=val & 0x1;
    }
    return res;
}  

int parity4(unsigned *array,int len)
{
  int i,val,res=0;
  unsigned x;
  for(i=0;i<len;i++){
    do{
      val ^=x;
      x >>=1;
    }while(x);
    asm(
     "ini3:	      		\n\t"	//traducción casi literal
     "   xor %[x],%[val] 	\n\t"	// sólo nos interesa LSB
     "   shr $1, %[x]	    		\n\t"
     "   test %[x], %[x]	      		\n\t"
     "	 jnz ini3			\n\t"
      : [val]"+r" (val)		// e/s:	empieza siendo cero
      : [x]"r"  (x)		// entrada: valor elemento
    );
    res+=val & 0x1;
    }
    return res;
}

int parity5(unsigned *array,int len)
{
  int i,j,val,res=0;
  unsigned x;
  for(i=0;i<len;i++){
    x=array[i];
    for(j=16;j==1;j/=2){
      x^= x>>j;
    }
    res+= x & 0x01;
  }

return res;
}

int parity6(unsigned* array,int len)
{
  int i,res=0;
  unsigned x;
  for(i=0;i<len;i++){
    x=array[i];
    asm(
		"mov	%[x], 	%%edx		\n\t"
		"shr	$16,	%%edx		\n\t"
		"xor	%[x],	%%edx		\n\t"
		"xor	%%dh,	%%dl		\n\t"
		"setpo  %%dl			\n\t"
		"movzx	%%dl,	%[x]		\n\t"
		: [x] "+r" (x) // input
		:
		: "edx"
    );
    res+=x;
  }
  return res;
}


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
    crono(parity1, "parity1 (lenguaje C -	     for)"); //cambiar nombres
    crono(parity2, "parity2 (lenguaje C -	   while)"); //y descripcion
    crono(parity3, "parity3 (1.CS:APP 3.22 - mask final)"); //Computer System: A programmer perspective. Hallaron
    crono(parity4, "parity4 (l.S:APP 3.49-group 8b   )");
    crono(parity5, "parity5 (asm SSE3 - pshufb 128b  )");
    crono(parity6,"parity6 (asm SSE4 - parity 32b  )");

#if ! COPY_PASTE_CALC
    printf("Calculando= %d\n", RESULT); 
#endif
    exit(0);
}
