Wrapper de red para los encoders

uso:

enco_to_net -d /dev/encoder_device -n multicast_group -p multicas_port -i packet_id

-d: dispositivo a usar (por ej: /dev/ttyUSB0)
-n: dirección multicast destino (por ej: 224.0.0.69)
-p: puerto multicast destino (por ej: 10002)
-i: identificador de paquete (por ej: 2)

Para setear correctamente el identificador de paquete es necesario ver la tabla del diccionario de identificadores para cada antena.