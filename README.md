# Annaora

**Annaora** is a file manager written from scratch in C & QML for Linux.

> **Early development — not functional yet**

The project is being built from the very beginning as a way to learn and explore **C programming, filesystem management, terminal interfaces, and software architecture**.

The long-term goal is to turn Annaora into a **fully customizable graphical file manager**, powered by Qt/QML and controlled through a custom configuration language.

---

## Vision

**Annaora is planned in two major stages :**

### Engine 1.0 — C / Linux

The first version focuses on building a complete and functional file manager in C that you can use in the terminal.

Planned features include:

- Directory navigation
- File management
- Copy / move / rename
- File deletion
- File search
- File information
- Permissions
- Keyboard navigation
- Terminal interface
- Configuration

The goal of Engine 1.0 is to make functional file manager and to build a solid understanding of how filesystems and Linux work at a low level.

---

## Engine 2.0 — Qt / QML

After Engine 1.0, Annaora will evolve into a graphical and highly customizable file manager using **Qt/QML**.
The interface will be designed to be customizable by the user without having to modify the source code.
For example, a future Annaora configuration could look like:

```
[folder-list]

border: 5px;
bg-color: #ffffff;
pos: ((100, 100)(200, 200));
```

The configuration language will eventually support things such as:

- UI elements
- Position and dimensions
- Colors
- Borders
- Fonts
- Layouts
- States
- Animations
- User interactions

The configuration will be parsed by Annaora and translated into the graphical interface.
The final goal is to make the UI **completely customizable by the user**.

---

## Development

Annaora is currently in the early stages of development.
Due to school starting again the developpment might be impacted and slowed so don't expect a 1.0 release too soon.

### Current version

**v0.3.2.0 - User can go through the folders like a real terminal based file explorer and colors are added for a better UX.**

---

## Roadmap

```text
v0.0.x  → Project foundations
v0.1.x  → Filesystem
v0.2.x  → Navigation
v0.3.x  → File information
v0.4.x  → File operations
v0.5.x  → Terminal UI
v0.6.x  → Selection & UX
v0.7.x  → Search & organization
v0.8.x  → Advanced features
v0.9.x  → Stabilization
v1.0.0  → Refactor & Engine 1.0

                    ↓

v2.0.0  → Qt/QML + Custom UI Engine
```

The `0.x` releases are expected to evolve organically while the project is being developed.
The `0.9.x` stage will represent a **functional but not yet fully refactored implementation**.
Version `1.0.0` will focus on restructuring, cleaning up, documenting, testing, and stabilizing the codebase before moving toward the Qt/QML architecture.

---

## Documentation

Project documentation will be progressively added as Annaora develops.

- [Changelog](CHANGELOG.md)
- Configuration language documentation — *coming in Engine 2.0*

---

## HOW TO USE ?

Its pretty easy as of now, arrow keys down and up to change items in the list and enter to enter a folder. Only folders, you can't see inside of a file as of now.

SINCE v0.3.0 :
- Press 'o' to open the informations menu of a FILE, only for files as of now.
- Press 'q' to close the informations menu.

---

## Building

> Build instructions will be added once the first functional version is available.

1. But for now clone the repo and follow the steps :
```bash
git clone https://github.com/Chaapoooo/Annaora.git
```
2. Then enter the directory :
```bash
cd Annaora
```
3. Launch Annaora :
```bash
./main
```

( Do ctrl + C to exit )

---

## Contributing

Annaora is currently a personal learning project and is under active development.
Contributions, ideas, bug reports and discussions are welcome, but the architecture may change significantly before version `1.0.0`.

---

## License

Annaora is released under the **MIT License**.

See [`LICENSE`](LICENSE) for the full license text.

---

## ⭐ About the project

Annaora is built from scratch with the goal of learning by actually building something usefull for daily usage.
Annaora is being built by a single CS student being 18, please respect my work and feel free to support it with a star or by sharing your ideas and feedbacks!