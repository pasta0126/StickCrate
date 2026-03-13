# StickCrate

StickCrate es una plataforma modular de utilidades para **M5StickC Plus2**.

El proyecto convierte el dispositivo en una **caja de herramientas portátil para desarrolladores y entusiastas tech**, permitiendo ejecutar utilidades directamente desde la interfaz del equipo y, de forma opcional, conectarse a una **app compañera en Android** para acceder a servicios de internet y APIs.

StickCrate se centra en simplicidad, modularidad y experimentación.

## Concepto

El M5Stick actúa como una **interfaz de comando portátil**, mientras que la conectividad externa (internet, APIs, servicios) puede resolverse mediante una app móvil compañera.

Esto permite crear herramientas que combinen:

- interacción en dispositivo
- fuentes de datos remotas
- consultas a APIs
- utilidades personalizadas

## Características

- Interfaz simple en el dispositivo
- Arquitectura modular de herramientas
- Soporte para múltiples utilidades
- Capacidades de consulta a APIs
- Diseñado para experimentación y herramientas personalizadas
- Firmware liviano

## Hardware

StickCrate corre sobre:

### Plataforma objetivo

M5StickC Plus2

Características clave del dispositivo:

- Microcontrolador ESP32
- Conectividad Wi-Fi
- Pantalla TFT de 1.14" (135×240)
- Botones para navegación
- Batería integrada
- IMU, buzzer y sensores

Estas características lo hacen ideal para construir utilidades portátiles pequeñas y herramientas IoT.

## Arquitectura

StickCrate se compone de tres partes principales:

### Firmware del dispositivo

Corre en el M5StickC Plus2.

Responsabilidades:

- interfaz de pantalla
- ejecución de herramientas locales
- comunicación con dispositivos externos

### App compañera Android

Actúa como puente de conectividad.

Responsabilidades:

- acceso a internet
- comunicación con APIs
- procesamiento remoto de datos

### APIs externas

Se puede integrar cualquier servicio REST, por ejemplo:

- servicios de clima
- utilidades de red
- proveedores de datos
- endpoints personalizados

## Casos de uso

StickCrate puede usarse para construir:

- herramientas de consulta de APIs
- controladores IoT
- utilidades de red
- paneles personales
- herramientas experimentales para desarrolladores

## Objetivos del proyecto

- Construir una **consola portátil de utilidades tech**
- Facilitar la experimentación con APIs
- Ofrecer una estructura modular para agregar herramientas
- Mantener el firmware liviano y hackeable

## Desarrollo

El firmware está pensado para desarrollarse con herramientas típicas de ESP32 como:

- Arduino
- PlatformIO
- ESP-IDF

## Hoja de ruta

Mejoras previstas:

- sistema de plugins modular
- capa de comunicación con Android
- más utilidades integradas
- conectores para APIs
- navegación de UI mejorada

## Contribuciones

StickCrate está diseñado para ser **hackeable y extensible**.

Ideas, experimentos y contribuciones son bienvenidos.

## Licencia

Este repositorio usa **The Unlicense** (dominio público).

Consulta [LICENSE](LICENSE) para el texto completo.
