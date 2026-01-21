# Analisis de Factibilidad: Control LCD Yaesu FT-747GX / FT-80C

## Objetivo

Reemplazar el controlador original M50932 con un HT1621B + Arduino para controlar directamente el display LCD propietario FTD8627PZ.

## Sistema Original

- **Radio**: Yaesu FT-747GX / FT-80C
- **Display**: DS01 FTD8627PZ (LCD propietario Yaesu)
- **Controlador original**: Q01 M50932-XXXFP (microcontrolador Mitsubishi con LCD driver integrado)

## Especificaciones del M50932

| Parametro    | Valor                                            |
| ------------ | ------------------------------------------------ |
| COM outputs  | 4 (COM0~COM3)                                    |
| SEG outputs  | 32 (SEG0~SEG23 + SEG24~SEG31 compartidos con P4) |
| Bias         | 1/2 o 1/3 seleccionable                          |
| Duty         | 1/2, 1/3, o 1/4 seleccionable                    |
| Vcc          | 2.7V ~ 5.5V                                      |
| Voltajes LCD | VL1, VL2, VL3 para niveles intermedios           |

## Especificaciones del HT1621B

| Parametro   | Valor                 |
| ----------- | --------------------- |
| COM outputs | 4                     |
| SEG outputs | 32                    |
| Bias        | 1/2 o 1/3             |
| Duty        | 1/2, 1/3, 1/4         |
| Vcc         | 2.4V ~ 5.2V           |
| Interfaz    | Serial (CS, WR, DATA) |
| RAM         | 32 x 4 bits           |

## Analisis de Señales

### COM0
- Captura: [captura-m50932-com0.png](captura-m50932-com0.png)
- Escala: 5V, 5ms/div
- Observacion: Señal tipica LCD multiplexado con 3 niveles (~0V, ~2.5V, ~5V)
- Indica: **Bias 1/3** con voltaje intermedio en Vcc/2
- Periodo frame: ~33ms (~30Hz frame rate)

### SEG8
- Captura: [captura-m50932-seg8.png](captura-m50932-seg8.png)
- Escala: 5V, 5ms/div
- Observacion: Señal cuadrada con transiciones rapidas
- Patron consistente con **duty 1/3** (3 COM activos)

## Comparacion de Compatibilidad

| Parametro  | M50932        | HT1621B       | Display FTD8627PZ |
| ---------- | ------------- | ------------- | ----------------- |
| COM        | 4             | 4             | COM0-COM2         |
| SEG        | 32            | 32            | 32                |
| Bias       | 1/2, 1/3      | 1/2, 1/3      | 1/3               |
| Duty       | 1/2, 1/3, 1/4 | 1/2, 1/3, 1/4 | 1/3               |
| Vcc        | 2.7V~5.5V     | 2.4V~5.2V     | -                 |
| Frame freq | ~30Hz         | Configurable  | ~30Hz             |

## Conteo de Segmentos del Display FTD8627PZ

### Indicadores de Texto (izquierda)

| Indicador    | Segmentos |
| ------------ | --------- |
| BAND         | 1         |
| SCAN         | 1         |
| SPLIT        | 1         |
| VFO A        | 1         |
| BUSY         | 1         |
| CLAR         | 1         |
| VFO B        | 1         |
| LOCK         | 1         |
| FAST         | 1         |
| M R          | 1         |
| **Subtotal** | **10**    |

### Indicadores de Modo (arriba)

| Indicador    | Segmentos |
| ------------ | --------- |
| LSB          | 1         |
| USB          | 1         |
| CW           | 1         |
| AM           | 1         |
| FM           | 1         |
| NAR          | 1         |
| CH           | 1         |
| **Subtotal** | **7**     |

### Display Numerico

| Elemento                          | Segmentos |
| --------------------------------- | --------- |
| 5 digitos grandes (7-seg)         | 35        |
| Puntos decimales                  | ~3        |
| 2 digitos pequeños CH (7-seg)     | 14        |
| PRI                               | 1         |
| Flechas (triangulos arriba/abajo) | 2         |
| **Subtotal**                      | **~55**   |

### Total Estimado: ~72 segmentos

## Esquema de Conexion

```
Arduino Nano/Uno    HT1621B              Display FTD8627PZ
----------------    -------              -----------------
D4  --------------> CS
D5  --------------> WR
D6  --------------> DATA
                    COM0 --------------> COM0
                    COM1 --------------> COM1
                    COM2 --------------> COM2
                    (COM3 no se usa)
                    SEG0~SEG31 --------> SEG0~SEG31
5V  --------------> VDD
5V --[diodo]------> VLCD (~4.3V)
GND --------------> VSS
```

**IMPORTANTE**: El M50932 original debe estar **aislado fisicamente** del display
(cortar pistas o desoldar). Si queda conectado en paralelo (aunque sin alimentacion),
sus diodos de proteccion internos cargan las lineas y reducen el voltaje de las senales.

## Cableado Display - Esquema de Colores

