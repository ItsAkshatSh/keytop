<h1 align="center">
  <br>
  <img src="https://raw.githubusercontent.com/ItsAkshatSh/keytop/refs/heads/main/ref/header.png" width=90%>
  <br>
  <br>
  keytop
  <br>
</h1>
<div align="center">

![ZMK](https://img.shields.io/badge/Made%20with-ZMK-2ec4b6)
![EasyEDA](https://img.shields.io/badge/Made%20with-EasyEDA-07162A)


Ever wanted a bluetooth keyboard that acted like a laptop keyboard, introducing keytop. Keytop runs on the Seeed Studio XIAO nrf52840, used for it's battery module, and bluetooth capabilities, along with 47 kailh choc v2 keys, and a TPS65201a trackpad module

### why?
</div>
I always wanted a keyboard that could connect to my pc while I was on my bed, so something that I can type with and move my cursor with, and the only thing that comes close to this is a laptop keyboard

<div align='center'>

### Parts!!

</div>

- ***47x Kailh Choc V2*** switches, for them clicklity clacking
- ***Seeed Studio XIAO nrf52840***, has great bluetooth capabilities and also includes a battery module!
- ***TPS65201a***, it's the trackpad module *(firmware for this is hell)*
- ***MCP23017-E/SP***, *EXTRRAAAA pins*
- ***1000 mAH 3.7V Lipo Battery***, Lowk huge battery

<div align='center'>

### Hardware!

</div>

want to work on this? head [here](https://github.com/ItsAkshatSh/keytop/tree/main/Hardware)

- Refer to the [BOM](https://github.com/ItsAkshatSh/keytop/blob/main/BOM.csv)
- Download the PCB files from [here](https://github.com/ItsAkshatSh/keytop/tree/main/Hardware)
- Order using the provided Gerber files
- Solder all components
- Download ZMK and other related softwares
- Build the UF2 boot file(or get it from [here](https://github.com/ItsAkshatSh/keytop/tree/main/Firmware/Build)) and place it under the new drive called ```XIAO-BLE```, using a Data cable
- and enjoy your new keyboard!

<div align='center'>

### Schematic

<img src="https://raw.githubusercontent.com/ItsAkshatSh/keytop/refs/heads/main/ref/PCBImgs/SCH.png">

### PCB

<img src = "https://github.com/ItsAkshatSh/keytop/blob/main/ref/PCBImgs/PCB.png?raw=true" width=700px height=900px>

<img src = "https://github.com/ItsAkshatSh/keytop/blob/main/ref/PCBImgs/3dPCB.png?raw=true" width=700px height=900px>

### CAD

<img src = "https://github.com/ItsAkshatSh/keytop/blob/main/ref/PCBImgs/CASE1.png?raw=true" width=700px height=900px>

Check it out on [Onshape](https://cad.onshape.com/documents/347e1b4c32ca0e99b6b4bdf0/w/21b40434a25b8381c4bb2092/e/18c35be39fdcfdaaf1d80c9d?renderMode=0&uiState=6a1940416ba01f1f6cb9e873)

### Firmware

</div>

The firmware is written using ZMK,
```
config/
  boards/
    arm/
      xiao_kb/
        CMakeLists.txt
        Kconfig.board
        Kconfig.defconfig
        Kconfig.xiao_kb
        board.cmake
        board.yml
        xiao_kb.defconfig
        xiao_kb.dts
        xiao_kb.yaml
        xiao_kb_defconfig
  drivers/input/
    CMakeLists.txt
    Kconfig
    tps65201a.c
```


<div align='center'>

### BOM



Check it out [here](https://github.com/ItsAkshatSh/keytop/blob/main/BOM.csv)!

| S no.   | Part                                    | Qty.      | link                                                                                                                                                                                                              | Price(aed)   | Price($)   |
|:--------|:----------------------------------------|:----------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-------------|:-----------|
| 1       | Mechanical Switches - low profile       | 60        | [Link](https://mechanicalkeyboards.com/products/kailh-choc-round-red-43g-low-profile-linear?sacode=042505&utm_source=simple-affiliate&utm_medium=social&utm_campaign=Patrick+%287843938566444%29)                         | Dhs. 46.80   | $12.74     |
| 2       | Keycaps                                 | 54        | [Link](https://chosfox.com/products/chocfox-cfx-up-keycaps-set-pre-sale?variant=45884540256450&country=AE&currency=USD&utm_source=chatgpt.com)                                                                            | Dhs 58.39    | $15.90     |
| 3       | Trackpad Module - Module - TPS65-201A-S | 1         | [Link](https://jlcpcb.com/partdetail/Azoteq-TPS65_201AS/C6339186)                                                                                                                                                         | -            | $9.56      |
| 4       | Trackpad Module - Glass overlay         | 1         | [Link](https://keycapsss.com/Azoteq-ProxSense-I2C-Touch-Sensor-Module-Capacitive-Trackpad/KC10195-65-GLASS)                                                                                                               | -            | $8.13      |
| 5       | Seeed Studio XIAO nRF52840              | 1         | [Link](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html)                                                                                                                                                   | -            | $9.90      |
| 6       | MCP23017 - S                            | 1         | [Link](https://www.lcsc.com/product-detail/C647352.html?s_z=n_q_MCP23017%2520E%252FSP&spm=wm.ssy.bg.0.xh&lcsc_vid=QlAMAlZQE1QKUlNfElINUwACQQIKVwECQlhaUVRREVExVlNRQVJbVVZUQldcXzsOAxUeFF5JWBYZEEoKFBINSQcJGk4dAgUUFAk%3D) | -            | $3         |
| 7       | MakerHawk 1000mAh 3.7V Lipo Battery     | 1         | [Link](https://www.amazon.ae/gp/product/B091XYZ2V3/ref=ewc_pr_img_1?smid=A1WV019O4JSKKB&psc=1)                                                                                                                            | Dhs 58       | $16        |
| 8       | PCB                                     | 5/2(pcba) |                                                                                                                                                                                                                   |              | $102.74    |
|         | **Total** |           |                                                                                                                                                                                                                   |              | $177.77    |

</div>

<div align='center'>

### Zine

<img src="https://github.com/ItsAkshatSh/keytop/blob/main/Magazine.png?raw=true" width=70% height=70%>

# Thank you Hackclub!
