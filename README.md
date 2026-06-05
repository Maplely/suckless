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

## Instalar

```sh
cd dmenu-flexipatch && sudo make clean install && cd ..
cd dwm-flexipatch && sudo make clean install && cd ..
cd st-flexipatch && sudo make clean install && cd ..
cd slock && sudo make clean install && cd ..
```

## Desfazer (uninstall)

Remove os binários instalados:

```sh
cd dmenu-flexipatch && sudo make uninstall && cd ..
cd dwm-flexipatch && sudo make uninstall && cd ..
cd st-flexipatch && sudo make uninstall && cd ..
cd slock && sudo make uninstall && cd ..
```

Remove as dependências (opcional):

```sh
sudo pacman -Rns picom feh dunst xbindkeys maim xclip \
  pulseaudio-utils lm_sensors libnotify
```
