# Teletype

Este es el firmware para crear una pantalla remota usando un ESP8266 o ESP32 y una pantalla TFT.

Antes de compilar, hay que crear el archivo `secret.h`. En el repositorio viene un archivo de ejemplo `secret.h.example`. Dentro del archivo hay que configurar el SSID y la contraseña de la WiFi, y demás parámetros que son privados y no deben subirse al repositorio de GitHub.

Los comandos de dibujo se envían por UDP o MQTT. Por ejemplo:

```
clear
cursor 0,3
println hola
println adios
line 10,100,100,120
```

Los comandos van uno por línea, pueden ir varios en el mismo paquete o cada uno en un paquete distinto.

# Quick Reference

## 🎨 Color

- `bgcolor RRGGBB` → Define el color de fondo
- `fgcolor RRGGBB` → Define el color de dibujo (texto y líneas)

---

## 🖊️ Gráficos (coordenadas en píxeles)

- `clear` → Limpia la pantalla

- `line x1,y1,x2,y2` → Línea entre dos puntos
- `hline x,y,width` → Línea horizontal
- `vline x,y,height` → Línea vertical

- `rect x,y,width,height` → Rectángulo
- `fillrect x,y,width,height` → Rectángulo relleno
- `erase x,y,width,height` → Borra un área

- `roundrect x,y,width,height,radio` → Rectángulo con esquinas redondeadas
- `fillroundrect x,y,width,height,radio` → Rectángulo redondeado relleno

- `circle x,y,radio` → Círculo
- `fillcircle x,y,radio` → Círculo relleno

---

## 🔤 Texto

- `print TEXT` → Escribe texto en la posición actual
- `println TEXT` → Escribe texto y avanza una línea

- `size n` → Tamaño de fuente (1 = normal, 2 = grande)

---

## 📍 Posicionamiento

- `cursor x,y` → Posición en rejilla (depende de `size`)
- `moveto x,y` → Posición absoluta en píxeles
- `move dx,dy` → Desplazamiento relativo desde la posición actual

---

## ⚓ Layout (anclaje)

- `anchor` → Guarda la posición actual como referencia
- `relto dx,dy` → Posiciona relativo al anchor
- `feed` → Nueva línea alineada al anchor (como `println` pero alineado)