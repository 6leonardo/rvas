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


![interface](https://github.com/6leonardo/rvas/blob/master/images/interface.png?raw=true)


### esp8266 for load cells

Scale to measure the weight of the gas cylinders

Costruiamo due bilance per pesare le bombole le due bombole del gas...

Si puo comprare le celle di carico oppure due bilancie di basso costo e riciclare la meccanica e le celle di carico delle due bilancie

Per intefacciarsi al sistema facciamo leggere i sensori da un ESP8266 tramite HX711 e usiamo la relativa libreria per interfacciarsi...

Il collegamento dei sensori di carico è visibile nei seguenti due schemi:

![loadcell-1](https://github.com/6leonardo/rvas/blob/master/images/loadCell-1.jpg?raw=true)

![loadcell-2](https://github.com/6leonardo/rvas/blob/master/images/loadCell-2.jpg?raw=true)





