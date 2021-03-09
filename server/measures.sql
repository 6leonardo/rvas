truncate table measures;

insert into measures(name,type) values('water/1/level',2);
insert into measures(name,type) values('water/2/level',2);
insert into measures(name,type) values('water/valve',1);
insert into measures(name,type) values('water/pump',1);
insert into measures(name,type) values('water/1/temp',2);

insert into measures(name,type) values('water/2/temp',2);
insert into measures(name,type) values('water/1/unload',1);
insert into measures(name,type) values('water/2/unload',1);
insert into measures(name,type) values('sensor/int/temp',2);
insert into measures(name,type) values('sensor/int/hum',2);
insert into measures(name,type) values('sensor/ext/temp',2);
insert into measures(name,type) values('sensor/ext/hum',2);

insert into measures(name,type) values('battery/1/volt',2);
insert into measures(name,type) values('battery/1/amp',2);
insert into measures(name,type) values('battery/1/back',2);
insert into measures(name,type) values('battery/2/volt',2);
insert into measures(name,type) values('battery/2/amp',2);
insert into measures(name,type) values('battery/3/volt',2);
insert into measures(name,type) values('battery/3/amp',2);

insert into measures(name,type) values('solar/volt',2);
insert into measures(name,type) values('solar/amp',2);
insert into measures(name,type) values('gas/1/level',2);
insert into measures(name,type) values('gas/2/level',2);

insert into measures(name,type) values('lights',1);

insert into measures(name,type) values('light/extern',1);
insert into measures(name,type) values('light/living',1);
insert into measures(name,type) values('light/kitchen',1);

insert into measures(name,type) values('generale',1);
insert into measures(name,type) values('inverter',1);

insert into measures(name,type) values('fridge/on',1);
insert into measures(name,type) values('heater/on',1);

insert into measures(name,type) values('airc/on',1);

insert into measures(name,type) values('fans/1',1);
insert into measures(name,type) values('fans/2',1);
insert into measures(name,type) values('fans/3',1);
insert into measures(name,type) values('fans/4',1);


update measures set v_int=1, update_time = CURRENT_TIMESTAMP() where type=1;
update measures set v_float=0, update_time = CURRENT_TIMESTAMP() where type=2;
update measures set v_string='', update_time = CURRENT_TIMESTAMP() where id=3;

