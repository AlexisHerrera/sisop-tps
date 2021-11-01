Simulando segmentación

1) Simulacion de traducciones

```

ARG seed 104128
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000030 (decimal 48)
  Segment 0 limit                  : 21

  Segment 1 base  (grows negative) : 0x000000d5 (decimal 213)
  Segment 1 limit                  : 29

Virtual Address Trace
  VA  0: 0x0000000c (decimal:   12) --> PA = decimal: 60
  VA  1: 0x00000007 (decimal:    7) --> PA = decimal: 55

```

```

ARG seed 104639
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000035 (decimal 53)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x000000dd (decimal 221)
  Segment 1 limit                  : 17

Virtual Address Trace
  VA  0: 0x00000030 (decimal:   48) --> PA = decimal: 205
  VA  1: 0x00000009 (decimal:    9) --> PA = decimal: 62

```

En ambos casos, las 2 direcciones virtuales se traducen a direcciones físicas válidas. Para hacer esto tradujimos las direcciones y chequeamos que se encuentren dentro de su segmento.
Antes de empezar con la traducción recordemos que necesitamos 6 bits para representar un espacio de 64 direcciones.
Luego si tiene el bit mas significativo con un 1 entonces la dirección virtual es mayor o igual a 32 en decimal.

En el caso del padrón 104128, la VA 12 se traduce como una dirección física en el segmento 0 al ser la VA menor a 32.
El offset es entonces 12 ya que en binario es 0|01100 y 0x01100 = 12 (dec). 
Este resultado es válido porque el límite que acepta el segmento 0 es 21.
Como el segmento  crece para los positivos, al sumarle el offset 12 a la base 48 es 60.


En el caso del padrón 104639, la VA es 48, su topbit es 1 y por lo tanto se traduce como una dirección física en el segmento 1.
El offset es entonces 16 ya que en binario es 1|10000 y 0x10000 = 16 (dec). 
Este resultado es válido porque el límite que acepta el segmento 1 es 17.
Como el segmento 1 crece para los negativos, al restarle el offset 16 a la base 221 es 205.


Se puede seguir el mismo camino para el resto de casos. Para todos los casos el resultado es válido.


3 ) Resultados con el flag -c:

```

ARG seed 104128
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000030 (decimal 48)
  Segment 0 limit                  : 21

  Segment 1 base  (grows negative) : 0x000000d5 (decimal 213)
  Segment 1 limit                  : 29

Virtual Address Trace
  VA  0: 0x0000000c (decimal:   12) --> VALID in SEG0: 0x0000003c (decimal:   60)
  VA  1: 0x00000007 (decimal:    7) --> VALID in SEG0: 0x00000037 (decimal:   55)

```

```

ARG seed 104639
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x00000035 (decimal 53)
  Segment 0 limit                  : 20

  Segment 1 base  (grows negative) : 0x000000dd (decimal 221)
  Segment 1 limit                  : 17

Virtual Address Trace
  VA  0: 0x00000030 (decimal:   48) --> VALID in SEG1: 0x000000cd (decimal:  205)
  VA  1: 0x00000009 (decimal:    9) --> VALID in SEG0: 0x0000003e (decimal:   62)

```
2) Traducciones inversas

En el primer caso, no es posible generar hacer una traducción inversa. 
Podemos ver que ambas direcciones virtuales (12 y 7) viven en el segmento 0, y las direcciones físicas que queremos acceder son 205 y 62.
Como el tamaño máximo de un segmento es de 32 posiciones, es imposible que 2 direcciones virtuales del mismo segmento accedan a 2 direcciones físicas con una separacion mayor a 32 direcciones. Como las direcciones físicas se encuentran a 143 posiciones, esta traduccion inversa no se puede lograr.

En cambio, en el otro caso si se puede hacer. Queremos que la dirección virtual 48 se traduzca a la dirección física 60 y la 9 en 55. La unica precaución que hay que tener es que no se nos superpongan los segmentos.
El segmento 1 debe comenzar en la dirección 76 y tener un tamaño de 16 y el segmento 0 debe comenzar en 46 y un límite de 10.
De esta forma al traducir la VA 48 con offset 16 entonces la PA es 76-16 = 60.
De manera similar para la VA 9 con offset 9, entonces la PA es 46+9 -> 55

```
./segmentation.py -a 64 -p 256 -s 104639 -A 48,9 -b 46 -l 10 -B 76 -L 16 -c
ARG seed 104639
ARG address space size 64
ARG phys mem size 256

Segment register information:

  Segment 0 base  (grows positive) : 0x0000002e (decimal 46)
  Segment 0 limit                  : 10

  Segment 1 base  (grows negative) : 0x0000004c (decimal 76)
  Segment 1 limit                  : 16

Virtual Address Trace
  VA  0: 0x00000030 (decimal:   48) --> VALID in SEG1: 0x0000003c (decimal:   60)
  VA  1: 0x00000009 (decimal:    9) --> VALID in SEG0: 0x00000037 (decimal:   55)


```

3) Límites de segmentación

1) El tamaño del espacio fisico es de 128 posiciones y el virtual de 32
2) En teoría es posible sin embargo el script no lo permite porque tendrías 2 direcciones virtuales distintas apuntando al mismo lugar de memoria física. 

```
./segmentation.py -a 32 -p 128 -A 10 -b 0 -l 10 -B 20 -L 11 -c
ARG seed 0
ARG address space size 32
ARG phys mem size 128

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 10

  Segment 1 base  (grows negative) : 0x00000014 (decimal 20)
  Segment 1 limit                  : 11

Error: segments overlap in physical memory

```

3) Si, es posible, teniendo 2 segmentos que tengan 16 direcciones se puede direccionar el 100% de la memoria virtual.

```

./segmentation.py -a 32 -p 128 -A 0,8,16,24,31 -b 0 -l 16 -B 128 -L 16 -c
ARG seed 0
ARG address space size 32
ARG phys mem size 128

Segment register information:

  Segment 0 base  (grows positive) : 0x00000000 (decimal 0)
  Segment 0 limit                  : 16

  Segment 1 base  (grows negative) : 0x00000080 (decimal 128)
  Segment 1 limit                  : 16

Virtual Address Trace
  VA  0: 0x00000000 (decimal:    0) --> VALID in SEG0: 0x00000000 (decimal:    0)
  VA  1: 0x00000008 (decimal:    8) --> VALID in SEG0: 0x00000008 (decimal:    8)
  VA  2: 0x00000010 (decimal:   16) --> VALID in SEG1: 0x00000070 (decimal:  112)
  VA  3: 0x00000018 (decimal:   24) --> VALID in SEG1: 0x00000078 (decimal:  120)
  VA  4: 0x0000001f (decimal:   31) --> VALID in SEG1: 0x0000007f (decimal:  127)


```

4) En cambio, no es posible direccionar el 90% de la memoria física, ya que solo podemos acceder por la misma a traves de direcciones virtuales y solamente tenemos 5 bits para direccionar la misma y 2 segmentos disponibles. Lo máximo que podemos direccionar es un 25% de la memoria física.
   