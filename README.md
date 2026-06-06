# suckless

Configuração personalizada do dwm, dmenu, st, slock, picom, dunst e X11.

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

## Instalar (tudo de uma vez)

```sh
./install.sh
```

Isso compila e instala os 4 programas suckless **e** copia as configurações de
picom, dunst, xbindkeys, Xresources, xinitrc, autostart, status bar, wallpaper
e cria a entrada `/usr/share/xsessions/dwm.desktop` para o DM.

Assim o DWM aparece na lista de sessões junto com outras DEs instaladas.

## Instalar só os binários

```sh
cd dmenu-flexipatch && sudo make clean install && cd ..
cd dwm-flexipatch && sudo make clean install && cd ..
cd st-flexipatch && sudo make clean install && cd ..
cd slock && sudo make clean install && cd ..
```

## Desfazer (uninstall)

Remove os binários e as configurações:

```sh
./uninstall.sh
```

Remove só os binários:

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

## Estrutura

```
suckless/
├── configs/
│   ├── picom/picom.conf
│   ├── dunst/dunstrc
│   ├── xbindkeysrc
│   ├── Xresources
│   ├── xinitrc
│   ├── xinitrc.local.example
│   ├── xprofile
│   ├── dwm.desktop
│   ├── fehbg
│   ├── wall.jpg
│   ├── mic-toggle.sh
│   ├── dwmstatus.sh
│   └── dwm/autostart.sh
├── dmenu-flexipatch/
├── dwm-flexipatch/
├── st-flexipatch/
├── slock/
├── install.sh
└── uninstall.sh
```
