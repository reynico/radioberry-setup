#!/bin/bash

CHIP=0
PIN=4
LOW_COUNT=0
REQUIRED_LOW_COUNT=2
NOTIFY_SECONDS=5

send_notification() {
    local user=$(who | grep -m1 'tty' | awk '{print $1}')
    if [ -n "$user" ]; then
        sudo -u "$user" DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$(id -u "$user")/bus \
            notify-send -u critical "Shutdown" "System will power off in ${NOTIFY_SECONDS} seconds"
    fi
}

while true; do
    VAL=$(gpioget -c $CHIP $PIN)

    if echo "$VAL" | grep -q "inactive"; then
        LOW_COUNT=$((LOW_COUNT + 1))
        if [ "$LOW_COUNT" -ge "$REQUIRED_LOW_COUNT" ]; then
            logger -t soft-shutdown "GPIO$PIN inactive for ${REQUIRED_LOW_COUNT} cycles, shutting down"
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
