# Ejercicio 5 [transmission_time_5]
40 sensores que se turnan y cada minuto envían 1.5 MB
1.5 MB = 1500000 Bytes
16 Mbps = 2000000 Bytes por segundo
Latencia = 20 ms = 0.02 s
**T** = 0.02 + (1500000)/(2000000) = 0.77
0.77 segundos el tiempo tardado por sensor.
0.77 * 40 = 30.8 segundos entre todos los sensores.
Si se agregan 30 sensores más entonces el tiempo aumentaría a 0.77 * 70 = 53.9 segundos.
Debido a que no se sobrepasa el minuto entonces se puede mantener el mismo ancho de banda sin problemas.
