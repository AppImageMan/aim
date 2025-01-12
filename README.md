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
- `aim list`
   + Lists installed packages
- `aim run PACKAGE`
   + Launch a package
- `aim available`
   + List all available packages
- `aim backup`
   + Archive all of your packages and your manifest and store them in `~/.aim-backup.tar.gz`
- `aim restore`
   + Replace your `~/Applications` folder with the files in the `~/.aim-backup.tar.gz`
- `aim help`
   + Show help info.

The following OPTIONS are allowed before some commands:

- `--ask/-a` will make the application ask you if you want to continue.
- `--backup/-b` will run `aim backup` before the command
- `--quiet/-q`
