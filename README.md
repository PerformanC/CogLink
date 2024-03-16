# CogLink

A Concord (C99) LavaLink/NodeLink client.

## Features

- C99 compatible
- Low memory usage (> 5MB of RAM a working bot)
- Portable

## Dependencies

- [`Concord`](https://github.com/Cogmasters/Concord) >= 2.2.0

> [!NOTE]
> CogLink only relies on dependencies already included in Concord.

## Installation

### 1. Install Concord

The installation guide for Concord can be found [in their repository](https://github.com/Cogmasters/concord?tab=readme-ov-file#build-instructions).

### 2. Clone the repository

```shell
$ git clone https://github.com/PerformanC/CogLink
```

### 3. Compile

```shell
$ cd CogLink
$ make -j4
```

> [!NOTE]
> The `-j4` flag is optional, but it will speed up the compilation process. The number after `-j` is the number of threads to use for compilation.

### 4. Install

```shell
$ sudo make install
```

## Usage

... TODO

## Documentation

[CogLink documentation](https://performanc.github.io/CoglinDocs/) is made with Doxygen, hosted on GitHub Pages.

The documentation can be built with the following command:

```shell
$ make gen_docs
```

> [!NOTE]
> Doxygen is required to build the documentation.

## Support

Any question or issue related to CogLink or other PerformanC projects can be can be made in [PerformanC's Discord server](https://discord.gg/uPveNfTuCJ).

For verified issues, please also create a GitHub issue for tracking the issue.

## Contributing

CogLink follows the PerformanC's [contribution guidelines](https://github.com/PerformanC/contributing). It is necessary to follow the guidelines to contribute to CogLink and other PerformanC projects.

## Projects using CogLink

None known yet, but if you are using CogLink, please let us know in [PerformanC's Discord server](https://discord.gg/uPveNfTuCJ) and we will add your project here.

## License

CogLink is licensed under [BSD 2-Clause License](LICENSE). You can read more about it on [Open Source Initiative](https://opensource.org/licenses/BSD-2-Clause).

* This project uses the latest standards of The PerformanC Organization.