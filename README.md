![License](https://img.shields.io/badge/License-GPLv3-blue.svg)
[![GitHub release](https://img.shields.io/github/release/rdmrocha/linkalho.svg)](https://github.com/rdmrocha/linkalho/releases/latest/)
[![Github all releases](https://img.shields.io/github/downloads/rdmrocha/linkalho/total.svg)](https://GitHub.com/rdmrocha/linkalho/releases/latest/)
[![Buy me a coffee](https://img.shields.io/badge/buy%20me%20a%20coffee-donate-yellow.svg)](https://paypal.me/rdmrocha)

<p align="center"><img src="https://raw.githubusercontent.com/rdmrocha/linkalho/master/icon.jpg"></p>

# <b>Linkalho</b>

Linkalho is an homebrew app that will link NNID accounts offline. It links (or unlinks) <b>existing</b> accounts so you won't lose your saves.
This app does not create a new account. If you create a new account and want to link it, just re-run the app.

<br>

## <b>Why do I need this app?</b>
If you never had issues with games/apps failing to launch until you link your Switch user account and you never experienced issues with certain titles stuck in _Updating game data_ or certain official emulators presenting you with nothing else but a black screen instead of the game selection, then you don't need to use this homebrew.

<br>

Do you like this app and find it useful? You can buy me a coffee clicking the link below. Thanks!<br>
<a href="https://paypal.me/rdmrocha"><img src="raw/buy-me-a-coffee.png" width="150px" /></a>

<br>

## <b>Installation</b>:
- Place the .nro file in the `/switch/linkalho` folder of your SDcard.
- Go to the homebrew app and run Linkalho

<br>

### <u>Reboot to payload</u>
After completing the selected operation, Linkalho will try to find the best way to reboot to an existing payload:
- if the user places a payload file in the application's root (`/switch/linkalho/payload.bin`)
- if Atmosphere is detected and `/atmosphere/reboot_payload.bin` exists
- if ReiNX is detected and `/ReiNX.bin` exists
- if SXOS is detected it will issue a soft-reboot

<br>

## <b>Usage</b>:

### <u>Link all accounts</u>
- Will link all accounts on the console. If any of the existing accounts is already linked, it will be re-linked.
This operation creates a backup in `/switch/linkalho/backups`

### <u>Unlink all accounts</u>
- Will remove all NNID linking from any accounts on the console regardless of the NNIDs being officially linked or not.
This operation creates a backup in `/switch/linkalho/backups`

### <u>Restore backup</u>
- Restores any previous state from a backup file. The file must be places in `/switch/linkalho/restore/restore.zip`
This operation creates a backup in `/switch/linkalho/backups`

### <u>Backup accounts</u>
- Will create a backup in `/switch/linkalho/backups`.
All linking and unlinking operations will produce a backup before making changes.
<b>You should only use this option if you want to manually create a backup!</b>

<br>

## <b>Screenshots</b>
<p align="center"><img src="raw/screenshot1.jpg"></p>
<p align="center"><img src="raw/screenshot2.jpg"></p>
<p align="center"><img src="raw/screenshot3.jpg"></p>
<p align="center"><img src="raw/screenshot4.jpg"></p>
<p align="center"><img src="raw/screenshot5.jpg"></p>
<p align="center"><img src="raw/screenshot6.jpg"></p>
<p align="center"><img src="raw/screenshot7.jpg"></p>
<p align="center"><img src="raw/screenshot8.jpg"></p>
<p align="center"><img src="raw/screenshot9.jpg"></p>

<br>

## <b>Credits</b>
  - [devkitPro](https://devkitpro.org) for the toolchain!
  - [natinusala (lib borealis)](https://github.com/natinusala/borealis) for the amazing library that mimicks the Switch's original UI and UX
  - [sebastiandev (zipper wrapper for minizip)](https://github.com/sebastiandev/zipper/) for their nice wrapper to the minizip
  - [Kronos2308](https://github.com/Kronos2308) for the help in the initial phases of research.
  - [SciresM](https://github.com/SciresM) for his "reboot to payload" code and [HamletDuFromage](https://github.com/HamletDuFromage) for the code contributions
  - **stick2target** for the icon, the beta testing and for supplying crucial files that helped in the creation of the generators.
  - **boredomisacrime** for the beta testing.
