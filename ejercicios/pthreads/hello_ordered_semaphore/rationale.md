# hello_ordered_semaphore
Este mecanismo de control de concurrencia sí permite
resolver este problema de manera predecible.  
El uso de semáforos, permite controlar en qué momento cada
hilo puede ingresar a la región crítica, en este caso
es posible gracias a que el arreglo de semáforos se
encuentra en memoria compartida.  
El uso de mutex para resolver este problema resulta "poco
natural", normalmente se usaría un mutex para controlar la
concurrencia de todos los hilos. La acción de unlock() la
debería realizar el mismo hilo que hizo lock() lo cual no
se puede para resolver este problema.  
Esa es la principal diferencia con resolverlo a través de
semáforos, donde el envío de señal (sem_post()) para el siguiente
hilo resulta "más natural", cada uno señala cuando el siguiente
trabajador puede ingresar a la región crítica.
