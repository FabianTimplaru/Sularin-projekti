
# Sularin – Backend

Tämä backend on osa Sularin IoT -projektia. Palvelin on toteutettu Node.js:n, Expressin ja TypeScriptin avulla.

Backend vastaanottaa Raspberry Pi Pico 2 W:n lähettämiä anturimittauksia, validoi vastaanotetun datan ja tarjoaa REST API -rajapinnan mittausten hakemiseen sekä asetusten hallintaan.

Palvelin käyttää Zod-kirjastoa tietojen validointiin ja API-tokenia mittausten lähettäjän tunnistamiseen.

## Vaatimukset

Ennen asennusta tarvitset seuraavat ohjelmistot:

- Node.js ja npm
- Git (projektin kloonaamista ja päivittämistä varten)

## Asennus

### 1. Projektin lataaminen tai päivittäminen

Jos projektia ei ole vielä ladattu tietokoneelle, kloonaa GitHub-repositorio:

```bash
git clone https://github.com/FabianTimplaru/Sularin-projekti.git
```

Jos projekti löytyy jo tietokoneelta, siirry projektin juurikansioon ja hae viimeisimmät muutokset:

```bash
git pull origin main
```

**Huomio:** Ennen `git pull` -komentoa varmista, ettei paikallisia tallentamattomia muutoksia ole. Tarvittaessa tallenna muutokset commitilla tai käytä `git stash` -komentoa.

### 2. Siirtyminen backend-kansioon

Jos kloonasit projektin juuri nyt, siirry backend-kansioon:

```bash
cd Sularin-projekti/backend
```

Jos olet jo projektin juurikansiossa:

```bash
cd backend
```

### 3. Riippuvuuksien asentaminen

Asenna projektin tarvitsemat riippuvuudet:

```bash
npm ci
```

Komento asentaa kaikki `package-lock.json`-tiedostossa määritellyt riippuvuudet.

Projektissa käytetään myös `dotenv`-kirjastoa ympäristömuuttujien lataamiseen. Sen tulee sisältyä projektin riippuvuuksiin.

Jos `dotenv` ei vielä sisälly projektiin, sen voi asentaa komennolla:

```bash
npm install dotenv
```

Asennuksen jälkeen myös `package.json` ja `package-lock.json` tulee päivittää GitHubiin.

### 4. Ympäristömuuttujien määrittäminen (.env)

Backend käyttää ympäristömuuttujia API-tokenin turvallisempaan käsittelyyn.

Luo `backend`-kansion juureen tiedosto nimeltä `.env`.

Kansiorakenne näyttää seuraavalta:

```text
backend/
├── src/
├── .env
├── package.json
├── package-lock.json
└── tsconfig.json
```

Lisää `.env`-tiedostoon seuraava muuttuja:

```dotenv
API_TOKEN=VAIHDA_TAHAN_OMA_SALAINEN_AVAIN
```

Korvaa esimerkkiarvo omalla riittävän pitkällä ja vaikeasti arvattavalla tokenilla.

Tokenin on oltava sama sekä backendissä että Raspberry Pi Pico 2 W:n määrityksissä, jotta Pico voi lähettää mittauksia palvelimelle.

Backend lukee ympäristömuuttujat `dotenv`-kirjaston avulla:

```typescript
import 'dotenv/config';
```

API-tokeniin päästään käsiksi seuraavasti:

```typescript
process.env.API_TOKEN
```

**Tärkeää: `.env`-tiedostoa ei saa koskaan lisätä GitHub-repositorioon!**

Tiedosto voi sisältää salasanoja, API-tokeneita ja muita salaisia tietoja.

Varmista, että projektin juurikansion `.gitignore`-tiedostossa on seuraava rivi:

```gitignore
.env
```

Tämä estää Gitin normaalisti lisäämästä `.env`-nimisiä tiedostoja versionhallintaan, kun tiedostot eivät ole jo seurannassa.

Voit tarkistaa, että Git ohittaa tiedoston, suorittamalla projektin juurikansiosta:

```bash
git check-ignore -v backend/.env
```

Jos `.env` on jo lisätty Gitin seurantaan, sen voi poistaa seurannasta säilyttäen paikallisen tiedoston:

```bash
git rm --cached backend/.env
```

