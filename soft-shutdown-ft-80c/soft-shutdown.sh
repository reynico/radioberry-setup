#!/bin/bash

CHIP=0
SENSE_PIN=15
HOLD_PIN=14
LOW_COUNT=0
REQUIRED_LOW_COUNT=2
NOTIFY_SECONDS=5

configure_gpio() {
    if command -v pinctrl >/dev/null 2>&1; then
        pinctrl set "$HOLD_PIN" op dh
        pinctrl set "$SENSE_PIN" ip pd
    elif command -v raspi-gpio >/dev/null 2>&1; then
        raspi-gpio set "$HOLD_PIN" op dh
        raspi-gpio set "$SENSE_PIN" ip pd
    else
        logger -t soft-shutdown "pinctrl/raspi-gpio not found; relying on boot GPIO configuration"
    fi
}

send_notification() {
    if ! command -v notify-send >/dev/null 2>&1; then
        logger -t soft-shutdown "notify-send not installed; skipping desktop notification"
        return
    fi

    local user=$(who | grep -m1 'tty' | awk '{print $1}')
    if [ -n "$user" ]; then
        sudo -u "$user" DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u "$user")/bus \
            notify-send -u critical "Shutdown" "System will power off in ${NOTIFY_SECONDS} seconds"
    fi
}

configure_gpio

while true; do
    if ! VAL=$(gpioget -c "$CHIP" "$SENSE_PIN" 2>/dev/null); then
        logger -t soft-shutdown "could not read GPIO$SENSE_PIN from gpiochip$CHIP"
        sleep 2
        continue
    fi

    if echo "$VAL" | grep -q "inactive"; then
        LOW_COUNT=$((LOW_COUNT + 1))
        if [ "$LOW_COUNT" -ge "$REQUIRED_LOW_COUNT" ]; then
            logger -t soft-shutdown "GPIO$SENSE_PIN inactive for ${REQUIRED_LOW_COUNT} cycles, shutting down"
            send_notification
            sleep $NOTIFY_SECONDS
            poweroff
            exit 0
        fi
    else
        LOW_COUNT=0
    fi

    sleep 2
done
