# suckless

Configuração personalizada do dwm, dmenu, st e slock.

## Dependências

### Build

```sh
sudo pacman -S --needed base-devel libx11 libxft libxinerama libxrender \
  libxrandr libxext libxfixes fontconfig pango fribidi imlib2 libxcb yajl
```

### Runtime

```sh
sudo pacman -S --needed picom feh dunst xbindkeys maim xclip \
  pulseaudio-utils lm_sensors libnotify polkit-gnome \
  xdg-desktop-portal-gtk xdg-desktop-portal pcmanfm thunar
```

### Compilar e instalar

```sh
cd dmenu-flexipatch && sudo make clean install && cd ..
cd dwm-flexipatch && sudo make clean install && cd ..
cd st-flexipatch && sudo make clean install && cd ..
cd slock && sudo make clean install && cd ..
```
