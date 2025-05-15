<h1 align="center">Kagu Engine</h1>

<p align="center"><i>3D Game Engine using Vulkan</i></p>

<p align="center">
  <img src="Images/KaguEngine_Logo.png" alt="Kagu Engine logo" width=400 height=400/>
</p>

<h2 align="center">Clone the repository</h2>

First of all, move to the directory you would like to clone the repository to :
```bash
cd path/to/repo
```

Then, clone the repository :
```bash
git clone --recursive https://github.com/dark-dylan-93220/KaguEngine
cd KaguEngine
```
Don't forget the `--recursive` flag as it adds the stb submodule from [nothings/stb](https://github.com/nothings/stb), used to treat textures inside KaguEngine.

To update the repository to the latest commit :
 - First make sure that you are in the `KaguEngine/` directory.
 - Then, run :
```bash
git checkout master
git pull
```

<h2 align="center">Project dependencies</h2>

In order for the engine to work properly, you must have a recent version of the Vulkan SDK installed on your system.

<i>See more at https://www.lunarg.com/vulkan-sdk/</i>

And you must also have CMake installed, with a minimum version being 3.12.

<i>See more at https://cmake.org/download/</i>

<h2 align="center">Building the application</h2>

First, move to the directory where the repository is located at :
```bash
cd path/to/repo
```
Then, make sure that, depending on your OS, you have all the dependencies installed :
 - Linux (using apt) :
```bash
sudo apt-get update
sudo apt-get install -y libxkbcommon-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```
 - Linux (using pacman) :
```bash
sudo pacman -Syu
sudo pacman -S --noconfirm libxkbcommon xorg-xinerama xorg-xcursor libxi mesa
```
Once this is all done, you can use CMake to build the project :
```bash
cmake -B path/to/build
cmake --build path/to/build
```

<h2 align="center">Contributing</h2>

<p align="center"><i>To add later on...</i></p>

<h2 align="center">License</h2>

KaguEngine uses the MIT license meaning that you can do whatever you want as long as you include the original copyright and license notice in any copy of the software / source.

See [LICENSE.txt](LICENSE.txt) for more information.