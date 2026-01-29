# LCD Controller para Yaesu FT-747GX / FT-80C

Controlador para reutilizar el display LCD original del Yaesu FT-747GX (FT-80C) con un Arduino y el chip HT1621B.

Muestra frecuencia y modo recibidos via comandos CAT desde pihpsdr u otro software compatible.

## Arquitectura

```
pihpsdr (CAT :19090) --> lcd_bridge.py --> /dev/lcd_controller --> Arduino --> HT1621B --> Display FTD8627PZ
```

## Hardware

- Arduino Nano/Uno
- HT1621B (controlador LCD)
- Display original FTD8627PZ del Yaesu FT-747GX
- Diodo 1N4148 (para VLCD)

### Conexiones Arduino -> HT1621B

| Arduino      | HT1621B             |
| ------------ | ------------------- |
| D4           | CS (pin 9)          |
| D5           | WR (pin 11)         |
| D6           | DATA (pin 12)       |
| 5V           | VDD (pin 17)        |
| 5V via diodo | VLCD (pin 16) ~4.3V |
| GND          | VSS (pin 13)        |

### Conexiones HT1621B -> Display

| HT1621B    | Display    |
| ---------- | ---------- |
| COM0       | COM0       |
| COM1       | COM1       |
| COM2       | COM2       |
| SEG0-SEG31 | SEG0-SEG31 |

**IMPORTANTE**: El M50932 original debe estar desconectado/aislado del display.

### S-Meter

El S-meter analógico original se controla via PWM desde el Arduino:

| Arduino | Componente        |
| ------- | ----------------- |
| D3      | PWM -> Filtro RC  |

El bridge Python lee el nivel de señal via CAT y lo envía al Arduino con el comando `SM`. El Arduino genera una señal PWM en D3 que se suaviza con un filtro RC:

- **R**: 23k ohm
- **C**: 10uF electrolitico

```
D3 ---[23k]---+--- S-Meter
              |
             [10uF]
              |
             GND
```

El filtro RC convierte el PWM en una tension DC proporcional al nivel de señal.

## Archivos

| Archivo                     | Descripcion                       |
| --------------------------- | --------------------------------- |
| `lcd-controller-ft-80c.ino` | Codigo principal del controlador  |
| `segment_map.h`             | Mapeo de segmentos del display    |
| `segment_map.csv`           | Mapeo en formato CSV (referencia) |
| `ht1621b_test/`             | Herramienta de testing y mapeo    |
| `lcd_bridge.py`             | Bridge Python para pihpsdr        |
| `lcd.md`                    | Documentacion tecnica detallada   |

## Instalacion

### 1. Flashear el Arduino

Abrir `lcd-controller-ft-80c.ino` en Arduino IDE y subir al board.

### 2. Configurar regla udev (puerto serial fijo)

```bash
sudo cp 99-arduino-lcd.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Esto crea el symlink `/dev/lcd_controller` para el Arduino.

### 3. Instalar dependencias Python

```bash
pip3 install pyserial
```

### 4. Ejecutar el bridge

```bash
python3 lcd_bridge.py /dev/lcd_controller
```

Asegurarse que pihpsdr este corriendo con rigctl habilitado (puerto 19090).

## Autostart con systemd

```bash
cp lcd_bridge.py /home/pi/
sudo cp lcd-bridge.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable lcd-bridge
sudo systemctl start lcd-bridge
```

Ver estado:
```bash
sudo systemctl status lcd-bridge
journalctl -u lcd-bridge -f
```

## Protocolo CAT

El bridge usa protocolo Kenwood CAT sobre TCP:

| Comando          | Funcion                                 |
| ---------------- | --------------------------------------- |
| `FA00014250000;` | Setear frecuencia VFO A (14.250.000 Hz) |
| `MD1;`           | Setear modo LSB                         |
| `MD2;`           | Setear modo USB                         |
| `MD3;`           | Setear modo CW                          |
| `MD4;`           | Setear modo FM                          |
| `MD5;`           | Setear modo AM                          |
| `FR0;`           | Seleccionar VFO A                       |
| `FR1;`           | Seleccionar VFO B                       |
| `TX;`            | Transmitiendo (apaga BUSY)              |
| `RX;`            | Recibiendo (enciende BUSY)              |

### Indicadores disponibles

- Modos: LSB, USB, CW, AM, FM, NAR
- VFO: VFO A, VFO B
- Funciones: BAND, SCAN, SPLIT, BUSY, CLAR, LOCK, FAST, MR, PRI, GEN, CAT

## Herramienta de Testing

La carpeta [ht1621b_test/](ht1621b_test/) contiene una herramienta para probar y mapear segmentos:

```
a = encender todos los segmentos
c = apagar todos
m = mapeo manual (ENTER para siguiente)
w = walk automatico
s XX B = encender addr(hex) bit
r XX B = apagar addr(hex) bit
t XX = toggle addr
```
