TP3: Multitarea con desalojo
===========================

multicore_init
--------------
1. 
En el archivo ```kern/mpentry.S``` se encuentran las etiquetas ```mpentry_start``` y ```mpentry_end```, entre las cuales se encuentran las instrucciones
que permiten iniciar cada AP (application processor).

Como es una instrucción que se ejecuta desde el BSP, entonces es necesario brindarle la dirección de memoria virtual (ya mapeado),
en donde se va a copiar este código. De esta manera, cada AP podrá utilizar como entry point, la dirección MPENTRY_PADDR (0x7000) de memoria física.

2.
Se utiliza la variable ```mpentry_kstack``` para indicar el stack pointer que utilizará cada AP.
Estas direcciones se calculan en la función boot_aps, una a una, hasta que todas las APs hayan iniciado.

Como se quiere tener un stack para cada procesador, se decide reservar espacio para cada stack de manera contigua.
Entonces, si se hiciese en ```kern/mpentry.S```, se debería tener acceso al ```kern_pgdir``` pero esto no es posible aún.
Además, tampoco es el objetivo que inicie su propio ```kern_pgdir``` porque ya existe.

3. 
Si ejecutamos ```objdump -d obj/kern/mpentry.o``` podemos ver que en la dirección relativa 0x32 se ejecuta dicha instrucción.
Entonces si este código está copiado en la dirección MPENTRY_PADDR (0x7000), el EIP deberá estar en la dirección 0x7032.
Recordando que se trabaja ejecuta en la dirección física, ya que cada AP está en modo real.

env_return
--------------

La funcion ```libmain()``` de libmain.c, se encarga de tomar el proceso actual, y ejecutar el ```umain()```.
Una vez finalizada la ejecución (y sus respectivas llamadas a las syscalls), se finaliza el proceso con un ```exit()```.

Antes ```env_destroy()``` simplemente llamaba a ```env_free()``` e iniciaba el monitor. Ahora puede que el proceso que se intente destruir aún siga ejecutándose en otro CPU (y por lo tanto el estado aún en ENV_RUNNING). Entonces se lo marca como zombie (porque aún no se lo puede matar), pero la próxima vez que lo trapee el kernel, si liberará en proceso.

Si el mismo proceso es el que se debe liberar, entonces se marca que el CPU actual no está corriendo ningún proceso y se fuerza un cambio de contexto.

sys_yield
--------------

Si en vez de lanzar ```user_hello``` lanzamos el programa ```user_yield``` podemos ver lo siguiente:
```bash
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000, CPU 0
Hello, I am environment 00001001, CPU 0
Hello, I am environment 00001002, CPU 0
Back in environment 00001000, iteration 0, CPU 0
Back in environment 00001001, iteration 0, CPU 0
Back in environment 00001002, iteration 0, CPU 0
Back in environment 00001000, iteration 1, CPU 0
Back in environment 00001001, iteration 1, CPU 0
Back in environment 00001002, iteration 1, CPU 0
Back in environment 00001000, iteration 2, CPU 0
Back in environment 00001001, iteration 2, CPU 0
Back in environment 00001002, iteration 2, CPU 0
Back in environment 00001000, iteration 3, CPU 0
Back in environment 00001001, iteration 3, CPU 0
Back in environment 00001002, iteration 3, CPU 0
Back in environment 00001000, iteration 4, CPU 0
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4, CPU 0
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4, CPU 0
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
```

El programa ```user_yield``` ejecuta un loop (de 5 iteraciones) en el cual simplemente fuerza un context-switch para un proceso.
Al forzar un context switch, se ejecuta el proceso siguiente (como lo programamos en sched_yield) por el round-robin.

Luego cuando el round-robin vuelva al proceso, imprimirá el mensaje ```Back in environment ...```.
Es por esto que podemos ver como se van alternando los env_id, y recién hasta que vuelve al proceso que forzó el context switch, se imprime el mensaje.

Finalmente, podemos ver que una vez que el proceso haya vuelto e imprimido su último mensaje (por ejemplo ```Back in environment 00001000, iteration 4, CPU 0```), simplemente indica que se realizó todo en ese proceso y se libera.

envid2env
--------------

La función ```envid2env``` al pasarle un id con valor 0, te devuelve el environment actual.
Por lo que al pasarle a ```sys_env_destroy``` un id con valor 0, este obtiene de  ```envid2env``` el ```struct env *``` al mismo environment que está corriendo. 

Para luego imprimir un mensaje de "exiting gracefully", y finalizar terminando el proceso con ```env_destroy```. 

dumbfork
--------------

1. No, el ```duppage``` que se utiliza en ```dumbfork``` siempre crea y mapea con los permisos PTE_P|PTE_U|PTE_W, por lo tanto
puede que el padre no tenga permisos de escritura, pero el hijo si lo va a tener.
2. 
```c
envid_t dumbfork(void) {
    // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly = true;
        pde_t pde = uvpd[PDX(addr)];
		if (pde & PTE_P) {
			pte_t pte = uvpt[PTX(addr)];
			if (pte & PTE_P) {
                readonly = (pte & PTE_W) == 0;
            }
        }
        duppage(envid, addr, readonly);
    }
    // ...
```
3.
```sys_page_map``` verifica que si la página de origen tiene permisos de lectura pero la página de destino quiere guardarse con permisos de escritura, entonces devuelve -E_INVAL (error).
Entonces en el caso de que readonly sea true, la primera llamada a sys_page_map va devolver un error, no va a hacer nada.

Entonces simplemente podemos de antemano saber si es o no readonly, por lo que podemos llamar a lo sumo 3 veces a la syscall
en una ejecución, de la siguiente manera:

```c
void duppage(envid_t dstenv, void *addr, bool readonly) {
    sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
    if (!readonly) {
        // caso donde hay write
        sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);
    } else {
        // caso donde es readonly
        sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
    }
    memmove(UTEMP, addr, PGSIZE);
    sys_page_unmap(0, UTEMP);
}
```
