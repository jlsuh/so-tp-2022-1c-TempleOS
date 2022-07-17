# TP Sistemas Operativos 1° Cuatrimestre 2022

<p align="center">
    <img src="https://user-images.githubusercontent.com/38252227/179381955-12534079-9667-4b02-87ce-208a9b8cbafd.png">
</p>

<p align="center">
    <strong>
        “For I know the plans I have for you,” declares the Lord, “plans to prosper you and not to harm you, plans to give you hope and a future”. - Jer 29:11
    </strong>
</p>

<p align="center">
    <strong>
        ‘He will wipe every tear from their eyes. There will be no more death’ or mourning or crying or pain, for the old order of things has passed away.” - Rev 21:4
    </strong>
</p>

## Integrantes
| Apellido, Nombre     | Usuario Github                                     | Módulo                  |
| -------------------- | -------------------------------------------------- | ----------------------- |
| Berro, Camila Andrea | [@Camila1401](https://github.com/Camila1401)       | Memoria                 |
| Lingeri, Martin      | [@MartinLingeri](https://github.com/MartinLingeri) | Memoria                 |
| Partarrie, Matias    | [@matiaspar2000](https://github.com/matiaspar2000) | Kernel / CPU            |
| Yarbuh, Juan Ignacio | [@Juyah](https://github.com/Juyah)                 | Memoria / CPU / Consola |
| Suh, Joel            | [@jlsuh](https://github.com/jlsuh)                 | Kernel / CPU / Consola  |

## Directivas e Implementación
| Documento                                                                                         |
| ------------------------------------------------------------------------------------------------- |
| [Enunciado](https://docs.google.com/document/d/17WP76Vsi6ZrYlpYT8xOPXzLf42rQgtyKsOdVkyL5Jj0/edit) |
| [Pruebas](https://docs.google.com/document/d/1SBBTCweMCiBg6TPTt7zxdinRh4ealRasbu0bVlkty5o/edit#)  |

## Pre-Deploy
1. Creación de VMs
    - Nombre: SO Server
    - Tipo: Linux
    - Versión: Ubuntu 32 bits
    - Usar un archivo de disco duro virtual existente e indicar `.vdi`
2. Implementar `Configuración > Red > Avanzadas > Reenvío de Puertos`:
    ![image](https://user-images.githubusercontent.com/38252227/179384699-a7157de7-a668-4c96-b104-7ef7cb74bd46.png)
3. Levantar VM
4. `ifconfig` y anotar IP local (LAN) de Kernel, Memoria & CPU en un google docs
5. Conectar con putty a la VM
    - IP: `192.168.0.x`
    - Puerto: 22

## Deploy
```bash
git clone https://<PersonalAccessToken>@github.com/jlsuh/so-tp-2022-1c-TempleOS.git
cd so-tp-2022-1c-TempleOS
bash deploy.sh <PersonalAccessToken>
bash config_ip.sh <kernelIP> <memoriaIP> <cpuIP>
```

## Ejecución Memoria / CPU / Kernel
```bash
cd <moduleName>
bash exec_<moduleName>.sh cfg/<useCase>.cfg
```

## Ejecución Consola(s)
```bash
cd consola
bash ejecuciones-de-prueba/<useCase>.sh
```
