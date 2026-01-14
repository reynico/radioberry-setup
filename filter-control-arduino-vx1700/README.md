# Control de filtros para Vertex VX 1700

El Vertex VX 1700 maneja los 7 filtros pasabajos a través de un contador BCD que convierte binario a decimal, el [TC4028BP](TC4028BP.PDF). De las 4 entradas binarias usa solamente 3: A (pin 10), B (pin 13) y C (pin 12), ya que son suficientes para mapear los 7 filtros. El programa para el Arduino convierte los datos binarios recibidos desde la Radioberry, usando el mismo protocolo que la Alex Filter Board.

| Banda | Comando (Alex) | C   | B   | A   | Filtro seleccionado | Frecuencia |
| ----- | -------------- | --- | --- | --- | ------------------- | ---------- |
| 160M  | 1608           | 0   | 0   | 1   | BPF1                | 1.8 MHz    |
| 80M   | 1604           | 0   | 1   | 0   | BPF2                | 3.5 MHz    |
| 60M   | 1602           | 0   | 1   | 1   | BPF3                | 5.3 MHz    |
| 40M   | 802            | 1   | 0   | 0   | BPF4                | 7 MHz      |
| 30M   | 401            | 1   | 0   | 1   | BPF5                | 10.1 MHz   |
| 20M   | 101            | 1   | 1   | 0   | BPF6                | 14 MHz     |
| 17M   | 164            | 1   | 1   | 0   | BPF6                | 18.1 MHz   |
| 15M   | 264            | 1   | 1   | 1   | BPF7                | 21 MHz     |
| 12M   | 4              | 1   | 1   | 1   | BPF7                | 24.9 MHz   |
| 10M   | 4              | 1   | 1   | 1   | BPF7                | 28 MHz     |


## Tabla de conexiones

El Arduino toma su alimentación de la línea de 3.3v de la Radioberry y se comunica con ella a través del protocolo I2C. Salida TX y Salida PA son para manejar la transmisión de un amplificador y su encendido. (PA = Power Amplifier). El diagrama esquemático adjunto indica como conectar todos los pines.

| Función               | Pin Radioberry | Pin Arduino | Pin TC4028BP |
| --------------------- | -------------- | ----------- | ------------ |
| **Datos I2C**         | 17 (SDA)       | A4 (SDA)    | -            |
| **Reloj I2C**         | 18 (SCL)       | A5 (SCL)    | -            |
| **Tierra**            | GND            | GND         | VSS (Pin 8)  |
| **Alimentación 3.3V** | 2 (3V3)        | 3.3V        | VDD (Pin 16) |
| **Control BPF A**     | -              | Pin 9       | A (Pin 10)   |
| **Control BPF B**     | -              | Pin 10      | B (Pin 13)   |
| **Control BPF C**     | -              | Pin 11      | C (Pin 12)   |
| **Entrada PTT**       | 3 (PTT Out)    | Pin 8       | -            |
| **Salida TX**         | -              | Pin 7       | -            |
| **Salida PA**         | -              | Pin 6       | -            |