**Huomio:** `.gitignore` ei poista aiemmin julkaistuja salasanoja Git-historiasta. Jos oikea token on päätynyt GitHubiin, token tulee vaihtaa uuteen.

## Palvelimen käynnistäminen

Suorita seuraavat komennot `backend`-kansiossa.

### Kehitystila

Käynnistä palvelin kehitystilassa:

```bash
npm run dev
```

Palvelin käynnistyy osoitteeseen:

http://localhost:3000

Kehitystilassa palvelin käynnistyy automaattisesti uudelleen lähdekoodin muuttuessa.

### Tuotantotila

Käännä TypeScript-koodi JavaScriptiksi:

```bash
npm run build
```

Käynnistä käännetty sovellus:

```bash
npm start
```

## Asennuksen testaaminen

Avaa selaimessa seuraava osoite:

http://localhost:3000

Jos palvelin toimii oikein, selaimessa näkyy:

`Sularin api is running`

Voit myös tarkistaa palvelimen tilatiedot osoitteessa:

http://localhost:3000/api/status

## API-rajapinnat

| Metodi | Osoite | Kuvaus |
|--------|--------|--------|
| GET | / | Palvelimen toiminnan tarkistus |
| GET | /api/status | Palvelimen tilatiedot |
| GET | /api/measurements | Kaikkien mittausten hakeminen |
| GET | /api/measurements/latest | Viimeisimmän mittauksen hakeminen |
| POST | /api/measurements | Uuden mittauksen lähettäminen |
| GET | /api/settings | Nykyisten asetusten hakeminen |
| PATCH | /api/settings | Asetusten päivittäminen |

### Esimerkki mittauksen lähettämisestä

Mittauksia voidaan lähettää esimerkiksi Postman-ohjelmalla tai suoraan Raspberry Pi Pico 2 W -laitteelta HTTP POST -pyynnöllä.

**Metodi:** `POST`

**Osoite:** `http://localhost:3000/api/measurements`

**Content-Type:** `application/json`

**Body (JSON):**

```json
{
  "token": "OMA_SALAINEN_API_TOKEN",
  "temp": 23.5,
  "airHumidity": 40.0,
  "soilMoisture_adc": 2500,
  "soilMoisture_percent": 65.0
}
```

Korvaa `OMA_SALAINEN_API_TOKEN` samalla arvolla, jonka olet määrittänyt `.env`-tiedostoon.

Lähetettävät tiedot:

| Kenttä | Kuvaus |
|--------|--------|
| token | API-token lähettäjän tunnistamista varten |
| temp | Ilman lämpötila Celsius-asteina |
| airHumidity | Ilman suhteellinen kosteus prosentteina |
| soilMoisture_adc | Mullan kosteusanturin raaka ADC-arvo |
| soilMoisture_percent | Laskettu mullan suhteellinen kosteusarvo prosentteina |

Tällä hetkellä kaikki viisi kenttää ovat pakollisia.

Palvelin tarkistaa Zod-kirjastolla lähetettyjen tietojen tyypit ja sallitut arvovälit. Tämän jälkeen API-tokenia verrataan palvelimen ympäristömuuttujaan.

Onnistuneen lähetyksen jälkeen palvelin palauttaa HTTP-tilakoodin `201 Created` ja seuraavan vastauksen:

```json
{
  "message": "Measurements received",
  "success": true
}
```

Palvelin lisää mittaukseen automaattisesti aikaleiman (`timestamp`), joten Picon ei tarvitse lähettää sitä.

API-tokenia ei tallenneta mittausten yhteyteen.

### API-tokenin tarkistus ja virhetilanteet

Backend tarkistaa jokaisen mittausten lähettämiseen käytetyn POST-pyynnön yhteydessä, että API-token vastaa ympäristömuuttujaan tallennettua tokenia.

| Tilanne | HTTP-tilakoodi |
|---------|----------------|
| Oikea token ja kelvolliset mittaukset | 201 Created |
| Virheellinen API-token | 401 Unauthorized |
| Puuttuva API-token | 400 Bad Request |
| Virheelliset mittaustiedot | 400 Bad Request |

Virheellisen API-tokenin tapauksessa palvelin palauttaa esimerkiksi:

```json
{
  "error": "Not authorised"
}
```

Jos lähetetty data ei vastaa määriteltyä Zod-skeemaa, palvelin palauttaa:

