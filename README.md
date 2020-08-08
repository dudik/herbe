# 🌱 herbe
> Daemon-less notifications without D-Bus. Minimal and lightweight.

![herbe](https://raw.githubusercontent.com/dudik/herbe/master/herbe.png)

## Usage
```shell
herbe "herbe notifications" " " "Daemon-less notifications without D-Bus. Minimal and lightweight."
```
will display the notification shown above

## Installation
### Dependencies
* X11 (Xlib)
* Xft
* freetype2

The names of packages are different depending on which distribution you use.
For example, if you use [Void Linux](https://voidlinux.org/) you will have to install these dependencies:
```shell
sudo xbps-install base-devel libX11-devel libXft-devel
```

### Build
```shell
git clone https://github.com/dudik/herbe
cd herbe
sudo make install
```
`make install` requires root privileges because it copies the resulting binary to `/usr/local/bin`. This makes `herbe` accessible globally.

You can also use `make clean` to remove the binary from the build folder, `sudo make uninstall` to remove the binary from `/usr/local/bin` or just `make` to build the binary locally.

## Configuration
herbe is configured at compile-time by editing `config.h`. Every option should be self-explanatory. There is no `height` option because height is determined by font size and text padding.

## Contribute
If you want to report a bug or you have a feature request, feel free to [open an issue](https://github.com/dudik/herbe/issues).
