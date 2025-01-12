# AppImage Manager

## Description

A tool for installing, updating, and removing AppImages.

Repository source can be found at [AIL](https://github.com/AppImageMan/ail.git)

## Usage

The AppImage Manager is a command line tool which accepts the following commands:

- `aim [OPTIONS] install PACKAGE [DATEVERS]`
   + Install a package by name
   + Optionally, install a specific package version (based on the date.txt file in the [AIL](https://github.com/AppImageMan/ail.git))
- `aim [OPTIONS] remove PACKAGE`
   + Removes a package from your system
- `aim [OPTIONS] upgrade`
   + Attempts to upgrade your AppImages
- `aim [OPTIONS] list`
   + Lists installed packages
- `aim [OPTIONS] run PACKAGE`
   + Launch a package
- `aim [OPTIONS] available`
   + List all available packages
- `aim [OPTIONS] backup`
   + Archive all of your packages and your manifest and store them in `~/.aim-backup.tar.gz`
- `aim [OPTIONS] restore`
   + Replace your `~/Applications` folder with the files in the `~/.aim-backup.tar.gz`

The following OPTIONS are allowed before a command (thought they don't always do anything):

- `--ask/-a` will make the application ask you if you want to continue.
- `--backup/-b` will run `aim backup` before the command
- `--quiet/-q` will hide all stdout/stderr
- `--help/-h` will show usage information

## Build

Dependencies:

- curl
- git
- g++
- GNU Make

Run `make RELEASE=1 -j$(nproc)`
