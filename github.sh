#/bin/bash
export PATH=$PATH
echo $PATH
sudo git add .
sudo git commit -m `date +%Y-%m-%d`
sudo git push https://github.com/ffteng46/hight_hft.git
