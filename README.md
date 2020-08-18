# 🌱 herbe
> Daemon-less notifications without D-Bus. Minimal and lightweight.

![herbe](https://raw.githubusercontent.com/dudik/herbe/master/herbe.png)

## Usage
```shell
$ herbe "herbe notifications" " " "Daemon-less notifications without D-Bus. Minimal and lightweight."
```
will display the notification shown above

* [Dismiss a notification](#dismiss-a-notification)
* [Actions](#actions)
* [Newlines](#newlines)
* [Multiple notifications](#multiple-notifications)
* [Notifications don't show up](#notifications-dont-show-up)

### Dismiss a notification
A notification can be dismissed either by clicking on it with `DISMISS_BUTTON` (set in config.h, defaults to left mouse button) or sending a `SIGUSR1` signal to it:
```shell
$ pkill -SIGUSR1 herbe
```

### Actions
Action is a piece of shell code that runs when a notification gets accepted. Accepting a notification is the same as dismissing it, but you have to use either `ACTION_BUTTON` (defaults to right mouse button) or the `SIGUSR2` signal.
An accepted notification always returns exit code 3. To specify an action:
```shell
$ herbe "Notification body" ; [ $? -eq 3 ] && echo "This is an action"
```
Where everything after `&&` is the action and will get executed after the notification gets accepted.

### Newlines
Every command line argument gets printed on a separate line by default e.g.:
```shell
$ herbe "First line" "Second line" "Third line" ...
```
You can also use `\n` e.g. in `bash`:
```shell
$ herbe $'First line\nSecond line\nThird line'
```
But by default `herbe` prints `\n` literally:
```shell
$ herbe "First line\nStill the first line"
```
Output of other programs will get printed correctly, just make sure to escape it (so you don't end up with every word on a separate line):
```shell
$ herbe "$(ps axch -o cmd:15,%cpu --sort=-%cpu | head)"
```

### Multiple notifications
Notifications are put in a queue and shown one after another in order of creation (first in, first out). They don't overlap and each one is shown for its entire duration.

### Notifications don't show up
Most likely a running notification got terminated forcefully (SIGKILL or any uncaught signal) which caused the semaphore not getting unlocked. First, kill any `herbe` instance that is stuck:
```shell
$ pkill -SIGKILL herbe
```
Then just call `herbe` without any arguments:
```shell
$ herbe
```
Notifications should now show up as expected.

Don't ever send any signals to `herbe` except these:
```shell
# same as pkill -SIGTERM herbe, terminates every running herbe process
$ pkill herbe

$ pkill -SIGUSR1 herbe
$ pkill -SIGUSR2 herbe
```
And you should be fine. That's all you really need to interact with `herbe`.

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
