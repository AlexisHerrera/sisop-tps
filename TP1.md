TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

$ nm kernel
f0117530 b addr_6845
f0100b2b t boot_alloc

la direccion f0100b2b es 4027583275 en decimal

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

page2pa calcula la direccion fisica de la pagina asosiada a la estructura PageInfo provista. Esta funcion le resta la direccion de la primer pagina(guardada en pages) y hace un shift de 12 posiciones para descartar los flags y quedarse con la direccion
page2kva calcula la direccion virtual de la misma pagina, primero calcula la direccion fisica con page2pa, y luego hace un KADDR, que le suma la direccion base del kernel.


map_region_large
----------------

Al usar las paginas grandes, no hace falta reservar manualmente una tabla de entrada del pageDirectory, por lo tanto podemos sacar la entry_pgtable y hacer que se maneje automaticamente mediante una pagina grande. Al no tener toda esta memoria alocada mediante el codigo, nos estamos ahorrando de una page table entera, que ocupan 1024 espacios de 32 bits, es decir 32KiB.

Esto es una cantidad fija de memoria, ya que se realiza antes del i386_init() y por lo tanto no se conocen las dimensiones de la maquina. Esta memoria es necesaria para el set-up del JOS y no es negociable.