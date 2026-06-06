#!/bin/bash
pactl set-source-mute @DEFAULT_SOURCE@ toggle
pactl set-source-volume @DEFAULT_SOURCE@ 50%

STATE=$(pactl get-source-mute @DEFAULT_SOURCE@ | awk '{print $2}')
if [ "$STATE" = "yes" ]; then
    notify-send -t 1000 "MIC OFF"
else
    notify-send -t 1000 "MIC ON"
fi
