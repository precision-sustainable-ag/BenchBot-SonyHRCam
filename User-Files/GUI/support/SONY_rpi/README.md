# RPi Remote Client

## Build requirements:

- Contents of this folder must remain together
- RPi 64 bit ARMv8 (tested on "bullseye")
- Built using the ARMv8 version of the [Sony SDK](https://support.d-imaging.sony.co.jp/app/sdk/en/index.html)

<br>

Check OS bit version (must return "64")
```
getconf LONG_BIT
```

<br>
Check RPi OS by running (this Remote CLi has only been tested on "bullseye"):

```
cat /etc/os-release
```