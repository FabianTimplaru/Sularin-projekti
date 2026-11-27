# Sularin-projekti: IOT-sovellus - Kasvuolosuhteiden seurantajärjestelmä​
## Johdanto
Tehtävänä oli suunnitella ja toteuttaa itse rakennettu sensori, jolla voidaan mitata valittua ilmiötä. Muutamien vaihtoehtojen tarkastelun jälkeen projektin aiheeksi valittiin mullan kosteuden mittaaminen. 
Järjestelmä on rakennettu resurssipula-teemalla: Laitteen rakentaminen on tehty mahdollisimman primitiivisillä komponenteilla ja työkaluilla.
Projektin ensimmäisessä vaiheessa toteutettiin itse rakennettu maankosteusanturi sekä mittausjärjestelmä, joka perustuu Raspberry Pi Pico 2 W -alustaan. Järjestelmä mittaa maaperän kosteutta ja muuntaa mittaustulokset prosentuaaliseen muotoon käyttäjän tulkit-tavaksi.
Projektin toisessa vaiheessa järjestelmää laajennettiin lisäämällä DHT11-anturi lämpötilan ja ilmankosteuden mittaamista varten. Lisäksi järjestelmään lisättiin LED-merkkivalo, jonka avulla voidaan ilmaista, kun maaperän kosteus alittaa asetetun hälytysrajan. Näin käyttäjä saa välittömän ilmoituksen kasvin mahdollisesta kastelutarpeesta.
Laitteessa on valmis kalibraatio tehty ja tämä on asetettu koodiin. Mutta jos asetukset eivät sovellu olosuhteisiin, käyttäjäkalibraatio on mahdollinen suorittaa resurssipulaa ajatellen käyttäen Raspberry Pi Picon Bootsel-painiketta. Samalla painikkeella pystytään myös palauttamaan tehdasasetukset laitteelle.
Mittaustietojen keskiarvot lähetetään kymmenen minuutin välein palvelimelle.
Järjestelmän ympärille toteutettiin myös backend- ja frontend-ratkaisut. Backend vastaan-ottaa mittausdataa laitteelta JSON-muodossa ja mahdollistaa järjestelmän asetusten hal-linnan ja frontend on käyttöliittymä, jossa nähdään mittaukset ja voidaan muokata  mullankuivuus hälytysraja-arvoa.
## Kansiorakenne
Projektin kansiorakenne näyttää seuraavalta:
```text
sularin-projekti/
├──backend
├──calibration
├──firmware
└──front
``` 
## Käyttöönotto
### Riippuvuudet ja vaatimukset:
#### Yleiset riippuvuudet:
- GIT (projektin kloonaamista ja päivittämistä varten)
#### Firmware
- Kääntötyökalut: Cmake, Ninja (tai Make), Python 3 sekä arm-none-eabi-gcc
- SDK ja kirjastot: Raspberry Pi Pico SDK
- firmware/wifi_config.h Määritetty --> WIFI_SSID, WIFI_PASSWORD, SERVER_ADDRESS ja API_KEY
- Laitteistomoduulit: dht11, pico_led ja bootsel_button
#### Backend
- Node.js ja npm
- express
- dotenv
- zod
- typescript, @types/node, @types/express, tsx/ts-node
- backend/.env ---> API_TOKEN
#### Frontend
- Moderni verkkoselain, joka tukee EventSource-rajapintaa (SSE) ja fetch-pyyntökä
- script.js HISTORIA_URL ({SERVER_ADDRESS}:3000/api/measurements) RAJA_URL  ({SERVER_ADDRESS}:3000/api/settings)

### Projektin lataaminen tai päivittäminen

Jos projektia ei ole vielä ladattu tietokoneelle, kloonaa GitHub-repositorio:

```bash
git clone https://github.com/FabianTimplaru/Sularin-projekti.git
```

Jos projekti löytyy jo tietokoneelta, siirry projektin juurikansioon ja hae viimeisimmät muutokset:

```bash
git pull origin main
```

**Huomio:** Ennen `git pull` -komentoa varmista, ettei paikallisia tallentamattomia muutoksia ole. Tarvittaessa tallenna muutokset commitilla tai käytä `git stash` -komentoa.

