# Control de filtros para Yaesu FT-80c/FT-747

El Yaesu FT-80c/FT-747/FT-747GX maneja los 6 filtros pasabanda a través de un contador BCD que convierte binario a decimal, el [μPD4028](1372-4028.pdf). Pero dado que el contador BCD está instalado en la main board, y la main board es mas probable que la eliminemos, este control de filtros, a diferencia del control de filtros del [Vertex VX1700](../filter-control-arduino/), maneja los 6 filtros directamente a través de un [UDN2981](UDN2981.pdf) (la versión PNP del ULN2003). El programa para el Arduino convierte los datos binarios recibidos desde la Radioberry, usando el mismo protocolo que la Alex Filter Board. El control de filtros también controla el relay de TX/RX que está dentro del LPF.

| Banda | Comando (Alex) | Salida Arduino | Filtro seleccionado | Cable color |
| ----- | -------------- | -------------- | ------------------- | ----------- |
| 160M  | 1608           | D8             | 1.8                 | Rojo        |
| 80M   | 1604           | D7             | 3.5                 | Naranja     |
| 60M   | 1602           | D6             | 7                   | Amarillo    |
| 40M   | 802            | D6             | 7                   | Amarillo    |
| 30M   | 401            | D5             | 10/14               | Verde       |
| 20M   | 101            | D5             | 10/14               | Verde       |
| 17M   | 164            | D4             | 18/21               | Azul        |
| 15M   | 264            | D4             | 18/21               | Azul        |
| 12M   | 232            | D3             | 24.5/28             | Violeta     |
| 10M   | 232            | D3             | 24.5/28             | Violeta     |


## Tabla de conexiones

El Arduino toma su alimentación de la línea de 3.3v de la Radioberry y se comunica con ella a través del protocolo I2C. Salida TX y Salida PA son para manejar la transmisión de un amplificador y su encendido. (PA = Power Amplifier). El diagrama esquemático adjunto indica como conectar todos los pines.

| Función               | Pin Radioberry | Pin Arduino | Pin TC4028BP |
| --------------------- | -------------- | ----------- | ------------ |
| **Datos I2C**         | 17 (SDA)       | A4 (SDA)    | -            |
| **Reloj I2C**         | 18 (SCL)       | A5 (SCL)    | -            |
| **Tierra**            | GND            | GND         | VSS (Pin 8)  |
| **Alimentación 3.3V** | 2 (3V3)        | 3.3V        | VDD (Pin 16) |
| **Entrada PTT**       | 3 (PTT Out)    | Pin A6      | -            |
| **Salida TX**         | -              | Pin 13      | -            |
| **Salida PA**         | -              | Pin 9       | -            |

Para conectar el PTT (MOX) del Yaesu FT-80c/FT-747/FT-747GX es necesario un divisor resistivo para bajar los 13.5v a algo razonable para el Arduino (~4.5v), para ello usé dos resistencias: 21k y 10k ohms.

Para manejar el relay de TX del LPF del Yaesu FT-80c/FT-747/FT-747GX, puse una resistencia de 180 ohms en serie para bajar la tensión a aproximadamente 7.5 volts que es lo que medí cuando probé la main board del equipo, en lugar de mandarle los 13.5v directo. Los relays de cada filtro si manejan 13.5v. El consumo del relay de TX es de aproximadamente 35 mA.