# open-mpc release tools (vendored)

`release.py`, `catalog_check.py`, `sdk_zip.py` and what they import, from
[open-mpc/mpc-vst-plugins](https://github.com/open-mpc/mpc-vst-plugins) `tools/` at commit
`bcc8242896f6948b2405014e8443c91941c4ded0` (current `main`, 2026-11-22), MIT (see LICENSE). They make PlateauXXL's release
zips as the catalog reads them: `-mpc-armv7.zip` (Gen1 and Force), `-mpc-aarch64.zip` (Gen2) and `-mpc-sdk.zip`
(Akai's one-folder format, MPC OS 3.10.1).

One change, in `release.py` and `catalog_check.py` (made with `tools/release_quotes.py`): the plugin's name and skin folder
are shell-escaped when they go into `install.sh` and `uninstall.sh`, because `L'Cronx` has an apostrophe (without it
`sh -n install.sh` fails). `.github/workflows/build.yml` runs `sh -n` on both scripts of every zip.
