# Controlador MIDI para botones y comandos

Los comandos en la Radioberry son procesados a través del protocolo MIDI. Una Raspberry PI Pico RP2040 es conectada a través de un puerto USB de la Raspberry PI 4. Este proyecto está basado en la versión original creada por [VU2DLE -- Radioberry_Console](https://github.com/VU2DLE/Radioberry_Console). Usa el mismo circuito esquemático (misma asignación de pines). El esquemático [también fue adjuntado a este repositorio](schematic.pdf).

## Instalación

1. Apretando el botón `BOOTSEL` de la Raspberry PI Pico RP2040, conectarla a nuestra computadora. Soltar el botón unos segundos después, esto montará una unidad extraible a la que le podemos copiar archivos.
2. Descargar CircuitPython (Versión usada en este proyecto: 9.2.8) desde [este enlace](https://circuitpython.org/board/raspberry_pi_pico/), es un archivo `.uf2`.
3. Copiar el archivo `adafruit-circuitpython-raspberry_pi_pico-en_US-9.2.8.uf2` a la unidad extraíble que se montó en el paso 1.
4. Desconectar la Raspberry PI Pico RP2040 y volver a conectarla pero sin presionar ningún botón.
5. Correr el script `./deploy-to-pico.sh`. El script está preparado para correr en Mac OS. Si estás usando Linux seguro tengas que ajustar el path donde se monta el volumen de la Raspberry PI Pico RP2040.
6. Conectar la Raspberry PI Pico RP2040 a la Raspberry PI de la Radioberry por USB.
7. En el programa PiHPSDR dirigirse a Menu > MIDI. En "Select MIDI Device" debería aparecer un ítem `Pico CircuitPython usb_midi.por`. Luego en esa misma ventana tocar el botón `Load` y cargar el archivo [usb-midi-definition.midi](usb-midi-definition.midi), que aloja todas las configuraciones necesarias para el mapeo de las teclas.

![MIDI configuration](midi-config-pihpsdr.png)
