#!/bin/bash

PREV_IDLE=0
PREV_TOTAL=0

while true; do
    # CPU
    STAT=$(grep 'cpu ' /proc/stat)
    CURR_IDLE=$(echo "$STAT" | awk '{print $5}')
    CURR_TOTAL=$(echo "$STAT" | awk '{for(i=2;i<=NF;i++) s+=$i; print s}')

    if [ "$PREV_TOTAL" -ne 0 ] 2>/dev/null; then
        DIFF_IDLE=$((CURR_IDLE - PREV_IDLE))
        DIFF_TOTAL=$((CURR_TOTAL - PREV_TOTAL))
        CPU=$((100 - (DIFF_IDLE * 100 / DIFF_TOTAL)))
    else
        CPU=0
    fi

    PREV_IDLE=$CURR_IDLE
    PREV_TOTAL=$CURR_TOTAL

    # Memory
    MEM=$(free -h | awk '/^Mem:/ {print $3"/"$2}')

    # Temperature
    TEMP=$(sensors | awk '/^Package id 0:/ {gsub(/[+°C]/,"",$4); print $4}')

    # Volume
    VOL=$(pactl get-sink-volume @DEFAULT_SINK@ | grep -Po '[0-9]+(?=%)' | head -1)%

    # Clock
    CLOCK=$(date "+%H:%M")
    DATE=$(date "+%d/%m")

    xsetroot -name "  ${CPU}% │  $MEM │  ${TEMP}°C │  $VOL │  $CLOCK  $DATE "
    sleep 1
done
