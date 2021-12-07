## Scheduling avanzado

Simulación de MLFQ
--------------
1. Se puede simular round-robin con el script de la siguiente manera:
* Utilizando una sola cola: --numQueues=1. Esto porque round-robin no utiliza colas de prioridad
* Definiendo un quantum de tiempo.  Por ejemplo, el mas sencillo: --quantum=1.

Con esto es suficiente para definir la política de planificación round-robin.
Se pueden agregar mas configuraciones, como tiempos de inicio distintos entre jobs, distinta duración de los jobs, cada cuanto tiempo tiene que realizar una operación de I/O.

Round robin mejora el response time (de hecho es la que mejor response time brinda), pero pagando el precio del turn-around time.
Con los siguientes parametros se va a poder ver un ejemplo de 3 tareas con 10 segundos de duración cada una y un quantum de 5 ut.

```bash
python2 ./mflq.py -c --numQueues=1 --quantum=5 --iotime=0 --numJobs=3 --jlist=0,10,0:0,10,0:0,10,0
```
![RR-Diagram](screenshots/round-robin-example.png)

```bash
Job List:
  Job  0: startTime   0 - runTime  10 - ioFreq   0
  Job  1: startTime   0 - runTime  10 - ioFreq   0
  Job  2: startTime   0 - runTime  10 - ioFreq   0

Execution Trace:

[ time 0 ] JOB BEGINS by JOB 0
[ time 0 ] JOB BEGINS by JOB 1
[ time 0 ] JOB BEGINS by JOB 2
[ time 0 ] Run JOB 0 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 9 (of 10) ]
[ time 1 ] Run JOB 0 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 8 (of 10) ]
[ time 2 ] Run JOB 0 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 7 (of 10) ]
[ time 3 ] Run JOB 0 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 6 (of 10) ]
[ time 4 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 5 (of 10) ]
[ time 5 ] Run JOB 1 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 9 (of 10) ]
[ time 6 ] Run JOB 1 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 8 (of 10) ]
[ time 7 ] Run JOB 1 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 7 (of 10) ]
[ time 8 ] Run JOB 1 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 6 (of 10) ]
[ time 9 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 5 (of 10) ]
[ time 10 ] Run JOB 2 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 9 (of 10) ]
[ time 11 ] Run JOB 2 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 8 (of 10) ]
[ time 12 ] Run JOB 2 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 7 (of 10) ]
[ time 13 ] Run JOB 2 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 6 (of 10) ]
[ time 14 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 5 (of 10) ]
[ time 15 ] Run JOB 0 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 4 (of 10) ]
[ time 16 ] Run JOB 0 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 3 (of 10) ]
[ time 17 ] Run JOB 0 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 2 (of 10) ]
[ time 18 ] Run JOB 0 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 1 (of 10) ]
[ time 19 ] Run JOB 0 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 10) ]
[ time 20 ] FINISHED JOB 0
[ time 20 ] Run JOB 1 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 4 (of 10) ]
[ time 21 ] Run JOB 1 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 3 (of 10) ]
[ time 22 ] Run JOB 1 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 2 (of 10) ]
[ time 23 ] Run JOB 1 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 1 (of 10) ]
[ time 24 ] Run JOB 1 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 10) ]
[ time 25 ] FINISHED JOB 1
[ time 25 ] Run JOB 2 at PRIORITY 0 [ TICKS 4 ALLOT 1 TIME 4 (of 10) ]
[ time 26 ] Run JOB 2 at PRIORITY 0 [ TICKS 3 ALLOT 1 TIME 3 (of 10) ]
[ time 27 ] Run JOB 2 at PRIORITY 0 [ TICKS 2 ALLOT 1 TIME 2 (of 10) ]
[ time 28 ] Run JOB 2 at PRIORITY 0 [ TICKS 1 ALLOT 1 TIME 1 (of 10) ]
[ time 29 ] Run JOB 2 at PRIORITY 0 [ TICKS 0 ALLOT 1 TIME 0 (of 10) ]
[ time 30 ] FINISHED JOB 2

Final statistics:
  Job  0: startTime   0 - response   0 - turnaround  20
  Job  1: startTime   0 - response   5 - turnaround  25
  Job  2: startTime   0 - response  10 - turnaround  30

  Avg  2: startTime n/a - response 5.00 - turnaround 25.00
```
Podemos ver, como por ejemplo el job 0 va a tener un turn-around time mayor (el doble) respecto a una política de scheduling como FIFO o SJF.
