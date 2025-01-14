# Projet IF3B
## Contrôle d'accès à un bâtiment

### Contexte :
Dans le cadre des bâtiments modernes, le **contrôle d'accès** représente un enjeu crucial en matière de sécurité, d'efficacité et de gestion des flux de personnes. Les solutions classiques, souvent basées sur des serrures mécaniques ou des systèmes peu intelligents, présentent des limitations : faible flexibilité, difficulté d'intégration avec des systèmes domotiques, et absence de suivi en temps réel.
Pour répondre à ces défis, **notre projet vise à concevoir une solution innovante et interactive de contrôle d'accès pour un bâtiment quelconque**. L'objectif est de développer un système fonctionnel et modulable, intégrant la **communication bidirectionnelle** via Node-RED

### Identification des problèmes :
- Comment faire une porte qui permet d’être verrouillé et qui peut s’ouvrir à distance ?
- Comment indiquer de manière sécurisé qu’une porte va s’ouvrir / se fermer ?
- Comment faire un accès sécurisé et à la fois polyvalent ?
- Comment tracer les entrées des employées par exemple ?

### Solutions proposées :
- Système d’ouverture / fermeture avec porte coulissante modulable via nodered
- Utilisation d’un afficheur OLED, d’un voyant sonore et visuel pour la fermeture et prévenir également avant la fermeture
- Utiliser à la fois un lecteur de badge (module RFID) et un pavé tactile permettant une solution polyvalente
- Permettre l’utilisation d’un badge spécifique ou bien lié un code aux badges quelconques pour déterminer l’entrée d’un employé.

### Répartition des tâches : 
- Conception hardware
- Câblage
- Conception software (capteurs, actionneurs, communication MQTT via wifi)
- Mise en place du serveur NodeRed

### Capteurs / Actionneurs :
- __Module RFID :__ Pour permettre à quiconque possédant un badge valide de rentrer
- __Pavé tactile (keypad) :__ Pour permettre à quiconque possédant le code de rentrer
- __Buzzer :__ Pour indiquer par un bruit toute action fais par l’utilisateur ou bien par le mécanisme susceptible d’être un danger
- __Afficheur OLED :__ Pour indiquer visuellement les instructions à faire ou bien énoncé les erreurs
- __LedStick 8 leds neopixel :__ Pour éclairé l’intérieur, modulable très facilement
- __Moteur stepper :__ moteur pas à pas pour pouvoir faire des rotations sur 360° et ainsi permettre l’utilisation d’un rail et d’un pignon pour ouvrir la porte
- __Serrure à solénoïde :__ serrure permettant d’ouvrir facilement la porte, se ferme en se réenclenchant.

**__Autres :__**
- Résistance
- Led rouge
- Alimentation 12V pour serrure à solénoïde
- Relais pour serrure
- Transformateur pour stepper
- ESP32
- Breadboard

### Détails des pins
**RFID** :
- RST : D4 (modulable)
- MISO : D19
- MOSI : D23
- SCK : D18
- SDA : D5

**Moteur stepper** :
- int1 : D14 (modulable)
- int2 : D27 (modulable)
- int3 : D26 (modulable)
- int4 : D25 (modulable)

**Keypad** :
- RX : RX2
- TX : TX2

**Serrure à solénoïde** :
- SIG : D32 (modulable)

**Afficheur OLED** :
- SCL : D22
- SDA : D21

**LedStick 8 leds neopixel** :
- DIN : D33 (modulable)

**Buzzer** :
- I/O : D13 (modulable)

**Led rouge** :
- D13 (modulable)

**VCC / GND** :
Tous les pins VCC (pour le courant) et GND (pour la masse), sont à brancher sur une breadboard

**__Attention :__** La led ne pourra pas fonctionner correctement, tous les pins jugés "safe" sur l'esp32 sont utilisés, la led s'allumera donc à chaque impulsion électrique envoyé pour faire fonctionner le buzzer

### Câblage
<img width="651" alt="Cablage" src="https://github.com/user-attachments/assets/30147bf9-b54b-415d-bfe6-369ea2827b7c" />
(seul la serrure est manquante)

### Rendu final
https://github.com/Jipmaa/IF3B_Controle_Acces_Batiment/tree/e7f2aa74cef77e2d99e59e1e05c4c83b59b73495/Rendu
