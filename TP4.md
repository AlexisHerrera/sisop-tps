TP4: FileSystem
===========================

DISKMAP
--------------

Super representa al disco, tanto a la parte fisica como a la virtual. super->s_nblocks es la cantidad total de bloques en el disco

Super se configura en fsformat.c, en la función opendisk().
En esta función, se crea la estructura del filesystem.
En uno de esos bloques que se reservó, está el Superbloque.
Aquí setea el numero magico, la cantidad de bloques y la raíz del mismo.
Estos valores son constantes para el filesystem de JOS.
