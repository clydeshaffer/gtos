# GameTank OS

At first this was just a demo for my mouse and keyboard adapter for the GameTank and it only repeated text entered, or simulated icons draggable on a desktop.
Then I started adding acutal OS features like multitasking, allocation, event handling, etc. So now I'm just gonna lean into it and add more stuff like loading external binaries and FAT16 support and blah blah blah...

Assumes that a particular PS/2 adapter is attached which is not solidified at time of writing nor is it implemented in the emulator yet, so you'll need real hardware


## Build requirements:

EXPERIMENTAL: You can hopefully install cc65 and Zopfli into the SDK directory with 'make prereqs'. Use your package manager if you need to install nodeJS

* cc65 https://cc65.github.io/

  * recommended to use a snapshot build or build from source as their last tagged release and apt package are outdated

* NodeJS https://nodejs.org/

* Zopfli https://github.com/google/zopfli

* GNU Make


## Testing requirements:

* GameTank Emulator https://github.com/clydeshaffer/GameTankEmulator
