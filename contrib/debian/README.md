
Debian
====================
This directory contains files used to package wizblcoind/wizblcoin-qt
for Debian-based Linux systems. If you compile wizblcoind/wizblcoin-qt yourself, there are some useful files here.

## wizblcoin: URI support ##


wizblcoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install wizblcoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your wizblcoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/wizblcoin128.png` to `/usr/share/pixmaps`

wizblcoin-qt.protocol (KDE)

