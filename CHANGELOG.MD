# Changelog

## [0.0.1] - 2026-08-29 / 00:36

### Added

* Initial `main.c`
* Initial Git repository

## [0.0.2] - 2026-08-29 / 09:10

### Added

* Just a `printf()`

## [0.0.3] - 2026-08-29 / 09:12

### Added

* Used `argc` & `argv`

## [0.0.4] - 2026-08-29 / 09:15

### Added

* Usage of `getcwd()`

## [0.1.0] - 2026-08-29 / 14:40

### Added

* We can read what's inside of a folder, damn! (lines 20–31)

## [0.1.1] - 2026-09-02 / 00:30

### Added

* We can now see if the listed objects are files or folders!

## [0.1.2] - 2026-09-02 / 01:05

### Added

* You can cycle through every detected element in the given folder.

## [0.1.3] - 2026-09-02 / 17:15

### Added

* Same as v0.1.2 but it looks nicer

## [0.2.0] - 2026-09-03 / 15:45

### Added

* Proud to announce v0.2.0 !! Now its partially functionnal, you can go through any folder you'd like !

## [0.2.1] - 2026-09-04 / 15:15

### Added

* Works on any linux computer, fixed the path problem, now it always opens /home/$USER on launch.

## [0.2.2] - 2026-09-04 / 15:55

### Added

* White bg and black font color for the selected item.

## [0.3.0] - 2026-09-04 / 16:40

### Added

* When pressing 'o' on a selected file (works only on files for now), will open a new 'informations menu' that is empty for now except for the name of the file

## [0.3.1] - 2026-09-04 / 16:50

### Added

* On the 'informations' menu, added the size of the opened file in bytes.

## [0.3.1.1] - 2026-09-04 / 16:55

### Fix

* Fixed the max size being 4096 bytes.
* Fixed the selected folder was not being displayed when leaving the informations menu

## [0.3.2] - 2026-09-04 / 21:05

### Added

* **In informations, for FILES, added :**
- Permissions
- Date
- Owner
- if the file a symlink ?

* **In informations, for FOLDER, added :**
- Size (i know it is 4096 bytes for now, it will be changed)
- Permissions
- Date
- Owner
- if the folder a symlink ?

## [0.3.2.1] - 2026-09-04 / 21-15

### Fix

* Fixed the changelog (again)

## [0.3.2.2] - 2026-09-04 / 22:20

### Fix

* Readme fix... (am I that stupid ?)

## [0.3.2.3] - 2026-09-04 / 23:35

### Fix

* Now the size of a folder is the real size of whats inside of it !

## [0.3.2.4] - 2026-09-04 / 23:38

### Fix

* Uhm, needed to fix the readme..

## [0.3.2.5] - 2026-09-04 / 23:50

### Update

* Now folder size is displayed in B, KiB, MiB, GiB or TiB depending on the size of the folder !

## [0.3.2.6] - 2026-09-04 / 23:54

### Fix

* Function refreshListFile did not update modified, owner and symlink correctly, fixed !

## [0.4.0.0] - 2026-09-04 / 13:55

### Added

- `c` key -> opens the copy menu (empty for now).
- `m` key -> opens the move menu (empty for now).
- `d` key -> deletes the selected folder/file (not working for now).
- `r` key -> opens the rename menu (empty for now).
- `t` key -> opens the mkdir / touch menu (empty for now).

### QOL

- Return now allows navigation to the parent folder, so by pressing `RETURN` key you will go back in the parent folder.

### Changed

- Folder names have now a `/` at the end (looks nicer than nothing).
- Selected files/folder now have a `--->` before them.
- The main column is larger for larger names of files.