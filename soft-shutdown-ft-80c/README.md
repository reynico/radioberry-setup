# Soft Shutdown para Yaesu FT-80C

Circuito y script para apagar la Raspberry Pi de forma segura cuando se apaga el equipo desde el boton frontal de power.

## Funcionamiento

1. Al apagar el equipo con SW1 (boton frontal), se corta la energia al equipo pero la Raspberry Pi sigue encendida gracias a GPIO14 que mantiene el relay K1 activo.
2. GPIO15 pierde energia cuando se apaga el equipo. El script `soft-shutdown.sh` detecta el nivel bajo y ejecuta un shutdown.
3. GPIO14 se configura como output high. Se mantiene alto mientras la Pi esta encendida y cae a 0V cuando la Pi se apaga. Esto permite al circuito externo saber si la Pi esta viva.
4. Durante el apagado, GPIO14 eventualmente pierde energia y deja de alimentar la base de Q3. El capacitor mantiene la carga por 5-6 segundos, permitiendo que la Pi se apague completamente sin corromper la tarjeta SD.

## Configuracion de Raspberry Pi

Editar `/boot/firmware/config.txt`:

```ini
enable_uart=0
gpio=14=op,dh
gpio=15=ip,pd
```

Reiniciar despues de cambiar `config.txt`:

```bash
sudo reboot
```

Si `/boot/firmware/cmdline.txt` contiene `console=serial0,115200`, `console=ttyAMA0,115200` o `console=ttyS0,115200`, quitar ese parametro y dejar el archivo en una sola linea.

## Instalación

```bash
sudo apt install gpiod libnotify-bin dunst
sudo install -d /opt/soft-shutdown
sudo install -m 755 soft-shutdown.sh /opt/soft-shutdown/soft-shutdown.sh
sudo install -m 644 soft-shutdown.service /etc/systemd/system/soft-shutdown.service
sudo systemctl daemon-reload
sudo systemctl enable soft-shutdown
sudo systemctl start soft-shutdown
```

`libnotify-bin` instala `notify-send`. `dunst` es el daemon de notificaciones; si el escritorio ya tiene uno, puede no ser necesario.

## Conexiones

| Función              | GPIO   |
| -------------------- | ------ |
| Deteccion de apagado | GPIO15 |
| Pi encendida         | GPIO14 |

GPIO15 debe tener un pull-down a GND para que no quede flotando. Usar el pull-down interno (`gpio=15=ip,pd`) y, si es posible, agregar un resistor externo de 47k a 100k entre GPIO15 y GND.

GPIO14 entrega 3.3V cuando esta alto. No manejar el relay directamente desde el GPIO; usar transistor o MOSFET con resistor de base/compuerta.
