# Ejercicio 24 [prime_process_reduction]
La máxima cantidad de procesos que el clúster permite es 1016. 
## prime_hybrid_int
#### Best output
970704 primes found in range [2,15000000[ in 0.638148s with 127 processes and 1016 threads  
## prime_process
#### Best output
970704 primes found in range [2,15000000[ in 1.98963s with 1016 processes
## prime_process_reduction
#### Best output
970704 primes found in range [2,15000000[ in 1.07799s with 1016 processes
  
La menor duración la produjo la solución *prime_hybrid_int*, esto debido a que la cantidad
de procesos que tienen que comunicarse es menor y la velocidad de comunicación entre
los hilos de cada proceso es mucho mayor.

#### Especificaciones del sistema
OS: Debian GNU/Linux 9 (stretch) 64-bit  
Virtualization: Oracle  
RAM: 3GiB  
Processor: Intel Core i5-6200 CPU @ 2.30GHz 2.40GHz  
Logical processors: 4  
Cores: 2
