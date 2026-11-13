# Appendix 10. Automatic execution of a command file

## Appendix 10. Automatic execution of a command file

### A10.1  Defining the command

When automatic writing of results to a file at set
intervals has been started in the program ([chapter 9.7](9.7_automaattinen_tiedostotulostus.md)), an
external command can be started every time the files have been written. This
mode of operation is started either on the form controlling automatic output or with the parameter

```
KOMENTO=suoritettava komento
```

The command to be executed can be almost any
command that requires no user action while running and that prints nothing
on the screen except through the "standard output" stream. This
output stream is automatically redirected out of sight. Even this restriction can be
removed by starting the program in a new window with the
START command.

In most cases it is advisable to collect the commands to be executed
in a command file (BAT or CMD file) and give the name of this
command file in the program parameter.

### A10.2  Automatic transfer of HTML files with rsync over SSH (recommended)

A modern and secure way to transfer automatically created
HTML results files to your own web server is `rsync` over an SSH connection.
`rsync` transfers only changed files, so the transfer is fast and saves
bandwidth, and the flag `--delay-updates` renames the individual files
from temporary names to their final names only at the end of the transfer. This way viewers
normally do not see a half-finished HTML file. SSH public
key authentication replaces the password, so the transfer is also suitable for automatic
operation.

These instructions replace the earlier `ftp` and `sftp2` examples, which are no longer
recommended for modern servers.

#### Prerequisites

- Windows 10/11 (22H2 or newer)
- WSL2 + Ubuntu (or another distribution), installed with `wsl --install` in PowerShell
  as administrator
- SSH access to the target server (e.g. `user@kisapalvelin.fi`) and
  write permission to its directory (e.g. `/var/www/html/kisa/`)

#### 1. Creating an SSH key pair and registering it on the server

Open a WSL terminal (command `wsl` from the command line). Create a key without
a passphrase so that the results software can run the transfer
automatically:

```bash
ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519_kisa
ssh-copy-id -i ~/.ssh/id_ed25519_kisa.pub user@kisapalvelin.fi
```

Test that logging in works without a password:

```bash
ssh -i ~/.ssh/id_ed25519_kisa user@kisapalvelin.fi echo ok
```

The first time you must accept the server's host key (`yes`).

#### 2. Trying the rsync command manually

The results software usually writes the HTML files to a dedicated directory such as
`C:\kisa\www\`. The same path appears in WSL as `/mnt/c/kisa/www/`.
First try it manually from the WSL terminal:

```bash
rsync -av --delete-after --delay-updates \
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa" \
  /mnt/c/kisa/www/ \
  user@kisapalvelin.fi:/var/www/html/kisa/
```

Meaning of the flags:

- `-a` = recursive archive mode (preserves timestamps)
- `-v` = shows the transferred files (for testing; leave out in the final script)
- `--delete-after` = deletes from the server files that are no longer
  in the source, only after the transfer has succeeded
- `--delay-updates` = transfers the files first under temporary names and renames them
  to their final names only at the end of the transfer

When the files appear on the server, SSH authentication and rsync are working.

#### 3. Windows command file siirto.cmd

Create the file that the results software will start. The command must not
print anything on the screen or ask anything (see A10.1). Therefore its output is redirected
to a log file. Save the script somewhere other than the published HTML directory,
e.g. as `C:\kisa\siirto\siirto.cmd`:

```cmd
@echo off
REM Transfer of results service HTML files to the server with rsync via WSL.
REM Called automatically from the KOMENTO parameter of HkKisaWin.

set LOG=C:\kisa\siirto\siirto.log

wsl -- rsync -a --delete-after --delay-updates ^
  -e "ssh -i /home/KÄYTTÄJÄ/.ssh/id_ed25519_kisa -o StrictHostKeyChecking=accept-new -o ConnectTimeout=10" ^
  /mnt/c/kisa/www/ ^
  user@kisapalvelin.fi:/var/www/html/kisa/ ^
  >> "%LOG%" 2>&1

exit /b %ERRORLEVEL%
```

Notes:

- `wsl -- komento` runs the command in the default WSL distribution without a new window.
- The `^` characters continue the line in a .cmd file.
- `ConnectTimeout=10` makes sure the command does not hang if the network
  is down.
- You can follow the transfer log from PowerShell with the command
  `Get-Content C:\kisa\siirto\siirto.log -Wait`.
- Keep `siirto.cmd` and `siirto.log` in a different directory from the published
  HTML files so that they are not mirrored to the server.

#### 4. Connecting to the results software

Add to the competition's `.cfg` file (e.g. `laskenta.cfg`):

```
HTML=c:\kisa\www\tulokset.htm/60/S
KOMENTO=c:\kisa\siirto\siirto.cmd
```

`HTML=...` writes the HTML files every 60 seconds class by class
(`/S`) and `KOMENTO=...` runs the `siirto.cmd` file every time after this.
The classes and split points to be output can be limited in more detail with the file
`AUTOFILE.LST` (see [chapter 9.7](9.7_automaattinen_tiedostotulostus.md)).

#### 5. Troubleshooting

| Symptom | Check |
|------|----------|
| Nothing is transferred | Run `siirto.cmd` manually from the command line and check `siirto.log` |
| `ssh: permission denied` | The public key is missing from the server's `~/.ssh/authorized_keys` file |
| `ssh: Host key verification failed` | Use `-o StrictHostKeyChecking=accept-new` and remove `~/.ssh/known_hosts` if necessary |
| `wsl: command not found` | WSL is not installed; run `wsl --install` in PowerShell |
| `rsync: command not found` | `sudo apt install rsync` in WSL (usually installed already) |
| The results software hangs | The `ConnectTimeout` flag is missing |
| The browser shows a half-finished page | Make sure `--delay-updates` is included |

#### 6. Alternative without WSL

If WSL is not available, you can use Windows' own
OpenSSH client (included in Windows 10/11) and a separately installed
rsync package (e.g. cwRsync or MSYS2). In this case `siirto.cmd`
calls `rsync.exe` directly, the paths are in Windows format
(`C:\kisa\html\`), and the SSH key is stored in the directory `%USERPROFILE%\.ssh\`.
WSL is nevertheless recommended, because rsync and ssh work there
natively without separate installations.

#### 7. Security notes

- Keep the server's SSH key only on the computer where it is needed.
  If the competition computer goes missing, remove the corresponding public key from the server's
  `authorized_keys` file.
- It is advisable to restrict the key's permissions in the `authorized_keys` file,
  e.g. with the definitions `command="rsync --server ..."` and
  `from="kisakone.ip"`; see `man sshd`, section *AUTHORIZED_KEYS FILE FORMAT*.
