
  // ---raja-arvot ---
const MAX_LAMPO = 30;     // liikaa lämpöä
const MAX_KOSTEUS = 70;   // liikaa kosteutta

const HISTORIA_URL = 'data.json'; // 'http://localhost:3000/api/measurements'
const LATEST_URL = 'data.json'; // 'http://localhost:3000/api/measurements/latest'
const RAJA_URL = 'http://localhost:3000/api/settings'; // "minMoistureThreshold": 30.0 tieto 

    //  --- rajasäätimen asetukset ---
const rajaSaadin = document.getElementById('myRange');
const rajaArvo = document.getElementById('rajaArvo');
const rajaNappi = document.getElementById('rajaTallenna');

let minimiKosteus = 49; // ledin raja-arvo, päivittyy säätimellä

function naytaRaja() {
    rajaArvo.textContent = rajaSaadin.value + ' %';
}

async function lahetaRaja() {
    const arvo = Number(rajaSaadin.value);
    console.log("nappia painettu, arvo: ", arvo)////////////////////// TODO patch
    minimiKosteus = arvo; 
}

async function haeRaja() {
    try {
    const haettuRaja = await fetch(RAJA_URL);
    if (!haettuRaja.ok) throw new Error('Virhe: ' + haettuRaja.status);
    const data3 = await haettuRaja.json();

    if (typeof data3.minMoistureThreshold === 'number') {
        minimiKosteus = data3.minMoistureThreshold;
        rajaSaadin.value = minimiKosteus;
        naytaRaja();
    }} catch (err) {
        console.error('Rajan haku epäonnisuti.', err);
    }   
}

    // --- kaavioiden asetukset ---

function luoKaavio(canvasId = 'kaavio'){
  const ctx = document.getElementById(canvasId).getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Lämpötila (°C)',
          data: [],
          borderColor: 'red',
          backgroundColor: 'transparent',
          yAxisID: 'yLampo',
          tension: 0.3,
          pointRadius: 0,
          borderWidth: 2
        },
        {
          label: 'Ilmankosteus (%)',
          data: [],
          borderColor: 'blue',
          backgroundColor: 'transparent',
          yAxisID: 'yKosteus',
          tension: 0.3,
          pointRadius: 0,
          borderWidth: 2
        },
        {
          label: 'Maankosteus (%)',
          data: [],
          borderColor: 'green',
          backgroundColor: 'transparent',
          yAxisID: 'yKosteus',
          tension: 0.3,
          pointRadius: 0,
          borderWidth: 2
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: 'index', intersect: false },
      scales: {
        yLampo: { position: 'left' },
        yKosteus: { position: 'right', grid: { drawOnChartArea: false } }
      }
    }
  });
}

    // --- Datan haku ---
async function haeData(){ //Historia fets
  const historia = await fetch(HISTORIA_URL); 
  if(!historia.ok) throw new Error('Virhe: ' + historia.status);
  const data = await historia.json();
  console.log(data);
  return data;
}

async function haeLatest(){ //kaaviolatest fetch
  const latest = await fetch(LATEST_URL); 
  if(!latest.ok) throw new Error('Virhe: ' + latest.status);
  const data2 = await latest.json();
  console.log(data2);
  return data2; //Täällä voi tulla virhe kun saatu data ei ole taulukkona -> funktio joka muuttaa datan taulukoksi.
}

    // --- Kaavioiden päivitys ---
    // --- Värivaroitus erikseen lämpötilalle ja kosteudelle ---
function tarkistaRajaArvot(kaavio, rivit){
    if (!rivit || rivit.length === 0) return;

    const viimeinen = rivit[rivit.length - 1];

    const lampoYlitys = viimeinen.temp > MAX_LAMPO;
    const kosteusYlitys = viimeinen.airHumidity > MAX_KOSTEUS;
    const maaLiianKuiva = viimeinen.soilMoisture_percent < minimiKosteus;

    // Lämpötilaviiva
    kaavio.data.datasets[0].borderColor = lampoYlitys ? 'darkred' : 'red';

    // Kosteusviiva
    kaavio.data.datasets[1].borderColor = kosteusYlitys ? 'darkred' : 'blue';

    // Maankosteusviiva
    kaavio.data.datasets[2].borderColor = maaLiianKuiva ? 'orange'  : 'green';
    kaavio.update();
}

function paivitaKaavio(kaavio, rivit){
  kaavio.data.labels = rivit.map(r => new Date(r.timestamp).toLocaleTimeString('fi-FI', { hour: '2-digit', minute: '2-digit' }));
  kaavio.data.datasets[0].data = rivit.map(r => r.temp);
  kaavio.data.datasets[1].data = rivit.map(r => r.airHumidity);
  kaavio.data.datasets[2].data = rivit.map(r => r.soilMoisture_percent);
  kaavio.update();
  // automaattinen raja-arvotarkistus
  tarkistaRajaArvot(kaavio, rivit); 
}

// --- Automaattinen päivitys vain kun välilehti näkyvissä ---
let autoTimer = null;

function startAutoUpdate() {
    if (autoTimer === null) {
        autoTimer = setInterval(() => {
            haeLatest()
                .then(rivit => paivitaKaavio(kaavioLatest, rivit))
                .catch(console.error);
        }, 5000); // 5 sekuntia
        console.log("Automaattinen päivitys käynnistetty");
    }
}

function stopAutoUpdate() {
    if (autoTimer !== null) {
        clearInterval(autoTimer);
        autoTimer = null;
        console.log("Automaattinen päivitys pysäytetty");
    }
}

// --- Käynnistys ---
const kaavio = luoKaavio('kaavio');
haeData().then(rivit => paivitaKaavio(kaavio, rivit)).catch(console.error);

const kaavioLatest = luoKaavio('kaavioLatest');
haeLatest().then(rivit => paivitaKaavio(kaavioLatest, rivit)).catch(console.error);

rajaSaadin.addEventListener('input', naytaRaja);
rajaNappi.addEventListener('click', lahetaRaja);
naytaRaja();
// haeRaja(); kun raja_url on toimiva

document.addEventListener("visibilitychange", () => {
    if (document.visibilityState === "visible") {
        startAutoUpdate();
    } else {
        stopAutoUpdate();
    }
});

// Käynnistä päivitys kun sivu avataan
startAutoUpdate();

