
# Sularin – Backend

Tämä backend on osa Sularin IoT -projektia. Palvelin on toteutettu Node.js:n, Expressin ja TypeScriptin avulla.

Backend vastaanottaa Raspberry Pi Pico 2 W:n lähettämiä anturimittauksia, validoi vastaanotetun datan ja tarjoaa REST API -rajapinnan mittausten hakemiseen sekä asetusten hallintaan.

## Vaatimukset

Ennen asennusta tarvitset seuraavat ohjelmistot:

- Node.js ja npm
- Git (projektin kloonaamista varten)


## Asennus

1. Kloonaa projektin GitHub-repositorio, jos sitä ei ole vielä ladattu tietokoneelle:

```bash
git clone https://github.com/FabianTimplaru/Sularin-projekti.git
```

Jos projekti löytyy jo tietokoneelta, siirry projektin juurikansioon ja hae viimeisimmät muutokset GitHubista:

```bash
git pull origin main
```

Huomioi, että ennen `git pull` -komentoa paikalliset muutokset kannattaa tallentaa tai tehdä niistä commit, jotta mahdollisilta ristiriidoilta vältytään.

2. Siirry backend-kansioon:

```bash
cd backend
```

Jos kloonasit projektin juuri nyt, siirry ensin projektin juurikansioon:

```bash
cd Sularin-projekti/backend
```

3. Asenna projektin riippuvuudet:

```bash
npm ci
```


## Palvelimen käynnistäminen

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

Voit myös tarkistaa API:n tilan osoitteessa:

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

## Huomioitavaa

- Mittaukset ja asetukset tallennetaan tällä hetkellä palvelimen keskusmuistiin.
- Tallennetut mittaukset häviävät palvelimen uudelleenkäynnistyksen yhteydessä.
- Palvelin käyttää porttia 3000.
- Tietokantatuki lisätään myöhemmässä kehitysvaiheessa.
- Raspberry Pi Pico 2 W:n yhdistäminen paikalliseen palvelimeen edellyttää tietokoneen lähiverkon IP-osoitteen käyttämistä `localhost`-osoitteen sijaan.

# Sularin – Backend

Tämä backend on osa Sularin IoT -projektia. Palvelin on toteutettu Node.js:n, Expressin ja TypeScriptin avulla.

Backend vastaanottaa Raspberry Pi Pico 2 W:n lähettämiä anturimittauksia, validoi vastaanotetun datan ja tarjoaa REST API -rajapinnan mittausten hakemiseen sekä asetusten hallintaan.

## Vaatimukset

Ennen asennusta tarvitset seuraavat ohjelmistot:

- Node.js ja npm
- Git (projektin kloonaamista varten)

## Asennus

1. Kloonaa projektin GitHub-repositorio:

```bash
git clone https://github.com/FabianTimplaru/Sularin-projekti.git
```

2. Siirry backend-kansioon:

```bash
cd Sularin-projekti/backend
```

3. Asenna projektin riippuvuudet:

```bash
npm ci
```

## Palvelimen käynnistäminen

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

Voit myös tarkistaa API:n tilan osoitteessa:

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

## Huomioitavaa

- Mittaukset ja asetukset tallennetaan tällä hetkellä palvelimen keskusmuistiin.
- Tallennetut mittaukset häviävät palvelimen uudelleenkäynnistyksen yhteydessä.
- Palvelin käyttää porttia 3000.
- Tietokantatuki lisätään myöhemmässä kehitysvaiheessa.
- Raspberry Pi Pico 2 W:n yhdistäminen paikalliseen palvelimeen edellyttää tietokoneen lähiverkon IP-osoitteen käyttämistä `localhost`-osoitteen sijaan.
