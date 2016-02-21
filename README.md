Passerelle
==========

Passerelle, formerly known as "Saitek Flight Panels for Lua", is a Lua module to
control your gaming devices. This module has been design to allow the Saitek [Flight
Instrument Panel](http://www.saitek.com/uk/prod/fip.html) (FIP) device and [X52 Pro](http://www.saitek.com/uk/prod/x52pro.html)
controller to be controlled from Lua.

Back in 2011, the motivation was to allow Flight Simulator 2004 users to use the
FIP with the simulator using Pete Dowson's [FSUIPC](http://www.schiratti.com/dowson.html) and its Lua scripting
capabilities. This module is now using in a wide range of flight simulators like
the DCS series.


A Lua module to control your gaming devices
-------------------------------------------

This module is based on Lua 5.1.4 and is known to work with:

- Official [Lua](http://www.lua.org) distribution
- Microsoft Flight Simulator 2004 using FSUIPC
- Microsoft Flight Simulator X using FSUIPC
- Microsoft [Flight Simulator X Steam Edition](http://store.steampowered.com/app/314160/) using FSUIPC
- Lockheed Martin's [Prepa3D](http://www.prepar3d.com) using FPUIPC
- Eagle Dynamics [DCS series](http://www.digitalcombatsimulator.com/en/index.php)

Any game implementing a Lua engine like Blizzard's [World of Warcraft](http://eu.battle.net/wow/en/)
should work too.


Building
--------

You can compile this module using Microsoft [Visual Studio Community 2005](https://www.microsoft.com/en-US/download/details.aspx?id=48146).
This version is totally free and full-featured. You'll also need to install the
[DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
dependency to support older DirectX version 9 and 10. You must uninstall all
versions of the Visual C++ 2010 Redistributable before installing the DirectX SDK
to prevent error S1023.

You can use the following commands:

```
C:\> MsiExec.exe /passive /X{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}

C:\> MsiExec.exe /passive /X{1D8E6291-B0D5-35EC-8441-6616F567A0F7}
```

After installation you can resinstall the latest version of the 2010
redistributable package for [x86](https://www.microsoft.com/en-US/download/details.aspx?id=8328)
and/or [x64](https://www.microsoft.com/en-US/download/details.aspx?id=13523).


Known Issues
------------

Prior to FSUIPC 3.997e/4.715a, FSUIPC doesn't unload Lua scripts and script
slot may be full executing and re-executing scripts for a single flight.


Changes
-------

#### 20120914 - v0.5.1

- ADDED: 64-bit version

#### 20110920 - v0.5

- FIXED: There were a problem in the reference counter calling Release(). The value could be lower than 0
- FIXED: Release() didn't work as expected and didn't really take care of the reference counter
- FIXED: SetString(...) was missing in previous versions!
- FIXED: Some random crashes was occurring running Flight Simulator
- CHANGED: Initialize() is now obsolete, kept for compatibility
- CHANGED: Release() is now obsolete, kept for compatibility
- ADDED: GetVersion() function
- ADDED: Support for multiple script execution at a time (supposed to be thread safe)

#### 20110624 - v0.2

- FIXED: module should be more stable now
- FIXED: better handling of device indices when unplugged/replugged
- CHANGED: Indices now start at 1 to comply with Lua table functioning
- ADDED: RegisterSoftButtonDownCallback(...) function
- ADDED: RegisterSoftButtonUpCallback(...) function
- ADDED: RegisterDeviceChangeCallback(...) function
- ADDED: SetProfile(...) function

#### 20110526 - Initial Release
