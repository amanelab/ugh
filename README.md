Ugh - The screen saver stopper
==============================

Overview
--------

In your daily computing life, you might be annoyed by the situation
when your system enters screen saver mode without your intention. To
avoid this, you can move your mouse slightly or tap inactive keys on
your keyboard at certain intervals. But these are also irritating ...

This program will do that for you automatically so that your system
will not go into screen saver mode. This program will keep you out of
screen saver mode by internally pressing the Ctrl key at a certain
interval.

Install
-------

If you get this module as a ZIP archive just unzip it. You can place
this package wherever you want.

If you get it as source code, make it and put the package where you
want.

Usage
-----

### Start

  1. Double-click the ugh.exe.
  2. Set the interval at which this program press the Ctrl key.
  3. Click the "Turn ON" button.

You can run this program as a background process by clicking the Close
button. And you can operate this program through the system tray icon
in the notification area.

### Stop

To stop pressing the key, right-click on the system tray icon and
select the "Turn OFF" menu item. This menu item will change to "Turn
ON". You can start pressing again by selecting this menu item.

You can also stop pressing the key by clicking the "Turn OFF" button
in the control window.

### Terminate

To terminate the process, right-click the system tray icon and select
the "Exit" menu item.

Uninstall
---------

### Remove files

Just trash the package directory which you placed on the installation.

### Cleaning the registory cache

This program adds its icon to the notification area. On Windows 10/11,
the Notify icon information is cached in the Windows registry. You can
view the cached content in the Taskbar Settings panel.

If you run this program located 2 or more times on the different
paths, these cache will be registered twice or more. Or you can view
the icon even if you uninstall this program. This is not the function
of this program, but the Windows OS specification.

If you want to remove the Notify icon cache, try the following:

  1. Run regedit.exe
  2. Navigate to the following key:

      HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\ -
        Microsoft\Windows\CurrentVersion\TrayNotify

  3. Remove the following keys:

      - IconStreams
      - PastIconsStream

  4. Restart explorer.exe using Task Manager.

Caution
-------

  - If your system is set to enter screen saver mode for security
    reasons, this program may pose a security risk. Please use it at
    your own risk.

  - This program presses the Ctrl key internally and doesn't care
    about what other keys the user presses. So, if you type on the
    keyboard while this program is running, there is a small
    possibility of unintended input.

Build
-----

If you want to build it yourself, you can do so by following these
steps:

  1. Go to the top of the source directory.
  2. Run make(1)

     make package

  3. You can get the package in the .build directory.

To build this program you will need MinGW64 or a similar toolchain.


Have fun!
