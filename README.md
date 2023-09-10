# Installation instructions

```bash
sudo apt update
sudo apt install ccache libopencv-dev python3-opencv cmake git libx11-dev libxfixes-dev libgl-dev libxext-dev
git clone https://github.com/ultravideo/uvgRTP
cd uvgRTP
mkdir build && cd build
cmake -DUVGRTP_DISABLE_CRYPTO=1 ..
make
sudo make install
cd <project-dir>
qmake
make
```
