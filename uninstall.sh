#!/bin/sh
set -e

# Desinstalar os binários suckless
for dir in dmenu-flexipatch dwm-flexipatch st-flexipatch slock; do
	echo "==> Desinstalando $dir..."
	cd "$dir" && sudo make uninstall && cd ..
done

# Remover configurações (opcional)
echo ""
echo "Deseja remover as configurações? (Ctrl+C para cancelar, Enter para continuar)"
read -r _

rm -f "$HOME/.config/picom/picom.conf"
rm -f "$HOME/.config/dunst/dunstrc"
rm -f "$HOME/.xbindkeysrc"
rm -f "$HOME/.Xresources"
rm -f "$HOME/.local/bin/mic-toggle.sh"

echo "==> Suckless desinstalado!"
