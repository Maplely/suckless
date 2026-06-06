#!/bin/sh
# ~/.dwm/autostart.sh — executado toda vez que o DWM inicia

feh --bg-scale ~/Imagens/wall.jpg &
dunst &
xbindkeys &
~/.local/bin/dwmstatus.sh &

