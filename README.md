# Filesafe
Descripción: El juego Filesafe se desarrolla en un mundo distópico en el que una inteligencia artificial controla a las personas mediante un chip insertado en sus cuellos. El jugador asume el rol de un agente encargado de liberar a las personas de este control mental y de formar un equipo que le permita escapar del lugar de forma segura. Para alcanzar este objetivo, el jugador debe seleccionar cuidadosamente los ítems necesarios para enfrentarse a cada personaje y hackear su chip. Si no logra hackearlo, perderá vida, y si su vida llega a cero, la partida finalizará.

## Cómo compilar y ejecutar la tarea:
**Consideraciones previas:**
- Tener instalado Visual Studio Code.
- Tener instalada la biblioteca Raylib.

**En caso de no tener la biblioteca Raylib aqui te mostramos los pasos de como instalarla:**
1.  Debes instalar MSYS2 MinGw W64. Puede hacerlo ingresando a la siguiente página: https://www.msys2.org/
Descarga e instala MSYS2 para Windows.

2.  Ahora debes ingresar a la terminal de MSYS2 MinGw W64 y escribir el siguiente comando:
```bash
pacman -Syu
```
- Para instalar el MinGW-w64 de 64 bits.
```bash
pacman -S mingw-w64-x86_64-toolchain
```
*Acepta todo.

- Para instalar el Raylib ejecuta el:
```bash
pacman -S mingw-w64-x86_64-raylib
```
**Recomendación:** desde ahora siempre abre MSYS2 MinGW 64-bit(puedes buscarla en el buscador del windows).

4. Para verificar que has instalado bien Raylib debes ejecutar los siguientes comandos en la terminal de MinGW64:
```bash
gcc --version
```
```bash
pkg-config --libs raylib
```


Ahora debes Ingresar al visual studio code. -Clonar en repositorio GitHub el link enviado al aula virtual:

**-----------Para compilar y ejecutar el codigo--------------**

Abre el archivo principal (por ejemplo, final2.c).

Abre la terminal con ctrl + ñ.

En la terminal bash, compila el programa con el siguiente comando :
## Ingresar al juego en la terminal powershell:
- Para compilar el código:
```bash
./compilador
```
- Para ejecutar el juego:
```bash
./juego
```
## Para agregar el avance de solo un archivo: 
- En la terminal bash ingresa los siguientes comandos:
```bash
git add README.md
```
```bash
git commit -m "avance"
```
```bash
git push origin main
```

