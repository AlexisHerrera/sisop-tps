TP2: Procesos de usuario
========================

env_alloc
---------

1. Se le asignan los id 0x1000, 0x1001, 0x1002, 0x1003, 0x1004 a los 5 primeros procesos.
Esto sucede porque para generar el id se hace lo siguiente:
a) La base del id es 0x1000 (por el shift a la izquierda de ENVGENSHIFT bits)
b) A eso se le suma el env_id del proceso. En este caso 0 al iniciarse todo el array envs en 0.
c) Se limpia los ultimos 10 bits (lo hace el ~(NENV-1)).

Por ultimo se hace un or de la base del id (0x1000) con el offset del nuevo environment.
Lo cual permite variar los ids de manera incremental. 

2. Si se lanzan NENV(1024) environments/procesos y luego se destruye el proceso 630
el cual tiene env_id 0x1000+630 = 0x1276, entonces al crear y matar un proceso de manera
consecutiva los env_id serán:
a) Para el primer proceso, primero sucederá que el env_id no es igual a 0 sino
que es el que tenía antes (0x1276), y a este se le suma 0x1000 y se limpian los ultimos 10 bits.
Entonces queda por el momento como 0x2000
b) Luego se le aplica un or con la posición en el arreglo, en este caso 630. Lo cual lo deja con
0x2276.

Si aplicamos este proceso una y otra vez (sin matar o agregar un proceso en medio), simplemente
estaremos sumando 0x1000 al env_id. Por lo que las primeras 5 iteraciones serán:
0x2276, 0x3276, 0x4276, 0x5276, 0x6276.


env_init_percpu
---------------


La función lgdt es un wrapper de la instrucción lgdt (load global descriptor table).
La instrucción carga el registro GDTR (de 48 bits) con 16 bits de limite y 24 bits de base.
Entonces al ejecutar lgdt(&gdt_pd) cargamos el pseudo descriptor al registro gdtr.
Podemos ver que el pseudo descriptor esta formado por el límite de 16 bits (sizeof(gdt)-1) y 
la direccion de la tabla gdt (24bits).
Sin embargo esta gdt ya esta cargada, y la instrucción lgdt solo carga estos 48 bits (6 bytes)
en el registro GDTR.

env_pop_tf
----------
1) 
Al ejecutar la instrucción "movl %0,%%esp" el stack pointer ahora apunta al trapframe. 
Entonces en el tope de la pila tiene un trapframe, el cual los primeros 32 bytes son 
los 8 registros de propósito general. Luego popal se encarga de restaurar estos registros.

Después de restaurar los registros ES, DS y también de saltearse tf_trapno y tf_errcode, el stack pointer
apunta al primer registro del conjunto [EIP, CS, E_FLAGS, ESP, SS], es decir EIP.

Como se puede ver, el tercer elemento de la pila es el registro e_flags, el cual contiene el estado actual
del procesador.

2)

El nivel de privilegio actual o CPL se puede saber según los últimos 2 bits del registro CS.
Entonces el CPU va a poder comparar el CS del trapframe con el CS actual, para ver si también popea
al ESP y al SS (que sucede cuando se retorna a un nivel de privilegio inferior).


gdb_hello
---------

...
