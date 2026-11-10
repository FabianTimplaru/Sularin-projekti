
# Sularin – Backend

Tämä backend on osa Sularin IoT -projektia. Palvelin on toteutettu Node.js:n, Expressin ja TypeScriptin avulla.

Backend vastaanottaa Raspberry Pi Pico 2 W:n lähettämiä anturimittauksia, validoi vastaanotetun datan ja tarjoaa REST API -rajapinnan mittausten hakemiseen sekä asetusten hallintaan.

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

## Palvelimen käynnistäminen

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

Mittauksia voidaan lähettää esimerkiksi Postman-ohjelmalla.

**Metodi:** `POST`

**Osoite:** `http://localhost:3000/api/measurements`

**Body (JSON):**

```json
{
  "moisture": 45.5,
  "airHumidity": 60.2,
  "temperature": 22.5,
  "rawAdc": 2500
}
```

Onnistuneen lähetyksen jälkeen palvelin palauttaa HTTP-tilakoodin `201` ja seuraavan vastauksen:

```json
{
  "message": "Measurements received",
  "success": true
}
```

### Esimerkki asetusten päivittämisestä

**Metodi:** `PATCH`

**Osoite:** `http://localhost:3000/api/settings`

**Body (JSON):**

```json
{
  "minMoistureThreshold": 30
}
```

Asetus määrittää mullan kosteuden alarajan prosentteina.

## Huomioitavaa

- Mittaukset ja asetukset tallennetaan tällä hetkellä palvelimen keskusmuistiin.
- Tallennetut mittaukset häviävät palvelimen uudelleenkäynnistyksen yhteydessä.
- Asetusten muutokset palautuvat oletusarvoihin palvelimen uudelleenkäynnistyksen yhteydessä.
- Palvelin käyttää porttia 3000.
- Tietokantatuki lisätään myöhemmässä kehitysvaiheessa.
- Raspberry Pi Pico 2 W:n yhdistäminen paikalliseen palvelimeen edellyttää tietokoneen lähiverkon IP-osoitteen käyttämistä `localhost`-osoitteen sijaan.
