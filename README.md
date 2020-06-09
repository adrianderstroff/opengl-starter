# OpenGL Starter

This is a minimal and simple OpenGL Starter project that provides you with the necessary boilderplate code to get you up and running. This project uses modern CMake and [CPM](https://github.com/TheLartians/CPM.cmake) to download and configure all dependencies for you. The project uses GLFW3 for window generation, GLAD for retrieving the OpenGL functions supported by your system and GLM for vector maths. Follow the **prerequisits** and **build instructions** to set up your project. You then should be able to run the Hello World OpenGL project. From there you can completely change the source files to your liking.

IDEs that support CMake should automatically detect when you add or remove files in the *src* or *assets* folder, however not all IDEs support this feature. In that case simply rerun CMake.

## 1 Prerequisits

The OpenGL starter requires CMake 3.14+ and Python to be installed. The following sections describe how to install both dependencies on your target OS. 

### 1.1 Windows

A fresh installation of Windows should't contain either of the two dependencies, thus you can download them from their respective websites.

#### 1.1.1 Install Python
Navigate to [python.org](python.org) and download and install the latest version.

#### 1.1.2 Install CMake
Navigate to [https://cmake.org/download/](https://cmake.org/download/) and pick the installer for your system and run it to install CMake.

### 1.2 Linux

Under most Linux distros a version of Python should already be installed. If an older version of CMake has been already installed make sure to update to a version greater or equal to 3.14.

#### 1.2.1 Install Python
You can first check if Python is installed by running

```bash
python --version
```

In case python is not recognized, you should first update your package manager. In case of Ubuntu you can simply run
```bash
sudo apt-get update
```

Then you should be able to install python via
```bash
sudo apt-get install python3.8
```

or any other version of python by replacing the versioning number through the version of your choice.
When checking the version again it should now correctly display the version of the newly installed python.

#### 1.2.2 Install CMake
First you have to make sure that you have installed a CMake version of 3.14 or higher. If not you can perform the following instructions (taken from [here](https://askubuntu.com/questions/355565/how-do-i-install-the-latest-version-of-cmake-from-the-command-line)):

i) remove the current version of CMake
```bash
sudo apt purge --auto-remove cmake
```

ii) download the new version of CMake and extract it into a temporary directory
```bash
version=3.16
build=5
mkdir ~/temp
cd ~/temp
wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz
tar -xzvf cmake-$version.$build.tar.gz
cd cmake-$version.$build/
```

iii) build and install CMake
```bash
./bootstrap
make -j$(nproc)
sudo make install
```

iv) check if the installation was successful
```bash
cmake --version
```

Make sure the change *version* and *build* to fit your needs, the rest of the instructions should stay the same.

#### 1.2.3 Install all dependencies for GLFW

As for Ubuntu 18.04 the following dependencies had to be installed for GLFW to work. Those dependencies might differ depending on your Linux distribution.

```bash
sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### 1.3 Mac OS

On Mac OS X Python should already be installed. If an older version of CMake has been already installed make sure to update to a version greater or equal to 3.14.

#### 1.3.1 Install Python

Newer versions of Mac OS X should come with Python 2.7 already preinstalled. If not, then you can follow [this](https://docs.python-guide.org/starting/install3/osx/) article to setup Python 3.

#### 1.3.2 Install CMake

CMake can be installed via downloading the *.dmg* file from their [website](https://cmake.org/download/). After installing CMake, it needs to be added to the path in order to call it from the command line tool. As detailed in [this](https://tudat.tudelft.nl/installation/setupDevMacOs.html) article, the following two commands can be run to do the job

```bash
sudo mkdir -p /usr/local/bin
sudo /Applications/CMake.app/Contents/bin/cmake-gui --install=/usr/local/bin
```

Alternatively if you have Homebrew already installed on your system, you can simply run

```bash
brew install cmake
```

## 2 Build the project

CMake usually comes with a GUI and a terminal application. Using the GUI might be easier for beginners. In both cases create a build folder inside the opengl-starter directory or any directory of your choice. 

Then run CMake. If you choose the GUI application, you are then prompted to select a source and a build directory. For the source directory pick the opengl-starter directory, for the build directory pick the directory of your created build folder. After picking both folders press the *Configure* button. If you configure the project for the first time you are prompted to pick your compiler from a list of compilers, make sure to pick your compiler of choice and press OK. Then CMake will execute the CMakeLists script. This may take a while since it has to download GLAD, GLFW and GLM from the internet. After it executed the script the list of configurations will show all exposed CMake options in red. Now you can change any options to your liking, tho all options should be configured in a way that should work. Next press the *Configure* button again until no field is red. Then press *Generate*. 

In case you choose to use the CMake terminal application you can simply navigate to your build folder and then run `cmake /path/to/opengl-starter` to configure and generate the project.

After the configuration is done, depending on your build system you might be able to press the *Open Project* button in the CMake GUI application which will launch the IDE of your choice. Alternatively navigate the build folder and there you should find a target (e.g. *.sln* for Windows) that you can execute. Under linux you can navigate to the build folder and then execute `make -jX`, where X is the number of your logical cores (e.g. `make -j8`), to create an executable of your project.