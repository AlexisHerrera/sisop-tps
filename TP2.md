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

b) Luego se le aplica un or con la posición en el arreglo, en este caso 630 (0x276). Lo cual lo deja con
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
Sin embargo esta gdt ya esta en memoria, y la instrucción lgdt solo carga estos 48 bits (6 bytes)
en el registro GDTR.

env_pop_tf
----------
1)  Al ejecutar la instrucción "movl %0,%%esp" el stack pointer ahora apunta al trapframe. 
Entonces en el tope de la pila tiene un trapframe, el cual los primeros 32 bytes son 
los 8 registros de propósito general. Luego popal se encarga de restaurar estos registros.

Después de restaurar los registros ES, DS y también de saltearse tf_trapno y tf_errcode, el stack pointer
apunta al primer registro del conjunto [EIP, CS, E_FLAGS, ESP, SS], es decir EIP.

Como se puede ver, el tercer elemento de la pila es el registro e_flags, el cual contiene el estado actual
del procesador.

2) El nivel de privilegio actual o CPL se puede saber según los últimos 2 bits del registro CS.
Entonces el CPU va a poder comparar el CS del trapframe con el CS actual, para ver si también popea
al ESP y al SS (que sucede cuando se retorna a un nivel de privilegio inferior).

gdb_hello
---------

1.
```bash
	  (gdb) break env_pop_tf
      Punto de interrupción 1 at 0xf0102f25: file kern/env.c, line 484.
      (gdb) c
      Continuando.
      Se asume que la arquitectura objetivo es i386
      => 0xf0102f25 <env_pop_tf>:	endbr32 
      Breakpoint 1, env_pop_tf (tf=0xf01c6000) at kern/env.c:484 
```
2. QEMU: 
```bash
QEMU 4.2.1 monitor - type 'help' for more information
(qemu)
```
GDB:
```bash
(qemu) info registers
EAX=003bc000 EBX=f01c6000 ECX=f03bc000 EDX=0000020d
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102f25 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
...
```
3. 
```bash
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c6000
```
4. 
```bash
(gdb) print sizeof(struct Trapframe)/sizeof(int)
$2 = 17
(gdb) x/17x tf
0xf01c6000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c6010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c6020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c6030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c6040:	0x00000023
```
5. 
```bash
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0102f25 <+0>:	endbr32 
   0xf0102f29 <+4>:	push   %ebp
   0xf0102f2a <+5>:	mov    %esp,%ebp
   0xf0102f2c <+7>:	sub    $0xc,%esp
   0xf0102f2f <+10>:	mov    0x8(%ebp),%esp
   0xf0102f32 <+13>:	popa   
   0xf0102f33 <+14>:	pop    %es
   0xf0102f34 <+15>:	pop    %ds
   0xf0102f35 <+16>:	add    $0x8,%esp
   0xf0102f38 <+19>:	iret   
   0xf0102f39 <+20>:	push   $0xf010548f
   0xf0102f3e <+25>:	push   $0x1ee
   0xf0102f43 <+30>:	push   $0xf010544a
   0xf0102f48 <+35>:	call   0xf01000af <_panic>
```
Para ir a la instrucción despues del ``` mov 0x8(%ebp),%esp ```tengo que hacer ```si 5```.
```bash
(gdb) si 5
=> 0xf0102f32 <env_pop_tf+13>:	popa   
0xf0102f32 in env_pop_tf (tf=0x0) at kern/env.c:485
485		asm volatile("\tmovl %0,%%esp\n"
```
6. 
```bash
(gdb) x/17x $sp
0xf01c6000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c6010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c6020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c6030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c6040:	0x00000023
```
Se puede ver que efectivamente ahora el stack pointer apunta al struct Trapframe tf.

7. Antes de analizar lo que hay en el tf, hay que remarcar que lo que se muesta como:
```
0xf01c6000:	0x00000000	0x00000000	0x00000000	0x00000000
```
significa que en la dirección 0xf01c6000 hay un int(32 bits) en 0.

Luego en la dirección 0xf01c6004, otro 0 de 32 bits, y así para el resto. Entonces por cada linea se muestran 4 ints.