```json
{
  "error": "Invalid measurement data"
}
```

Virheellisiä tai luvattomia mittauksia ei tallenneta.

### Mittausten hakeminen

Kaikki tallennetut mittaukset voidaan hakea osoitteesta:

`GET /api/measurements`

Viimeisin mittaus voidaan hakea osoitteesta:

`GET /api/measurements/latest`

Mittaukset palautetaan JSON-muodossa. Vastaukseen sisältyy palvelimen luoma aikaleima, mutta ei API-tokenia.

Jos mittauksia ei ole vielä tallennettu, viimeisimmän mittauksen haku palauttaa HTTP-tilakoodin `404 Not Found`.

### Esimerkki asetusten päivittämisestä

**Metodi:** `PATCH`

**Osoite:** `http://localhost:3000/api/settings`

**Body (JSON):**

```json
{
  "minMoistureThreshold": 30.0
}
```

Asetus määrittää mullan kosteuden alarajan prosentteina.

Asetusten validointiin käytetään Zod-kirjastoa. Tällä hetkellä sallittu raja-arvo on 10–50 prosenttia.

Onnistuneen päivityksen jälkeen palvelin tallentaa uuden arvon muistiin ja palauttaa päivitetyt asetukset HTTP-tilakoodilla `200 OK`.

Nykyiset asetukset voidaan hakea osoitteesta:

`GET /api/settings`

Esimerkkivastaus:

```json
{
  "minMoistureThreshold": 30
}
```

Tarkoituksena on, että Raspberry Pi Pico 2 W hakee asetukset palvelimelta säännöllisesti HTTP GET -pyynnöillä.

Kun raja-arvoa muutetaan frontendin kautta PATCH-pyynnöllä, Pico saa uuden arvon seuraavalla onnistuneella asetusten hakukerralla.

Pico käyttää vastaanotettua raja-arvoa LED-varoituksen ohjaamiseen.

**Huomio:** Nykyisessä toteutuksessa API-tokenin tarkistus koskee mittausten POST-pyyntöä. Asetusten muuttamiseen tarkoitettua PATCH-rajapintaa ei ole vielä suojattu vastaavalla tunnistautumisella. Tämä tulee huomioida ennen palvelimen julkaisemista internetiin.

## Raspberry Pi Pico 2 W:n yhdistäminen

Kun backend toimii paikallisesti, Raspberry Pi Pico 2 W voi lähettää mittauksia samaan lähiverkkoon yhdistetylle tietokoneelle.

Pico ei voi käyttää `localhost`-osoitetta tietokoneen backend-palvelimen tavoittamiseen, koska `localhost` viittaa aina pyynnön lähettävään laitteeseen.

Sen sijaan käytetään backend-palvelinta suorittavan tietokoneen lähiverkon IP-osoitetta.

Windowsissa osoitteen voi tarkistaa komennolla:

```powershell
ipconfig
```

Esimerkiksi jos tietokoneen IP-osoite on `192.168.1.100`, Pico lähettää mittaukset osoitteeseen:

```text
http://192.168.1.100:3000/api/measurements
```

Picon ja tietokoneen on oltava verkossa, jossa laitteiden välinen liikenne on sallittu. Lisäksi tietokoneen palomuurin tulee sallia yhteys backend-palvelimen porttiin 3000.

## Huomioitavaa

- Mittaukset ja asetukset tallennetaan tällä hetkellä palvelimen keskusmuistiin.
- Tallennetut mittaukset häviävät palvelimen uudelleenkäynnistyksen yhteydessä.
- Asetusten muutokset palautuvat oletusarvoihin palvelimen uudelleenkäynnistyksen yhteydessä.
- Palvelin käyttää oletuksena porttia 3000.
- Varsinaista tietokantaa ei ole vielä käytössä.
- `.env`-tiedosto sisältää salaisia tietoja, eikä sitä saa koskaan julkaista GitHubissa.
- API-token tulee määrittää erikseen jokaisessa ympäristössä, jossa backend suoritetaan.
- Paikallisen HTTP-yhteyden liikenne ei ole salattu. Jos backend julkaistaan internetiin, yhteyksissä tulee käyttää HTTPS-protokollaa.
- Ennen julkista käyttöönottoa myös asetusten muuttamiseen tarkoitettu rajapinta tulee suojata asianmukaisesti.
