# rvas

RV automation

A project for motorhomes automation

at the moment is only a skeleton


## hardware

raspberry pi zero

esp8266 for sensors

## Install

### raspberry pi 

install mariadb-server -- at the moment with mqtt version doesn't need
install nodejs
install mosquitto -- for mqtt version

create db with the script in init_db

set mosquitto user:pi and password: demodemo

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

per il momento c'e' solo il sensore bilancia..

## Bilancia

compila il sorgente in sensors/scale (scale_http per la versione http)

nel file config_settings inserisci wifi ssid e password del raspberry e il nome host raspberry, inoltre sono presenti i settaggi per quanto non si collega al raspberry e cra un hostspot quindi controllare AP_SID e AP_PWD

l'esp si accende se trova il raspberry manda i pesi e poi va in deepsleep e si risveglia dopo due minuti

Per accenderlo e fare in modo che avvi l'hotspot premere il pulsante collegato come nello schema seguente

Se si rimane collegati l'esp rimane acceso mentre se ci si scollega dal web l'esp dopo 5 minuti torna in deepsleep

Nel caso si voglia controllare l'esp tramite interfaccia web quando in wifi pubblicare su mqtt con il seguente comando

```bash
mosquitto_pub -u pi -P demodemo -t "gas/wakeup" -m "on" -r
mosquitto_sub -u pi -P demodemo -t "slave/url/open"
```

Al sucessivox risveglio l'esp mandera un messaggio mqtt (visibile tramite il secondo comando) che indica l'IP dove collegarsi.

I collegamente da fare oltre quelli dello schema in calce sono, 

HX711 vcc gnd DT(ESP GPIO4) CLK (ESP GPIO5) per una bilancia... i pin sono definiti in pin.h

Per il deepsleep è necessario che il pin **RST** sia collegato al **D0** come si vede nello schema.


![interface](https://github.com/6leonardo/rvas/blob/master/images/interface.png?raw=true)


![loadcell-1](https://github.com/6leonardo/rvas/blob/master/images/loadCell-1.jpg?raw=true)

![loadcell-2](https://github.com/6leonardo/rvas/blob/master/images/loadCell-2.jpg?raw=true)


## Collegamento pulsante e reset per Deep Sleep

![loadcell-3](https://github.com/6leonardo/rvas/blob/master/images/pulsante.png?raw=true)


## Consumi energetici

Nella cartella sensors/ammeter si trova il sorgente per utilizzare un esp8266 per leggere i consumi energetici di 5 sorgenti di corrente.

Al fine di poter leggere i consumi energetici di corrente e potenza e anche le tensioni dei 5 generatori (per me tre batterie, un pannello solare e la corrente dell'alternatore) ho utilizzato 5 INA219 i quali possono avere solo 4 indirizzi i2c quindi ho untilizzato un muliplexer i2c, TCA9548A che permette di collegare fino a 8 periferiche i2c allo stesso bus.

I collegamenti sono molto semplici, basta collegare ESP8266 al TCA9548A e quindi tutti gli INA219 alle uscite del TCA9548A e le relative alimentazioni.

Importante è sapeere che gli INA219 hanno gia una resistenza tra V* e V- di 0.1Ω e che quindi se si utilizzano gli shunt occorre dissaldare dalle schedine INA219 la resitenza di 0.1Ω e collegare V+ e V- ai due capi dello shunt e per leggere la tensione di alimentazione e calcolare correttamente la potenza occorre mettere lo shunt sul positivo.

Nell cartella è presente anche la libreria Adafruit che ho modificato per permettere di configure gli INA219 con i corretti valori di shunt, e correnti.

Il calcolo delle resistenza di shunt si vede dalle specifiche, ad esempio una shunt di 80mV per 150A ha una resistenza di 0.08/150=0.0005333Ω



