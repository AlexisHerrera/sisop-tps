# Malloc challenge

## Parte 1:

mmap crea un nuevo mapeo en la Virtual Address space del proceso que la llama.
En este caso, si un usuario utiliza malloc, recurre a la implementación de la libreria que utiliza la syscall mmap. Por lo tanto, se mapeará en la VAS del Usuario que utiliza malloc, pero esto es invisible para el usuario.

Para esta primera parte, al utilizar malloc por primera vez se se hará una llamada a mmap para mapear 16Kib, y de allí administrar los pedidos del usuario (incluyendo este). Si en algún momento, no hay suficiente espacio para satisfacer el pedido de memoria del usuario, entonces falla malloc devolviendo NULL y seteando el errno.

Pensando en el caso donde ocupo toda la memoria de golpe. Debería quedar así:  
* pos 0 a 7: header (size =  y magic = 1234567)
* pos 8 a 16367: payload
* pos 16368 a 16383: node_t

Por lo que lo máximo que se puede pedir es 16360 bytes.

Se brinda un set de test para comprobar funcionamiento interno de la implementación de la libreria:  ```./test ```.
Este set de pruebas tiene un análisis mas detallado del manejo de la memoria, los cuales no serían fáciles de probar con la libreria original (por ejemplo ```mm_cur_avail_space```)

### Programa de ejemplo

Se realiza un pequeño programa (main.c) donde:
1. Se hace un pedido de memoria de 100*4 bytes, se utiliza y luego se libera.
2. Se hace un pedido de 30*8 bytes para alojar un arreglo de arreglo de ints.
3. Se hace un pedido de 100*4 bytes para cada arreglo de ints.
4. Tras su uso se libera la memoria del arreglo de arreglos y de cada arreglo
5. Se pide una cantidad cercana a 16KiB (16000 bytes) (coalesce)

Se puede hacer las cuentas (100×4+30×8+100×4×30+16000) y ver que se alloco en total 28640 bytes.

Se puede utilizar el programa de la siguiente forma (nuestra libreria):
```bash
$ valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./main
```
O tambien probarlo con el malloc del sistema
```bash
$ LIB_MALLOC=1 valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./main
```
El output en los dos es el mismo:
```bash
==39825== Memcheck, a memory error detector
==39825== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==39825== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==39825== Command: ./main
==39825== 
==39825== 
==39825== HEAP SUMMARY:
==39825==     in use at exit: 0 bytes in 0 blocks
==39825==   total heap usage: 33 allocs, 33 frees, 28,640 bytes allocated
==39825== 
==39825== All heap blocks were freed -- no leaks are possible
==39825== 
==39825== For lists of detected and suppressed errors, rerun with: -s
==39825== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
## Parte 3: Análisis de la implementación
1. Si pido, por ejemplo, solo 1 byte de memoria muchas veces, siempre estaré utilizando MIN_BLOCK_SIZE, desaprovechando todo el espacio que no utilizo.
```c
    void* ptr1 = mm_malloc(1);
    void* ptr2 = mm_malloc(1);
    mm_cur_avail_space() // INITIAL_AVAIL_SPACE-2*(32+sizeof(header_t)) = 16360-80=16280
```
2. Si se quiere evitar el problema anterior, se debe dejar de utilizar como bloque minimo a MIN_BLOCK_SIZE.
 Existen otras mejoras, por ejemplo al buscar un siguiente bloque libre. Por ejemplo con next fit puedo esparcir las regiones allocadas al final de la region de memoria, evitando así los splinters al inicio de la memoria.
3. Binning mantiene agrupado regiones de memoria disponibles según su tamaño. Esto permite mayor rapidez en encontrar una región libre para un pedido de memoria. En cambio, con first fit debo recorrer, en el peor caso, cada bloque libre. Con binning simplemente sería ir al bloque correspondiente al pedido de memoria y verificar si en esa lista hay un bloque libre y alojarlo allí, casi O(1).