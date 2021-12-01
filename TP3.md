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




 