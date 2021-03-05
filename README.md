# rvas

RV automation

A project for motorhomes automation

at the moment is only a skeleton


## hardware

raspberry pi zero

esp8266 for sensors

## Install

### raspberry pi 

install mariadb-server
install nodejs

create db with the script in init_db

then execute

sudo node index.js 80 2

you can remove the use of sudo with the following commands

```bash
sudo usermod -a -G gpio
sudo apt-get install libcap2-bin
sudo setcap cap_net_bind_service=+ep `readlink -f \`which node\``
```

to see the outputs 

```bash
watch -n 0.2 "gpio readall"
```


### esp8266

compile and upload the sample

The sample send some sensor measures to the raspberry after that go to deep sleep for 30 seconds... then it wake up and send again

The Esp8266 in deep sleep use minimum power possible

In deep sleep to wake up the esp8266 you must connect **RST** pin to **D0** 


![interface](https://github.com/6leonardo/rvas/blob/master/interface.png?raw=true)


