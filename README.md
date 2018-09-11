ttt
---

An ncurses based tic tac toe with multi-player and single-player AI mode

### Build

- You need to have the version of ncurses on your system.
  On Ubuntu, for example, you should run the following to install the library:
  ```bash
  sudo apt-get install libncurses-dev
  ```
- Run `make` to build and run the binary


### Modes

You can pass two command-line parameters to the binary:
- the first parameter is the size of the grid, for example, you can pass 4 to
  play in a 4x4 sized grid: `./ttt 4`. Defaults to 3
- the second parameter is the game mode; 0 for two-player mode, 1 for AI
  mode.  Defaults to 0

### License

See [LICENSE](https://github.com/aonemd/ttt/blob/master/LICENSE).