| Color     | Conexiones       | Cantidad      |
| --------- | ---------------- | ------------- |
| Naranja   | COM0, COM1, COM2 | 3             |
| Rojo      | SEG0 - SEG5      | 6             |
| Marron    | SEG6 - SEG11     | 6             |
| Azul      | SEG12 - SEG17    | 6             |
| Verde     | SEG18 - SEG23    | 6             |
| Violeta   | SEG24 - SEG29    | 6             |
| Amarillo  | SEG30 - SEG31    | 2             |
| **Total** |                  | **35 cables** |

## Configuracion HT1621B Requerida

Comandos de inicializacion:
- `SYS_EN` (0x01) - Habilitar sistema
- `LCD_ON` (0x03) - Encender LCD
- `BIAS_1/3_3COM` (0x25) - Configurar bias 1/3, 3 COM
- `RC_256K` (0x18) - Oscilador RC interno

**Nota**: El display usa solo 3 COM (COM0, COM1, COM2). COM3 no esta conectado.

## Arduino - Segment Scanner

### Archivos

- `ht1621b_scan/ht1621b_scan.ino` - Codigo para escanear y mapear segmentos
- `segment_map_template.csv` - Plantilla para documentar el mapeo

### Conexion Arduino -> HT1621B

```
Arduino     HT1621B              Display FTD8627PZ
-------     -------              -----------------
D4   -----> CS   (pin 9)
D5   -----> WR   (pin 11)
D6   -----> DATA (pin 12)
5V -------> VDD  (pin 17)
5V --[D]--> VLCD (pin 16)        (~4.3V con diodo)
GND ------> VSS  (pin 13)
            COM0 --------------> COM0
            COM1 --------------> COM1
            COM2 --------------> COM2
            (COM3 no se usa)
            SEG0~SEG31 --------> SEG0~SEG31
```

### Contraste (VLCD)

El contraste del LCD depende del voltaje en el pin VLCD del HT1621B.

**IMPORTANTE**: VLCD debe ser **menor que VDD** segun el datasheet.

| VLCD  | Resultado                             |
| ----- | ------------------------------------- |
| ~4.3V | Buen contraste (probado, funciona)    |
| ~3.3V | Voltaje insuficiente, senales debiles |
| < 2V  | Muy tenue o invisible                 |

**Configuracion recomendada**: Usar un diodo en serie desde 5V para obtener ~4.3V en VLCD.

```
5V --[diodo 1N4148]--> VLCD (~4.3V)
```

### Comandos Serial (115200 baud)

| Comando | Funcion                            |
| ------- | ---------------------------------- |
| `s`     | Scan automatico (2s por segmento)  |
| `f`     | Scan rapido (500ms por segmento)   |
| `m`     | Scan manual (ENTER para siguiente) |
| `a`     | Encender TODOS los segmentos       |
| `c`     | Apagar todos los segmentos         |
| `t XX`  | Probar direccion especifica (hex)  |

### Procedimiento de Mapeo

1. Conectar Arduino + HT1621B al display
2. Abrir Serial Monitor (115200 baud)
3. Enviar `a` para verificar que todo funciona (todos los segmentos encendidos)
4. Enviar `c` para apagar
5. Enviar `m` para scan manual
6. Por cada segmento que se enciende, anotar en `segment_map_template.csv`:
   - ELEMENT: ej. "DIG1_A", "BAND", "LSB", "DP1"
   - DESCRIPTION: ej. "Digito 1 segmento A", "Indicador BAND", "Punto decimal 1"

### Nomenclatura sugerida para segmentos

```
Digitos 7-segmentos:        Indicadores:
      A                     BAND, SCAN, SPLIT, VFO_A
     ---                    BUSY, CLAR, VFO_B
  F |   | B                 LOCK, FAST, M_R
     -G-                    LSB, USB, CW, AM, FM, NAR, CH
  E |   | C                 PRI, ARROW_UP, ARROW_DN
     ---
      D

DIG1_A = Digito 1, segmento A
DIG1_DP = Digito 1, punto decimal
CH1_A = Canal digito 1, segmento A
```

## Referencias

- Datasheet M50932: [M50930.PDF](M50930.PDF)
- Datasheet HT1621B: [ht1621b.PDF](ht1621b.PDF)
- Esquematico FT-747: [FT-747_Schematic.pdf](FT-747_Schematic.pdf) (pagina 4 - Display Unit)
- Capturas osciloscopio M50932 original: [captura-m50932-com0.png](captura-m50932-com0.png),[captura-m50932-seg8.png](captura-m50932-seg8.png)
- Capturas osciloscopio HT1621B: [captura-ht1621-com0.png](captura-ht1621-com0.png),[captura-ht1621-seg8.png](captura-ht1621-seg8.png)

## Notas

- El display muestra frecuencia en formato `XX.XXX.X` MHz
- Los 2 digitos pequeños + "CH" son para numero de canal/memoria
- Las flechas triangulares indican direccion de sintonizacion
- "PRI" indica canal prioritario en modo scan
