# 🌱 herbe
> Daemon-less notifications without D-Bus. Minimal and lightweight.

<p align="center">
  <img src="https://user-images.githubusercontent.com/24730635/90975811-cd62fd00-e537-11ea-9169-92e68a71d0a0.gif" />
</p>

## Features
* Under 200 lines of code
* Doesn't run in the background, just displays the notification and exits
* No external dependencies except Xlib and Xft
* Configurable through `config.h` or Xresources ([using this patch](https://github.com/dudik/herbe/pull/11))
* [Actions support](#actions)
* Extensible through [patches](https://github.com/dudik/herbe/pulls?q=is%3Aopen+is%3Apr+label%3Apatch)

## Table of contents

* [Usage](#usage)
  * [Patches](#patches)
  * [Dismiss a notification](#dismiss-a-notification)
  * [Actions](#actions)
  * [Newlines](#newlines)
  * [Multiple notifications](#multiple-notifications)
  * [Notifications don't show up](#notifications-dont-show-up)
* [Installation](#installation)
  * [Packages](#packages)
  * [Dependencies](#dependencies)
  * [Build](#build)
* [Configuration](#configuration)
* [Contribute](#contribute)

## Usage

### Patches
[List of available patches](https://github.com/dudik/herbe/pulls?q=is%3Aopen+is%3Apr+label%3Apatch)

To create a new patch you'll have to open a pull request with your changes. Append `.diff` to the pull request URL to get a downloadable diff file. Don't forget to prefix the title with `patch:` and to apply the `patch` label to it. For inspiration, look at [my Xresources patch](https://github.com/dudik/herbe/pull/11). Thank you.

_Note: This patching method was heavily inspired by [dylan's sowm](https://github.com/dylanaraps/sowm)._

### Dismiss a notification
A notification can be dismissed either by clicking on it with `DISMISS_BUTTON` (set in config.h, defaults to left mouse button) or sending a `SIGUSR1` signal to it:
```shell
$ pkill -SIGUSR1 herbe
```
Dismissed notifications return exit code 2.

### Actions
Action is a piece of shell code that runs when a notification gets accepted. Accepting a notification is the same as dismissing it, but you have to use either `ACTION_BUTTON` (defaults to right mouse button) or the `SIGUSR2` signal.
An accepted notification always returns exit code 0. To specify an action:
```shell
$ herbe "Notification body" && echo "This is an action"
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
### Packages
[![Packaging status](https://repology.org/badge/vertical-allrepos/herbe.svg)](https://repology.org/project/herbe/versions)

[OpenBSD patch](https://github.com/dudik/herbe/pull/4)

[FreeBSD patch](https://github.com/dudik/herbe/pull/16)

[Wayland port](https://github.com/muevoid/Wayherb) by [muevoid](https://github.com/muevoid)

**Only the [herbe-git AUR package](https://aur.archlinux.org/packages/herbe-git/) is maintained by me.**

### Dependencies
* X11 (Xlib)
* Xft

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

[Xresources patch](https://github.com/dudik/herbe/pull/11)

## Contribute
If you want to report a bug or you have a feature request, feel free to [open an issue](https://github.com/dudik/herbe/issues).
