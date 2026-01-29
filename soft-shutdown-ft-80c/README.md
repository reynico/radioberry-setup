# Soft Shutdown para Yaesu FT-80C

Circuito y script para apagar la Raspberry Pi de forma segura cuando se apaga el equipo desde el boton frontal de power.

## Funcionamiento

1. Al apagar el equipo con SW1 (boton frontal), se corta la energia al equipo pero la Raspberry Pi sigue encendida gracias a GPIO14 que mantiene el relay K1 activo.
2. GPIO4 pierde energia cuando se apaga el equipo. El script `soft-shutdown.sh` detecta el nivel bajo y ejecuta un shutdown.
3. Durante el apagado, GPIO14 eventualmente pierde energia y deja de alimentar la base de Q3. El capacitor mantiene la carga por 5-6 segundos, permitiendo que la Pi se apague completamente sin corromper la tarjeta SD.

## Instalación

```bash
sudo apt install dunst
sudo cp soft-shutdown.sh /usr/local/bin/
sudo chmod +x /usr/local/bin/soft-shutdown.sh
sudo cp soft-shutdown.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable soft-shutdown
sudo systemctl start soft-shutdown
```

## Conexiones

| Función              | GPIO   |
| -------------------- | ------ |
| Deteccion de apagado | GPIO4  |
| Control relay K1     | GPIO14 |
