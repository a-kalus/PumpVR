# PumpVR
 PumpVR renders the weight of objects and avatars in Virtual Reality through liquid mass transfer. The system was presented in the paper <a href="https://doi.org/10.1145/3544548.3581172">PumpVR: Rendering the Weight of Objects and Avatars through Liquid Mass Transfer in Virtual Reality.</a> Please take a look at the article to get an overview of the project.

 
See `pump_vr.ino` for the arduino sketch and `arduinoCtrler.cs` for the Unity integration. For the hardware implementation see below.
 



 
 ## Reference
 To cite this work, please use the following reference information.
 

 ```
 @inproceedings{10.1145/3544548.3581172,
author = {Kalus, Alexander and Kocur, Martin and Klein, Johannes and Mayer, Manuel and Henze, Niels},
title = {PumpVR: Rendering the Weight of Objects and Avatars through Liquid Mass Transfer in Virtual Reality},
year = {2023},
isbn = {9781450394215},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
url = {https://doi.org/10.1145/3544548.3581172},
doi = {10.1145/3544548.3581172},
abstract = {Perceiving objects’ and avatars’ weight in Virtual Reality (VR) is important to understand their properties and naturally interact with them. However, commercial VR controllers cannot render weight. Controllers presented by previous work are single-handed, slow, or only render a small mass. In this paper, we present PumpVR that renders weight by varying the controllers’ mass according to the properties of virtual objects or bodies. Using a bi-directional pump and solenoid valves, the system changes the controllers’ absolute weight by transferring water in or out with an average error of less than 5%. We implemented VR use cases with objects and avatars of different weights to compare the system with standard controllers. A study with 24 participants revealed significantly higher realism and enjoyment when using PumpVR to interact with virtual objects. Using the system to render body weight had significant effects on virtual embodiment, perceived exertion, and self-perceived fitness.},
booktitle = {Proceedings of the 2023 CHI Conference on Human Factors in Computing Systems},
articleno = {263},
numpages = {13},
keywords = {virtual reality, virtual embodiment, weight interface, haptic controllers, weight perception},
location = {Hamburg, Germany},
series = {CHI '23}
}
```

 ## Circuit
 To build PumpVR create the circuit as follows:
 
![Alt text](/PumpVR_electronics_design.png?raw=true "Title")
| Nr | Part | Example |
| ------ | ------ | ------ |
| 1 | 12V Power Supply | [Link](https://www.reichelt.de/labornetzgeraet-1-16-v-0-30-a-stabilisiert-programmierbar-hcs-3300-usb-p132412.html?&trstct=pol_9&nbc=1) |
| 2 | Solenoid valves | [Link](https://www.amazon.de/dp/B09B3L6GQ3) |
| 3 | 4 channel 5V relay module | [Link](https://www.amazon.de/gp/product/B01M8G4Y7Z?psc=1) |
| 4 | Bluetooth RF-Transceiver Module | [Link](https://www.amazon.de/AZDelivery-Bluetooth-Wireless-RF-Transceiver-Modul-serielle/dp/B0722MD4FY) |
| 5 | Arduino Micro | [Link](https://store.arduino.cc/products/arduino-micro) |
| 6 | High current relay module | [Link](https://www.amazon.de/dp/B07TWH4PNF) |
| 7 | Reversible impeller pump | [Link](https://marco-pumps.shop/marco-up1-jr-reversible-impeller-pump-28-l-min-with-on-off-integrated-switch-12-volt-16201112/?xdomain_data=ZIyV2KFldmWS4NjUfp1Pbt%2Bor%2FiYXK%2BpGfbohCFOhLvaK3uwcySiJMpVoBA%3D) |


 ## Hydraulic system
To assemble the hydraulics please refer to the images below. You can use [these water bags (1 × 1l, 2 × 0.5l)](https://www.amazon.de/gp/product/B08C5J4RBN?psc=1) and conventional hoses with an inner diameter of 10 mm.
![chi23-489-fig3](https://user-images.githubusercontent.com/13471928/218504137-cfad9c12-9fdd-4ba1-9ad8-4d6968fc639d.jpg)


![pumpvr_wide](https://user-images.githubusercontent.com/13471928/218719153-8f31c4ac-8183-4378-9807-cbbdb411e295.jpg)
