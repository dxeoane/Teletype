# Teletype

Este es el firmware para crear una pantalla remota usando un ESP8266 y una pantalla ST7789.

Los comandos de dibujo se envían por UDP. Por ejemplo:

```
clear
cursor 0,3
println hola
println adios
line 10,100,100,120
```

Los comandos van uno por línea, pueden ir varios en el mismo paquete UDP o cada uno en un paquete distinto.

La lista de comandos es:

- `clear`
- `channel`
- `alert`
- `print`
- `println`
- `cursor`
- `size`
- `bgcolor`
- `fgcolor`
- `line`
- `hline`
- `vline`
- `rect`
- `fillrect`
- `erase`
- `circle`
- `fillcircle`
- `roundrect`
- `fillroundrect`
- `draw`
- `scale`
- `palette`
- `scanline`

Antes de compilar, hay que crear el archivo `secret.h`. En el repositorio viene un archivo de ejemplo `secret.h.example`. Dentro del archivo hay que configurar el SSID y la contraseña de la WiFi, y demás parámetros que son privados y no deben subirse al repositorio de GitHub.
