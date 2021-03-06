.section .data
	.macro linea 			#bateria de ejemplos sin signo
#		.int 1,1,1,1
#		.int 2,2,2,2
#		.int 1,2,3,4
#		.int -1,-1,-1,-1
#		.int 0xffffffff,0xffffffff,0xffffffff,0xffffffff
#		.int 0x08000000,0x08000000,0x08000000,0x08000000
#		.int 0x10000000,0x20000000,0x40000000,0x80000000

#		 bateria de ejemplos con signo

		.int -1,-1,-1,-1
#		.int 1,-2,1,-2
#		.int 1,2,-3,-4
#		.int 0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF
#		.int 0x80000000,0x80000000,0x80000000,0x80000000
#		.int 0x04000000,0x04000000,0x04000000,0x04000000
#		.int 0x08000000,0x08000000,0x08000000,0x08000000
#		.int 0xFC000000,0xFC000000,0xFC000000,0xFC000000
#		.int 0xF8000000,0xF8000000,0xF8000000,0xF8000000
#		.int 0xF0000000,0xE0000000,0xE0000000,0xE0000000
	.endm
lista: .irpc i,12345678
		linea
       .endr		
#		.int 1,1,1,1, 1,1,1,1
#		.int 1,1,1,1, 1,1,1,1
#		.int 1,1,1,1, 1,1,1,1
#		.int 1,1,1,1, 1,1,1,1
		

longlista:	
	.int (.-lista)/4
resultado:
	.quad 0x0123456789ABCDEF
formato:
	.ascii "suma = %lld = %llx hex\n\0"		# formato paraq printf() libC	


.section .text
#  start:	.global _start
  main:.global main

	mov    $lista, %ebx
	mov longlista, %ecx
	call suma
	mov %eax, resultado
	mov %edx, resultado+4
	
	push resultado+4
	push resultado
	push resultado+4
	push resultado
	push $formato
	call printf
	add $20, %esp

	mov $1, %eax
	mov $0, %ebx
	int $0x80

# SUBRUTINA: suma(int* lista, int longlista);
# entrada: 1) %ebx = dirección inicio array
#	   2) %ecx = número de elementos a sumar
# salida   3) %eax = resultado de la suma

suma:
#	push %edx
	mov $0, %edi
	mov $0, %ebp
	mov $0, %esi
bucle:
	mov (%ebx,%esi,4), %eax
	cltd
	add %eax,%edi
	adc %edx,%ebp
	inc %esi
	cmp %esi,%ecx
	jne bucle

	mov %edi,%eax
	mov %ebp,%edx
#	pop %esi
	ret
