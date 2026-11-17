// === 1. Asetukset ja API-osoitteet ===
const HISTORIA_URL = 'http://localhost:3000/api/measurements';
const RAJA_URL = 'http://localhost:3000/api/settings';

// === 2. Sovelluksen tila ===
let measurements = [];
let minimiKosteus = 49;

// === 3. Sivun HTML-elementit ===
const kosteusRajaSaadin = document.getElementById('kosteusRajaSaadin');
const rajaArvo = document.getElementById('rajaArvo');
const rajaNappi = document.getElementById('rajaTallenna');

// --- Kaavioiden valikot ---
const mittausValinta = document.getElementById('mittausValinta');
const aikavaliValinta = document.getElementById('aikavaliValinta');

// === 4. Datan haku ===
async function haeData() { //Historia fets
  const historia = await fetch(HISTORIA_URL);
  if (!historia.ok) throw new Error('Virhe: ' + historia.status);
  const data = await historia.json();
  console.log(data);
  return data;
}

async function haeRaja() {
  try {
    const haettuRaja = await fetch(RAJA_URL);
    if (!haettuRaja.ok) throw new Error('Virhe: ' + haettuRaja.status);
    const data3 = await haettuRaja.json();

    if (typeof data3.minMoistureThreshold === 'number') {
      minimiKosteus = data3.minMoistureThreshold;
      kosteusRajaSaadin.value = minimiKosteus;
      naytaRaja();
    }
  } catch (err) {
    console.error('Rajan haku epäonnistui.', err);
  }
}

// === 5. Raja-arvosäätimen toiminnot ===
function naytaRaja() {
  rajaArvo.textContent = kosteusRajaSaadin.value + ' %';
}

async function lahetaRaja() {
  try {
    const arvo = Number(kosteusRajaSaadin.value);
    const response = await fetch(RAJA_URL, {
      method: 'PATCH',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        "minMoistureThreshold": arvo
      })
    });
    if (!response.ok) {
      throw new Error('Virhe: ' + response.status);
    }
    minimiKosteus = arvo;
    console.log(`Min kosteus: ${minimiKosteus}`);
  } catch (err) {
    console.error('Rajan muokkaminen epäonnistui', err);
  }
}

// === 6. Kaavion asetukset ja luonti ===
function luoKaavio(canvasId) {
  const ctx = document.getElementById(canvasId).getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Mullan kosteus (%)',
          data: [],
          borderColor: 'green',
          backgroundColor: 'transparent',
          yAxisID: 'yArvo',
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
        x: {
          ticks: {
            autoSkip: true,
            maxTicksLimit: 10,
            maxRotation: 0
          }
        },
        yArvo: {
          position: 'left'
        }
      }
    }
  });
}

// === 7. Kaavion päivitys ja historian suodatus ===
function paivitaKaavio(kaavio, rivit) {
  let dateOptions;

  switch (aikavaliValinta.value) {
    case 'paiva':
      dateOptions = {
        hour: '2-digit',
        minute: '2-digit'
      };
      break;

    case 'kuukausi':
      dateOptions = {
        day: '2-digit',
        month: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      };
      break;

    case 'vuosi':
      dateOptions = {
        day: '2-digit',
        month: '2-digit'
      };
      break;

    case 'kaikki':
      dateOptions = {
        day: '2-digit',
        month: '2-digit',
        year: 'numeric'
      };
      break;
  }

  kaavio.data.labels = rivit.map(r =>
    new Date(r.timestamp).toLocaleString('fi-FI', dateOptions)
  );
  const selectedMetric = mittausValinta.value;
  const selectedLabel = mittausValinta.options[mittausValinta.selectedIndex].textContent;
  const colors = {
    soilMoisture_percent: 'green',
    temp: 'red',
    airHumidity: 'blue',
    soilMoisture_adc: 'purple'
  };
  kaavio.data.datasets[0].borderColor = colors[selectedMetric] || 'black';
  kaavio.data.datasets[0].label = selectedLabel;
  kaavio.data.datasets[0].data = rivit.map(r => r[selectedMetric]);
  kaavio.update();
}

function paivitaHistoria() {
  const now = new Date();
  let startDate;

  switch (aikavaliValinta.value) {
    case 'paiva':
      startDate = new Date(now.getFullYear(), now.getMonth(), now.getDate());
      break;

    case 'kuukausi':
      startDate = new Date(now.getFullYear(), now.getMonth(), 1);
      break;

    case 'vuosi':
      startDate = new Date(now.getFullYear(), 0, 1);
      break;

    case 'kaikki':
      startDate = null;
      break;
  }

  const filteredMeasurements = startDate
    ? measurements.filter(m => new Date(m.timestamp) >= startDate)
    : measurements;

  paivitaKaavio(historiaKaavio, filteredMeasurements);
}

// === 8. Mittauskorttien päivitys ===
function paivitaMittausKortit(measurement) {
  document.getElementById("arvoLampo").textContent = `${measurement.temp} °C`;
  document.getElementById("arvoIlmanKosteus").textContent = `${measurement.airHumidity} %`;
  document.getElementById("arvoMullanKosteus").textContent = `${measurement.soilMoisture_percent} %`;
}

// === 9. Käynnistys ja tapahtumankuuntelijat ===
// --- SSE (Server-Sent Events) ---
function aloitaSSE() {
  const eventSource = new EventSource(
    'http://localhost:3000/api/measurements/stream'
  );
  eventSource.addEventListener('measurement', (event) => {
    const data = JSON.parse(event.data);
    console.log(data);
    measurements.push(data);
    paivitaHistoria();
    paivitaMittausKortit(data);
  });
}

// --- Historian lataus ---
haeData()
  .then(rivit => {
    measurements = rivit;
    paivitaHistoria();
    const lastMeasurement = rivit.at(-1);
    if (lastMeasurement) {
      paivitaMittausKortit(lastMeasurement);
    }
  })
  .catch(console.error)
  .finally(aloitaSSE);

// --- Kaavion luonti ---
const historiaKaavio = luoKaavio('historiaKaavio');

// --- Raja-arvosäätimen tapahtumat ---
kosteusRajaSaadin.addEventListener('input', naytaRaja);
rajaNappi.addEventListener('click', lahetaRaja);
naytaRaja();
haeRaja();

// --- Kaavion valikoiden tapahtumat ---
mittausValinta.addEventListener('change', paivitaHistoria);

aikavaliValinta.addEventListener('change', paivitaHistoria);