### Tarvittavat komponentit ja työkalut
- Raspberry Pi Pico 2 W
- Koekytkentälevy
- 2 kpl M6x70mm Pultit
- 6 kpl M6 Muttereita
- Muovinen kiinnityslevy 2 - 8 mm
- DHT11
- Punainen LED
- 2 kpl 2K2-vastuksia
- Yksisäikeistä kytkentäjohtoa 0,6mm
- mitta
- Sivuleikkurit tai sakset
- työkalu kiinnityslevyn rei'ittämiseksi.
### Laitteen fyysinen rakentaminen
Käyttöönotossa lähdetään liikkeelle kasaamalla Raspberry Pi Pico-laite kytkentäkaavion mukaisesti:
1. Kehitysalusta Raspberry Pi Pico 2W asennetaan koekytkentälevyyn pitkittäin keskelle, mahdollistaen kaikkien pinnien käyttämisen. Kehitysalusta on hyvä varmistaa, että se on pohjassa asti, jotta liitännät ovat varmasti hyvin kytkeytyneet.
2. Leikataan leikkuulaudasta 10*cm10cm pala saksilla
3. Merkitään kaksi pistettä 3 cm toisistaan.
4. Perforoidaan reiät merkittyihin kohtiin.
5. Suurennetaan reiät siten, että pultit mahtuvat läpi rei’istä. Siistitään reikien ympärys.
6. Kierretään yksi mutteri molempien pulttien ympärille ja asennetaan pultit levyn reikiin. Etäisyys levystä pultin kantaan on 58mm.
7. Asennetaan molempiin pultteihin toinen mutteri.
8. Leikataan 30cm johdot: Musta ja vihreä.
9. Kuoritaan johdot 3 cm:n matkalta toisesta päästä.
10. Punotaan kaapelin kuoritut päät pulttien ympärille ja kiinnitetään ne muttereilla.
11. Leikataan kapeahko 3cm*10cm levynpala ja Merkitään kaksi pistettä 3 cm toisistaan.
12. Perforoidaan reiät merkittyihin kohtiin ja suurennetaan ne pultteja varten sopiviksi.
13. Asetetaan kapea levy pulttien kärkien päälle.
14. Molemmat yksisäikeisten johtojen irralliset päät kuoritaan noin 5mm.
15. Valitaan 2,2 kΩ vastus ja asetetaan se [33]GND:n ja [31]GP26(ADC)-pinneihin.
16. Musta johto kytketään [31]GP26(ADC)-pinniin.
17. Vihreä johto kytketään [20]GP15-pinniin.
18. Leikataan sopivan mittaiset johdot DHT11 varten ja LEDiä varten ja kuoritaan johtojen päät
19. Asetetaan DHT11 ja LED kytkentälevylle
20. DHT11 1-VCC kytketään [36]3V3-pinniin
21. DHT11 2-DATA kytketään [19]GP14-pinniin
22. DHT11 4 GND kytketään [33] GND-pinniin
23. LED + kytketään 2,2 kΩ vastukseen ja [17]GP13-pinniin
24. LED - kytketään [33]GND
25. Tarkistetaan lopputulos ja todetaan laite valmiiksi

### Firmwaren käyttöönotto:
#### Firmwaren kansiorakenne
```text
firmware/
├── include/
├── lib/dht11
├── src/
├── wifi_config.h
├── CMakeLists.txt
├── pico_sdk_import.cmake
└── wifi_config example.h
```
#### Firmwaren asennus
Kun olet projekti-kansiossa terminaalissa/Powershellissä/CMD:ssä (cmake/make):

Siirrytään firmware-kansioon projektin juurikansiosta:
```bash
cd firmware
```
Luodaan build-kansio ja siirrytään kyseiseen kansioon
```bash 
mkdir build
```
```bash
cd build
```
- Esikäännetään koodi
```bash
cmake -DPICO_BOARD=pico2_w ..
```
- Käännetään esikäännetty-koodi laitteelle sopivaksi 
```bash
make -j4
```
- Siirrä PicoApp.uf2-tiedosto Raspberry Pi Picoon

**Huomio:** firmware/wifi_config.h pitää olla määritelty asetukset, jotta laite toimii oikein.

### Backend:n käyttöönotto:
#### Backendin kansiorakenne:
```text
backend/
├── src/
├── .env
├── package.json
├── package-lock.json
└── tsconfig.json
```
#### Backendin asentaminen
Siirrytään backend-kansioon projektin juurikansiosta
```bash
cd backend
```
Asennetaan projektin tarvitsemat riippuvuudet
```bash
npm ci
```
Komento asentaa kaikki `package-lock.json`-tiedostossa määritellyt riippuvuudet.

Projektissa käytetään myös `dotenv`-kirjastoa ympäristömuuttujien lataamiseen. Sen tulee sisältyä projektin riippuvuuksiin.

Jos `dotenv` ei vielä sisälly projektiin, sen voi asentaa komennolla:

```bash
npm install dotenv
```

Lisää `.env`-tiedostoon seuraava muuttuja:

```dotenv
API_TOKEN=VAIHDA_TAHAN_OMA_SALAINEN_AVAIN
```

Tokenin on oltava sama sekä backendissä että Raspberry Pi Pico 2 W:n määrityksissä, jotta Pico voi lähettää mittauksia palvelimelle.

#### Palvelimen käynnistäminen

Suorita seuraavat komennot `backend`-kansiossa.

##### Kehitystila

Käynnistä palvelin kehitystilassa:

```bash
npm run dev
```

Palvelin käynnistyy osoitteeseen:

http://localhost:3000

Kehitystilassa palvelin käynnistyy automaattisesti uudelleen lähdekoodin muuttuessa.

##### Tuotantotila

Käännä TypeScript-koodi JavaScriptiksi:

```bash
npm run build
```

Käynnistä käännetty sovellus:

```bash
npm start
```
