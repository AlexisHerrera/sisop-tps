TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

a)
Primero obtenemos donde termina el kernel.
$ nm obj/kern/kernel | grep end
f0117950 B end

Entonces de ahí simplemente alineamos a PGSIZE (que es 4096).
Es decir, la dirección es: 0xf0117950 + 4096 - 0xf0117950 % 4096 = 0xf0118000.

```

0x0000fff0 in ?? ()
(gdb) b pmap.c:88
Breakpoint 1 at 0xf0100b2b: file kern/pmap.c, line 89.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0100b2b <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=65684) at kern/pmap.c:89
89	{
(gdb) finish
Run till exit from #0  boot_alloc (n=65684) at kern/pmap.c:89
=> 0xf01025f0 <mem_init+26>:	mov    %eax,0xf0117948
mem_init () at kern/pmap.c:143
143		kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
Value returned is $1 = (void *) 0xf0118000     <-----

```
0xf0118000 = 4027678720

page_alloc
----------

page2pa calcula la direccion fisica de la pagina asosiada a la estructura PageInfo provista.  
Esta funcion le resta la direccion de la primer pagina(guardada en pages) y hace un shift de 12 posiciones, lo cual es equivalente a multiplicar 12 veces por 2, es decir por 4096.
Por lo tanto, obtengo el offset y luego multiplico por la cantidad de páginas, esto es la dirección física


page2kva calcula la direccion virtual de la misma pagina, primero calcula la direccion fisica con page2pa, y luego hace un KADDR, que le suma la direccion base del kernel.


map_region_large
----------------

Al usar las paginas grandes, no hace falta reservar manualmente una tabla de entrada del pageDirectory, por lo tanto podemos sacar la entry_pgtable y hacer que se maneje automaticamente mediante una pagina grande.
Para las short pages ocupamos una posicion en la page dir que hace referencia a un page table de 1024 entradas de 4 bytes.
En cambio para las large pages solo necesitaremos la tabla de page dir, el cual ocupa 4 bytes x 1024 entradas a esa tabla.
Entonces me ahorro por cada entrada de page dir, esa page table de 1024 x 4 bytes = 4096 bytes = 4KibiBytes.
Lógicamente cuanto más direcciones tengamos a las que direccionar, mas ahorro tendremos.
