# TP1 - Sistemas Embebidos (ITBA)

Implementacion de un sistema de control de acceso para oficinas usando una placa FRDM-K64F, encoder rotativo, display de 7 segmentos y lector de tarjeta magnetica.

## Objetivo

El sistema debe validar acceso en dos pasos:

1. Ingreso de ID de usuario (8 digitos) por encoder o tarjeta magnetica.
2. Ingreso de PIN (4 o 5 digitos) por encoder.

Si ID y PIN son correctos, se habilita el pestillo electrico (emulado con un LED) durante 5 segundos.

## Requerimientos obligatorios

- Ingreso de ID por encoder o tarjeta magnetica.
- Ingreso de PIN solo por encoder.
- Correccion de errores de escritura y cancelacion del intento.
- Ajuste de intensidad del display con el encoder.
- Mostrar ID en display.
- Ocultar PIN en display (por ejemplo con guiones).
- Manejo de display de 7 segmentos y 3 LEDs usando 2 shift-registers 74HC595 en serie.
- Arquitectura por capas: aplicacion separada de drivers (hardware transparente para aplicacion).
- Uso eficiente de interrupciones (sin perdida de eventos y sin codigo bloqueante).
- Pin de test (TP) para medir tiempo de ISR.

## Requerimientos deseados

- Timeout por inactividad y regreso a estado inicial.
- Retardo por PIN incorrecto.
- Bloqueo de ID tras 3 intentos fallidos consecutivos.
- Interfaz mas intuitiva (animaciones, letras/numeros, feedback con LED RGB).
- Modo administrador para alta/baja de usuarios.
- Cambio de PIN por usuario.

## Arquitectura propuesta

Se recomienda organizar el firmware en capas:

- `app` (logica de negocio y FSM de acceso).
- `drivers` de bajo nivel (GPIO, interrupciones, timing, shift-register, encoder, lector).
- `board` (mapeo de pines y configuracion especifica de hardware).

### Estructura actual del repo

```
TP1_Embebidos/
|-- README.md
`-- TP1_code/
		|-- SDK/
		|   |-- CMSIS/
		|   `-- startup/
		`-- source/
				|-- App.c
				|-- board.h
				|-- gpio.c
				|-- gpio.h
				|-- pisr.c
				`-- pisr.h
```

## FSM sugerida (capa aplicacion)

Estados recomendados:

- `IDLE`: estado inicial, espera actividad.
- `INGRESO_ID`: carga de ID por encoder o tarjeta.
- `VALIDANDO_ID`: verifica existencia y estado del usuario.
- `INGRESO_PIN`: ingreso enmascarado de PIN.
- `VALIDANDO_PIN`: compara PIN y actualiza contador de intentos.
- `ACCESO_OK`: activa pestillo por 5 s.
- `ACCESO_DENEGADO`: feedback de error y reglas de seguridad.
- `BLOQUEADO`: bloqueo temporal o permanente segun politica.

Eventos de entrada tipicos:

- Giro encoder, click encoder, timeout, lectura de tarjeta, fin de temporizador, cancelacion.

## Uso de interrupciones (criterio)

- Capturar eventos en ISR con minima logica.
- Encolar o marcar flags atomicos para procesar en `main`.
- Evitar delays bloqueantes dentro de ISR.
- Activar pin TP al entrar/salir de ISR para medicion de carga temporal.

## Interfaz de usuario

- Display:
	- ID visible durante ingreso.
	- PIN oculto (guiones u otro simbolo fijo).
- LED de pestillo:
	- Encendido durante 5 s en acceso valido.
- LEDs de estado:
	- Feedback de operacion (espera, error, bloqueado, exito).

## Seguridad recomendada

- `timeout` de sesion por inactividad.
- Penalizacion progresiva por errores consecutivos.
- Bloqueo de ID tras 3 intentos fallidos.
- Limpieza de buffers al cancelar o tras timeout.

## Colaboracion entre VS Code y MCUXpresso IDE

Este repositorio esta preparado para trabajo en equipo en distintos IDEs.

- Versionar codigo fuente, headers y configuraciones necesarias del proyecto.
- No versionar artefactos generados por compilacion/debug (`Debug/`, `Release/`, objetos, logs, binarios de salida).
- Mantener cambios atomicos por feature (una rama por funcionalidad).

## Compilacion y prueba

La compilacion puede hacerse desde MCUXpresso o desde herramientas de la catedra, segun la configuracion del proyecto.

Checklist minimo de validacion:

1. Verificar ingreso de ID por encoder.
2. Verificar ingreso de ID por tarjeta magnetica.
3. Verificar ingreso de PIN oculto.
4. Verificar correccion/cancelacion.
5. Verificar activacion de pestillo por 5 s.
6. Verificar comportamiento frente a PIN incorrecto (incluyendo bloqueo, si aplica).
7. Medir tiempo de ISR con TP.

## Pendientes sugeridos

- Definir formato final de almacenamiento de usuarios (estructura en RAM/Flash).
- Completar driver de shift-register y multiplexado de display.
- Integrar parser de banda magnetica segun ISO/IEC 7811-2.
- Documentar diagrama de estados final y manual de usuario.