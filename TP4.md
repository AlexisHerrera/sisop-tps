TP4: FileSystem
===========================

DISKMAP
--------------

Super representa al disco, tanto a la parte fisica como a la virtual. super->s_nblocks son la cantidad total de bloques en el disco

Super se configura en fsformat.c
Se aloca un BLKSIZE
Se setea el numero magico, la cantidad de bloques y la raiz del mismo.