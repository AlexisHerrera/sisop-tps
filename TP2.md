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

...


env_pop_tf
----------

...


gdb_hello
---------

...