Las primeras dos líneas (8 ints) perteneces a los registros de propósito común. Todas están seteadas a 0.

```bash
0xf01c6000:	0x00000000	0x00000000	0x00000000	0x00000000
            reg_edi     reg_esi     reg_ebp     reg_oesp
0xf01c6010:	0x00000000	0x00000000	0x00000000	0x00000000
            reg_ebx     reg_edx     reg_ecx     reg_eax
```

La tercera linea corresponde a lo siguiente:

```bash
0xf01c6020:	0x00000023	0x00000023	0x00000000	0x00000000
            padding1|es padding2|ds tf_trapno   tf_err		
```
Notar que al ser little endian, primero se ubica el es o ds y luego el padding.
El ES y DS estan seteados a 0x23 porque en el env_alloc se hizo:
```cpp
e->env_tf.tf_ds = GD_UD | 3;
e->env_tf.tf_es = GD_UD | 3;
```
donde GD_UD es 0x20 y el 3 es por el ring usuario.

La cuarta linea:

```bash
0xf01c6030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
            tf_eip      padding3|cs tf_eflags   tf_esp
```
Donde tf_eip es la dirección de la instrucción que va a correr el proceso, en este caso hello.

Nuevamente padding3|cs se seteo en el env_alloc.

Y por último tf_esp es el stack pointer que va a tener el proceso. En este caso se puede ver que es la misma dirección que marca USTACKTOP en el memlayout.h.

La quinta y última linea:
```bash
0xf01c6040:	0x00000023
            padding4|ss
```
Es simplemente otro registro que se marca en el env_alloc.


8. Para continuar hasta el iret se debe hacer un ```si 4```.
```bash
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c6030
EIP=f0102f38 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
Se puede ver que efectivamente se cambiaron los valores de registro al del environment, los cuales analizamos anteriormente. 

* Se setearon a 0 los registros de propósito general (los primeros 7).
* El ESP y el EIP siguen estando en valores similares, cambiaron un poco por la ejecución de comandos. Es decir, no cambiaron al del environment aún. Esto lo causo la instrucción popal 
* También cambiaron el ES y DS, Esto lo generaron las instruciones ```popl %es``` y ```popl %ds```.
* Puede verse que los de la tercera fila no cambiaron, como el CPL porque este está en el CS y este no se modificó.

9. Para ejecutar iret se puede hacer  ```si 1```
```bash
(gdb) si 1
=> 0x800020:	cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) p $pc
$1 = (void (*)()) 0x800020
(gdb) add-symbol-file obj/user/hello 0x800020
add symbol table from file "obj/user/hello" at
	.text_addr = 0x800020
(y or n) y
Reading symbols from obj/user/hello...
(gdb) p $pc
$2 = (void (*)()) 0x800020 <_start>
```
```bash
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```
Se produjeron los siguientes cambios respecto al anterior:
* Cambio el ESP, esto al pasar del ring 0 al 3 se popea el stack
* Cambio el EIP, paso de direcciones altas a una baja.
* Cambio el CS, y por lo tanto tambien el CPL.
* Cambio el EFL

Todos estos cambios lo produjo el iret, terminó de actualizar los registros a los valores del environment.

10. 
```bash
(gdb) tbreak syscall
Punto de interrupción temporal 2 at 0x800a3e: syscall. (2 locations)
(gdb) c
Continuando.
=> 0x800a3e <syscall+17>:	mov    0x8(%ebp),%ecx

