# Rationale
La duración no es constante, varía mucho de una ejecución a otra por lo que no
se puede estimar.
La varianza en la duración de hello_ordered_wait se debe a varios factores:
1. Cantidad de threads que se crean y velocidad asociada.
2. Limitación en cantidad de CPUs (colas de espera).
3. Espera activa como mecanismo de sincronización.
Mientras que el main_thread crea muchos threads a gran velocidad, los otros CPUs
disponibles tienen que repartirse esos hilos que están en cola de espera, al
haber interrupciones para poder satisfacer la demanda de tantos procesos puede
que el thread que estaba en camino a realizar su tarea sea dirigido al final de una
cola. Dada esa situación, todos los demás hilos que estaban en cola van a 
malgastar su tiempo de ejecución esperando que siga su turno, el cual llegará
hasta que aquel hilo que fue encolado vuelva a tener CPU y así suceda con los que le siguen.
