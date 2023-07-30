# TCP-chat-server
This is a program that emulates a chat server using TCP protocol.

Explicar e algunas ideas utilizadas para hacer el programa pedido.
El n umero m aximo de clientes que se pueden conectar son 6 clientes, puesto que se admiten m aximo 10 sockets,
de los cuales, los primero cuatro ya son utilizados (el socket del servidor es el 3). Cada cliente est a representado
por un  le descriptor, y toda la informaci on acerca del estado de este  le descriptor se encuentra condensado en 3
arreglos: clients linked, clients status y clients name.
1. clients name: este arreglo relaciona un  le descriptor con su nombre en string.
2. clients status: relaciona un  le descriptor con su estado. Cada elemento i de clients status tiene 3 posibles
estados:
-1: el entero correpondiente al elemento no tiene asignado ning un  le descriptor.
0: hay un  le descriptor asignado a i, y est a en estado de espera.
1: hay un  le descriptor asignado a i, y est a en una conversaci on con otro cliente.
3. clients linked: relaciona el  le descriptor de un cliente con otro  le descriptor del cliente con el cual est a
conversando.
-1: El cliente no est a conversando con otro cliente.
fd: El cliente est a conversando con otro cliente con  le descriptor fd.
Con estos 3 arreglos es posible controlar las acciones de cada cliente.
Otra idea importante es la manera en la que un cliente puede terminar la conversaci on sin desconectarse del servidor.
Para lograr esto solo de nimos 2 maneras de terminar el programa: con un Exit o con un Bye. Y analizamos varios
casos:
1. El cliente estaba en espera y se desconecta del servidor: con Exit, este caso se considera como una salida
normal del cliente. Se cierra el socket del cliente y se borra de los arreglos de control.
2. El cliente conversaba con otro cliente, y termina la conversaci on sin despedirse con Bye: con Exit, este caso se
considera como una anomal  a, o interrupci on inesperada del cliente. Se cierra el socket del cliente y se borra
de los arreglos de control.
3. El cliente conversaba con otro cliente y decide terminar la conversaci on: con Bye, este caso se considera como
una despedida. No se cierra el socket, si no que se cambia el estatus del cliente a no conversando, y se le
solicita que elija de nuevo un cliente para conversar.
Otra nota importante es que en muchos casos se program o el errno para noti car de errores producidos en tiempo
de ejecuci on, pero que no matan el programa. Esto es porque no queremos que el servidor caiga. Tambi en evitamos
escribir algunos errnos en el cliente para evitar que se muestren en pantalla inde nidamente mensajes de errno.
Un cliente puede platicar consigo mismo sin que se rompa la l ogica del programa, y de hecho esto evita problemas
como men us vac  os, y muchos errnos.
