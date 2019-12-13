# Ejercicio 23 [prime_process]
## prime_hybrid_int
#### Best output
970704 primes found in range [2,15000000[ in 1.53734s with 2 processes and 16 threads  
## prime_process
#### Best output
970704 primes found in range [2,15000000[ in 1.09954s with 16 processes  
  
La menor duración la produjo la solución *prime_process*, el hecho que esta solución
únicamente emplee recursos en comunicación punto a punto y no en creación y destrucción
de threads hace que sea más eficiente.

#### Especificaciones del sistema
OS: Debian GNU/Linux 9 (stretch) 64-bit  
Virtualization: Oracle  
RAM: 3GiB  
Processor: Intel Core i5-6200 CPU @ 2.30GHz 2.40GHz  
Logical processors: 4  
Cores: 2
