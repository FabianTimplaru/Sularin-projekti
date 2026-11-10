
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
