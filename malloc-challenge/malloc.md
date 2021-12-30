# Malloc challenge

Parte 1:

mmap crea un nuevo mapeo en la Virtual Address space del proceso que la llama.
En este caso, si un usuario utiliza malloc, recurre a la implementación de la libreria que utiliza la syscall mmap. Por lo tanto, se mapeará en la VAS del Usuario que utiliza malloc, pero esto es invisible para el usuario.

Para esta primera parte, al utilizar malloc por primera vez se se hará una llamada a mmap para mapear 16Kib, y de allí administrar los pedidos del usuario (incluyendo este). Si en algún momento, no hay suficiente espacio para satisfacer el pedido de memoria del usuario, entonces falla malloc devolviendo NULL y seteando el errno.
