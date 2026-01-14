# Controlador MIDI FT-80C para piHPSDR

Controlador MIDI para Yaesu FT-80C usando Raspberry Pi Pico RP2040. Este controlador conecta los botones del panel frontal, potenciómetros y encoder de frecuencia del FT-80C con piHPSDR a través del protocolo MIDI.

## Configuración de Hardware

### Botones (15 en total)
Todos los botones se conectan a masa cuando se presionan, usando resistencias pull-up internas:

| Botón         | Pin GPIO | Nota MIDI |
| ------------- | -------- | --------- |
| narrow        | GP0      | 1         |
| att           | GP1      | 2         |
| noise blanker | GP2      | 3         |
| mode <        | GP3      | 4         |
| mode >        | GP4      | 5         |
| vfo > m       | GP5      | 6         |
| m > vfo       | GP6      | 7         |
| vfo           | GP7      | 8         |
| mr            | GP8      | 9         |
| split         | GP9      | 10        |
| pri-m         | GP10     | 11        |
| fast          | GP11     | 12        |
| band          | GP12     | 13        |
| clar          | GP13     | 14        |
| d lock        | GP14     | 15        |

### Potenciómetros (3 en total)
Conectados a los pines ADC (GP26/A0, GP27/A1, GP28/A2):

| Control        | Pin       | MIDI CC |
| -------------- | --------- | ------- |
| sql (squelch)  | A0 (GP26) | 21      |
| mic (ganancia) | A1 (GP27) | 22      |
| drive          | A2 (GP28) | 23      |

### Encoder Rotativo
Un único encoder rotativo sin botón para control de frecuencia:

| Función   | Pines      | MIDI CC |
| --------- | ---------- | ------- |
| frequency | GP15, GP16 | 11      |

### LED
El LED integrado (GP25) indica actividad de los botones.

## Instalación

1. Mantén presionado el botón `BOOTSEL` de la Raspberry Pi Pico RP2040 mientras la conectas a tu computadora. Suelta el botón después de unos segundos. Se montará una unidad extraíble.
2. Descarga CircuitPython (Versión 9.2.8 o posterior) desde [este enlace](https://circuitpython.org/board/raspberry_pi_pico/) como archivo `.uf2`.
3. Copia el archivo `.uf2` a la unidad extraíble montada.
4. Desconecta y vuelve a conectar la Pico sin presionar ningún botón.
5. Ejecuta el script de despliegue: `./deploy-to-pico.sh` (para macOS; usuarios de Linux pueden necesitar ajustar la ruta de montaje).
6. Conecta la Raspberry Pi Pico RP2040 a la Raspberry Pi que ejecuta piHPSDR vía USB.
7. En piHPSDR, ve a Menu > MIDI. Selecciona `Pico CircuitPython usb_midi.por` como dispositivo MIDI. Carga el archivo de configuración `usb-midi-definition.midi` usando el botón `Load`.

## Protocolo MIDI

- **Botones**: Envían MIDI Note On (valor 127) cuando se presionan y Note Off (valor 0) cuando se sueltan
- **Potenciómetros**: Envían MIDI Control Change con valores 0-127, con filtrado de umbral de ruido
- **Encoder**: Envía MIDI Control Change con valores relativos (63 = antihorario, 65 = horario)

## Notas

- Los potenciómetros usan un umbral de 512 unidades ADC para prevenir spam MIDI por ruido
- Los valores ADC (0-65535) se convierten al rango MIDI (0-127) mediante desplazamiento a la derecha de 9 bits
- El loop principal corre a 100Hz (delay de 10ms) para control responsivo
- El canal MIDI está configurado en 3 por defecto
