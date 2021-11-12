TP2: Procesos de usuario
========================

env_alloc
---------

1. Se le asignan los id 0x1000, 0x2000, 0x3000, 0x4000, 0x5000
2. Serán distintos env_id distintos como los que se obtuvieron en 1.


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
