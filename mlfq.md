## Scheduling avanzado

Simulación de MLFQ
--------------
1. Se puede simular round-robin con el script de la siguiente manera:
* Utilizando una sola cola: --numQueues=1. Esto porque round-robin no utiliza colas de prioridad
* Definiendo un quantum de tiempo.  Por ejemplo, el mas sencillo: --quantum=1.
Con esto es suficiente para definir la política de planificación round-robin.
Se pueden agregar mas configuraciones, como tiempos de inicio distintos entre jobs, distinta duración de los jobs, cada cuanto tiempo tiene que realizar una operación de I/O.

Vamos a realizar un ejemplo, donde RR es malo. Por ejemplo tareas de similar magnitud.
Esto es malo porque el turn around time de las tareas se dispara. Configuremos el script de la siguiente manera:
python2 ./mflq.py -c --numQueues=1 --quantum=1 --iotime=0 --numJobs=3 --jlist=0,5,0:0,5,0:0,5,0
Es decir, un ejemplo donde hay 3 tareas de 5 unidades de duración.

Job List:
  Job  0: startTime   0 - runTime   5 - ioFreq   0
  Job  1: startTime   0 - runTime   5 - ioFreq   0
  Job  2: startTime   0 - runTime   5 - ioFreq   0

```
Execution Trace:

[ time 0 ] JOB BEGINS by JOB 0
[ time 0 ] JOB BEGINS by JOB 1
[ time 0 ] JOB BEGINS by JOB 2
[ time 0 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 4 (of 5) ]
[ time 1 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 4 (of 5) ]
[ time 2 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 4 (of 5) ]
[ time 3 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 3 (of 5) ]
[ time 4 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 3 (of 5) ]
[ time 5 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 3 (of 5) ]
[ time 6 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 2 (of 5) ]
[ time 7 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 2 (of 5) ]
[ time 8 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 2 (of 5) ]
[ time 9 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 1 (of 5) ]
[ time 10 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 1 (of 5) ]
[ time 11 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 1 (of 5) ]
[ time 12 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 5) ]
[ time 13 ] FINISHED JOB 0
[ time 13 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 5) ]
[ time 14 ] FINISHED JOB 1
[ time 14 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 5) ]
[ time 15 ] FINISHED JOB 2

Final statistics:
  Job  0: startTime   0 - response   0 - turnaround  13
  Job  1: startTime   0 - response   1 - turnaround  14
  Job  2: startTime   0 - response   2 - turnaround  15

  Avg  2: startTime n/a - response 1.00 - turnaround 14.00
```
Una cola o SJF completarían el trabajo en tan solo 15 unidades de tiempo.
Sin embargo en este caso tenemos un mejor tiempo de respuesta (1 ut) a cambio de un mayor turn around time (en promedio 14 ut).