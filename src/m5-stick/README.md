# Deploy por terminal (M5StickC Plus2)

```bash
# 1) (Solo una vez) instalar PlatformIO CLI
python -m pip install -U platformio

# 2) Compilar
python -m platformio run -e m5stickc_plus2

# 3) Subir al dispositivo
python -m platformio run -e m5stickc_plus2 -t upload

# 4) Ver monitor serie
python -m platformio device monitor -b 115200
```

```bash
# (Opcional) limpiar build
python -m platformio run -e m5stickc_plus2 -t clean
```

## Tabla de comportamiento de botones

| Contexto                 | BtnB corto                 | BtnB largo              | BtnA corto                   | BtnA largo                     | PWR corto                | PWR largo          |
| :----------------------- | :------------------------- | :---------------------- | :--------------------------- | :----------------------------- | :----------------------- | :----------------- |
| Global                   | Adelante / siguiente       | Atrás / cancelar        | Selección / acción           | Acción especial según pantalla | Atrás / cancelar         | Apagar dispositivo |
| Menú (`VIEW_MENU`)       | Siguiente opción del menú  | Atajo de atrás/cancelar | Entrar a opción seleccionada | Sin acción                     | Opción anterior del menú | Apaga              |
| Clicker (`VIEW_CLICKER`) | +1 al contador             | Volver a menú           | +10 al contador              | Reset contador a 0             | Volver a menú            | Apaga              |
| IMU (`VIEW_IMU`)         | Cambia modo visual (M1/M2) | Volver a menú           | Pausa/Reanuda lectura IMU    | Sin acción                     | Volver a menú            | Apaga              |

### Resumen de UX

- `B` = adelante / aceptar
- `PWR` corto = atrás / no / cancelar
- `PWR` largo = apagar
- `B` largo = cancelar/atrás (atajo)
