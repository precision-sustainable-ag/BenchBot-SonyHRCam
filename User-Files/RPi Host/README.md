HAS NOT BEEN TESTED ON BENCHBOT
ONLY BEEN TESTED ON PI 

These changes will allow you to run the script from the Raspberry Pi 4 directly instead o having to use a laptop. You'll still need a monitor, mouse, and keyboard. 

1. Confirm Raspberry pi OS
   1. must have 64-bit OS (tested on bulleyes)
   - `uname -m`
   - This must return "aarch64"
   - If not, install raspberry os image for 64-bit
     - Use the raspberry pi imager
2. Install miniforge 
   1. download arm64 conda env manager by clicking on this [link](https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-aarch64.sh)
   2. go to download location (probably `Downloads` folder) and run the script
    - `cd ~/Downloads`
    - `bash Miniforge3-Linux-aarch64.sh`
3. Create conda env using python 3.9
   - `conda create -n bbot python=3.9`
4. Install [Qt in raspberry pi](https://qengineering.eu/install-qt5-with-opencv-on-raspberry-pi-4.html)
   - `sudo apt-get install qtbase5-dev qtchooser`
   - `sudo apt-get install qt5-qmake qtbase5-dev-tools`
   - `sudo apt-get install qtcreator`
   - `sudo apt-get install qtdeclarative5-dev`
5. Install pyqt5 in system
   - `sudo apt-get install python3-pyqt5`
6. install pyqt5 in conda env using [miniforge](https://anaconda.org/conda-forge/pyqt) (conda install ...) not  `pip`
   - `conda install -c conda-forge pyqt`
7. follow instructions from google doc (using pip)
8.  