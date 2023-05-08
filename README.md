# PBO Ninja

This project is an open-source software application designed to enable manipulation of PBO/EBOs in DayZ. 

## Features

- PBO Hider
- Runtime EBO Decryptor

## Installation

### Requirements

- [Microsoft Detours](https://github.com/microsoft/Detours)

### Installation Steps

1. Install [vcpkg](https://vcpkg.io/en/) package manager by following its [installation instructions](https://vcpkg.io/en/getting-started.html).
2. Install detours:x64-windows-static package by running the following command in your terminal or command prompt:
`vcpkg install detours:x64-windows-static`

## Usage

##### PBO Ninja is not meant to be used against a BattlEye secured game. Please run DayZ without BattlEye or incorporate PBO Ninja into your own project.
* Inject `PBO-Ninja.dll` into DayZ_x64.exe using an injector such as [Xenos](https://github.com/DarthTon/Xenos).


## License

This project is licensed under the terms of the Mozilla Public License 2.0.

The Mozilla Public License 2.0 is a permissive open-source software license that allows you to use, modify, and distribute the software, as long as you include the original license and copyright notice in any copies or derivative works. The license also includes some additional terms and conditions, such as the requirement to disclose source code changes, and the prohibition of using the software for certain types of purposes, such as creating nuclear weapons.

For more information about the Mozilla Public License 2.0, please see the full license text [here](https://www.mozilla.org/en-US/MPL/2.0/).