Temporary breakpoint 2, syscall (num=0, check=-289415544, a1=4005551752, a2=13, a3=0, a4=0, a5=0) at lib/syscall.c:23
23		asm volatile("int %1\n"
(gdb) disas
Dump of assembler code for function syscall:
 ...
=> 0x00800a3e <+17>:	mov    0x8(%ebp),%ecx
   0x00800a41 <+20>:	mov    0xc(%ebp),%ebx
   0x00800a44 <+23>:	mov    0x10(%ebp),%edi
   0x00800a47 <+26>:	mov    0x14(%ebp),%esi
   0x00800a4a <+29>:	int    $0x30
   0x00800a4c <+31>:	cmpl   $0x0,-0x1c(%ebp)
(gdb) si 5
Se asume que la arquitectura objetivo es i8086
[f000:e05b]    0xfe05b:	cmpw   $0xffc8,%cs:(%esi)
0x0000e05b in ?? ()
```
Lo que sucede es que se ejecuta una syscall, la cual genera una interrupción (```int $0x30```) y pasa a modo kernel.
Esto se puede ver reflejado en el CPL (antes 3, ahora 0).
```bash
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000663
ESI=00000000 EDI=00000000 EBP=00000000 ESP=00000000
EIP=0000e05b EFL=00000002 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0 
ES =0000 00000000 0000ffff 00009300
CS =f000 000f0000 0000ffff 00009b00
```
kern_idt
---------
1)
```TRAPHANDLER``` y ```TRAPHANDLER_NOEC``` definen una función globalmente visible para handlear una interrupción. Luego de definirlas, las cargamos en la tabla IDT.

Para decidir cual usar se debe tener en cuenta que hay interrupciones que al handlearlas pushea un error code y otras que no.
Para saber cual elegir, se debe tener como referencia el manual de Intel (en este caso).

Si solo se usara la primera, se asumiria que siempre se pushea el error code, por lo que al cargar nuestro ```struct trapframe``` lo haríamos con 
registros/atributos incorrectos. Entonces al popearlos tendríamos de manera incorrecta, por ejemplo, el eip y esto llevaría a un page fault.

2)
La diferencia está en que si istrap es 1, se permite anidación de interrupciones. Es decir, se puede handlear una interrupción mientras que se estaba handleando otra, por ejemplo para priorizar una interrupcion de I/O.

Este feature es deseable para Kernels "reales", ya que sino interrupciones más importantes tendrían que esperar que finalice otras menos importantes como por ejemplo un overflow.

3)
El programa trata de generar un Page Fault o int 14. Sin embargo la interrupción que se genera es la 13, también conocida como General Protection.
Esto sucede porque se intentó ejecutar una interrupción desde el ring 3, cuando los permisos necesarios para ejecutarla son del ring 0.

Entonces, en vez de ejecutarse esa interrupción, se ve que como no tenía ese privilegio entonces se ejecuto la 13.
Para comprobar si este es el caso, podríamos cambiar el permiso de la interrupt a 3, y vemos que:

```diff
- SETGATE(idt[T_PGFLT], 0, GD_KT, trap_14_pf, 0);
+ SETGATE(idt[T_PGFLT], 0, GD_KT, trap_14_pf, 3);
```
```bash
Incoming TRAP frame at 0xefffffc0
[00001000] user fault va 00000000 ip 0000001b
TRAP frame at 0xefffffc0
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdff0
  oesp 0xefffffe0
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000e Page Fault
  cr2  0x00000000
  err  0x00800039 [kernel, read, protection]
  eip  0x0000001b
  cs   0x----0082
  flag 0xeebfdfd4
  esp  0x00000023
  ss   0x----ff53
[00001000] free env 00001000
```
Es decir, se genera la interrupción page fault. 

evil_hello
---------

La diferencia entre ambas funciones es que la original no provoca un page fault, en cambio la que se muestra sí.
Esto es porque no existe un mecanismo hasta el momento para verificar si la dirección que se le indica a ```sys_cputs``` la puede acceder el usuario. Ya que dicha syscall se ejecuta en el kernel, y aqui está mapeada la dirección  ```0xf010000c``` y también tiene permisos.

En cambio, al hacer ```char first = *entry ``` 
se accede a la dirección ```0xf010000c``` desde un proceso de usuario (ring 3), pero para acceder hay que tener permisos del kernel.

Se puede poner un break en ```trap_dispatch``` y comprobar que se handlea un page fault, al ejecutar dicha linea.

Al ejecutar el ```page_fault_handler```, este mata al proceso y por lo tanto nunca se ejecuta ```sys_cputs```.

Esto sí es un problema porque para el kernel, las direcciones que pide el usuario acceder sí las conoce y tiene permiso, por lo tanto no genera un page fault.
