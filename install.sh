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

cp configs/local/bin/dwmstatus.sh "$HOME/.local/bin/dwmstatus.sh"
chmod +x "$HOME/.local/bin/dwmstatus.sh"

cp configs/xinitrc "$HOME/.xinitrc"
chmod +x "$HOME/.xinitrc"
cp configs/xprofile "$HOME/.xprofile"
cp configs/fehbg "$HOME/.fehbg"
chmod +x "$HOME/.fehbg"

mkdir -p "$HOME/.dwm"
cp configs/dwm/autostart.sh "$HOME/.dwm/autostart.sh"
chmod +x "$HOME/.dwm/autostart.sh"

cp configs/wall.jpg "$HOME/Imagens/wall.jpg"

# Instalar entrada no display manager (para coexisitir com outras DEs)
echo "==> Instalando entrada no DM..."
sudo mkdir -p /usr/share/xsessions
sudo cp configs/dwm.desktop /usr/share/xsessions/dwm.desktop

echo "==> Suckless instalado com sucesso!"
