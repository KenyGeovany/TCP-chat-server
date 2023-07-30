Ejecución del programa:

En los 3 casos la compilación y ejecución se hace como sigue:

1. Primero en una terminal se ejecutará el servidor:

gcc server.c -o server.exe; ./server.exe

2. Luego en otras terminales se ejecutaran los clientes:

gcc client.c -o client.exe; ./client.exe pedro
gcc client.c -o client.exe; ./client.exe juan
gcc client.c -o client.exe; ./client.exe hasimoto
Etc.


Para terminar la conversación con otro cliente escribir "Bye".
Para desconectarse del servidor escribir "Exit".