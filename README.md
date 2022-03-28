Wrapper de red para los encoders

uso:

enco_to_net -d /dev/encoder_device -n multicast_group -p multicas_port -i packet_id

-d: dispositivo a usar (por ej: /dev/ttyUSB0)
-n: dirección multicast destino (por ej: 224.0.0.69)
-p: puerto multicast destino (por ej: 10002)
-i: identificador de paquete (por ej: 2)

Para setear correctamente el identificador de paquete es necesario ver la tabla del diccionario de identificadores para cada antena.



operador@ctrl-ant-x:~/EncodersIAR/src$ make
perador@ctrl-ant-x:~/ControlMotoresIAR/src$ make

sudo ln -s /home/operador/ControlMotoresIAR/bin/engines_server /usr/local/bin/engines_server
sudo ln -s /home/operador/EncodersIAR/bin/enco_to_net /usr/local/bin/enco_to_net

sudo ln -s /home/operador/EncodersIAR/service/encodersA2.service /etc/systemd/system/encodersA2.service
sudo ln -s /home/operador/ControlMotoresIAR/service/engine_serverA2.service /etc/systemd/system/engine_serverA2.service

ln -s /dev/ttyUSB1 /dev/ctrlmotores
ln -s /dev/ttyUSB0 /dev/encoders

systemctl stop engine_serverA2.service
systemctl stop encodersA2.service
sleep 2s
python ~/EncodersIAR/tlmy_enco.py
python ~/ControlMotores/telmy_control_Ax.py
sleep 2s
systemctl start engine_serverA2.service
systemctl start encodersA2.service
sleep 2s

systemctl status engine_serverA2.service
systemctl status encodersA2.service

systemctl stop engine_serverA2.service
systemctl stop encodersA2.service
