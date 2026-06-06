#!/bin/sh
set -e

# Instalar os binários suckless
for dir in dmenu-flexipatch dwm-flexipatch st-flexipatch slock; do
	echo "==> Instalando $dir..."
	cd "$dir" && sudo make clean install && cd ..
done

# Copiar configurações
echo "==> Copiando configurações..."

mkdir -p "$HOME/.config/picom"
cp configs/picom/picom.conf "$HOME/.config/picom/picom.conf"

mkdir -p "$HOME/.config/dunst"
cp configs/dunst/dunstrc "$HOME/.config/dunst/dunstrc"

cp configs/xbindkeysrc "$HOME/.xbindkeysrc"
cp configs/Xresources "$HOME/.Xresources"

mkdir -p "$HOME/.local/bin"
cp configs/mic-toggle.sh "$HOME/.local/bin/mic-toggle.sh"
chmod +x "$HOME/.local/bin/mic-toggle.sh"

echo "==> Suckless instalado com sucesso!"
