/*
 * PRUTHVI Home Automation
 * Frontend → Wi-Fi → ESP8266
 *
 * ESP8266 API:
 *   GET  /api/status
 *   POST /api/device
 *
 * Current ESP8266 IP:
 *   10.62.156.207
 */

const API_BASE =
  window.PRUTHVI_API_BASE || "http://10.62.156.207";

const cards = [...document.querySelectorAll(".device")];

const state = {
  led: false,
  bulb1: false,
  bulb2: false
};

let toastTimer;


/* =========================
   TOAST
========================= */

function toast(message) {
  const box = document.getElementById("toast");

  if (!box) return;

  const span = box.querySelector("span");

  if (span) {
    span.textContent = message;
  }

  box.classList.add("show");

  clearTimeout(toastTimer);

  toastTimer = setTimeout(() => {
    box.classList.remove("show");
  }, 2200);
}


/* =========================
   RENDER UI
========================= */

function render() {
  let on = 0;

  cards.forEach(card => {
    const id = card.dataset.id;
    const active = !!state[id];

    if (active) {
      on++;
    }

    card.classList.toggle("active", active);

    const badge = card.querySelector(".badge");
    const badgeText = card.querySelector(".badge b");
    const power = card.querySelector(".power span");
    const onButton = card.querySelector(".on");
    const offButton = card.querySelector(".off");

    if (badge) {
      badge.classList.toggle("on", active);
    }

    if (badgeText) {
      badgeText.textContent = active ? "ON" : "OFF";
    }

    if (power) {
      power.textContent = active ? "100%" : "0%";
    }

    if (onButton) {
      onButton.classList.toggle("selected", active);
    }

    if (offButton) {
      offButton.classList.toggle("selected", !active);
    }
  });

  const counter = document.getElementById("counter");

  if (counter) {
    counter.textContent = `${on} / ${cards.length}`;
  }
}


/* =========================
   ACTIVITY
========================= */

function activity(name, action) {
  const list = document.getElementById("activity");

  if (!list) return;

  const empty = list.querySelector(".empty");

  if (empty) {
    empty.remove();
  }

  const row = document.createElement("div");

  row.className = "activity-item";

  const time = new Date().toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit"
  });

  row.innerHTML = `
    <span class="activity-icon">
      ${action === "ON" ? "✓" : "○"}
    </span>

    <div>
      <b>${name} ${action}</b>
      <small>${time}</small>
    </div>
  `;

  list.prepend(row);
}


/* =========================
   GET ESP8266 STATUS
========================= */

async function loadStatus() {
  try {
    const response = await fetch(
      `${API_BASE}/api/status`,
      {
        method: "GET",
        cache: "no-store"
      }
    );

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const data = await response.json();

    state.led = String(data.led).toUpperCase() === "ON";
    state.bulb1 = String(data.bulb1).toUpperCase() === "ON";
    state.bulb2 = String(data.bulb2).toUpperCase() === "ON";

    render();

    console.log("PRUTHVI ESP8266 status:", data);

    return true;

  } catch (error) {
    console.error("ESP8266 status error:", error);

    toast("PRUTHVI ESP8266 unavailable");

    return false;
  }
}


/* =========================
   DEVICE COMMAND
========================= */

async function command(card, on) {
  const id = card.dataset.id;
  const nameElement = card.querySelector("h3");

  const name = nameElement
    ? nameElement.textContent
    : id;

  const newState = on ? "ON" : "OFF";

  try {

    /*
     * ESP8266 expects:
     *
     * device=led&state=ON
     *
     * NOT JSON.
     */

    const body = new URLSearchParams();

    body.append("device", id);
    body.append("state", newState);

    const response = await fetch(
      `${API_BASE}/api/device`,
      {
        method: "POST",

        headers: {
          "Content-Type":
            "application/x-www-form-urlencoded"
        },

        body: body.toString()
      }
    );

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    /*
     * Update UI only after ESP8266
     * successfully accepts the command.
     */

    state[id] = on;

    render();

    activity(name, newState);

    toast(`${name} ${newState}`);

  } catch (error) {

    console.error(
      "PRUTHVI device command failed:",
      error
    );

    toast(
      `Cannot connect to PRUTHVI ESP8266`
    );
  }
}


/* =========================
   DEVICE BUTTONS
========================= */

cards.forEach(card => {

  const onButton = card.querySelector(".on");
  const offButton = card.querySelector(".off");

  if (onButton) {
    onButton.addEventListener(
      "click",
      () => command(card, true)
    );
  }

  if (offButton) {
    offButton.addEventListener(
      "click",
      () => command(card, false)
    );
  }

});


/* =========================
   ALL ON
========================= */

document
  .getElementById("allOn")
  ?.addEventListener("click", async () => {

    for (const card of cards) {
      await command(card, true);
    }

    toast("All devices ON");
  });


/* =========================
   ALL OFF
========================= */

document
  .getElementById("allOff")
  ?.addEventListener("click", async () => {

    for (const card of cards) {
      await command(card, false);
    }

    toast("All devices OFF");
  });


/* =========================
   CLEAR ACTIVITY
========================= */

document
  .getElementById("clear")
  ?.addEventListener("click", () => {

    const activityBox =
      document.getElementById("activity");

    if (activityBox) {
      activityBox.innerHTML =
        '<div class="empty">◎<span>No actions yet</span></div>';
    }

    toast("Activity cleared");
  });


/* =========================
   REFRESH
========================= */

document
  .getElementById("refresh")
  ?.addEventListener("click", async () => {

    toast("Refreshing PRUTHVI...");

    await loadStatus();

    toast("PRUTHVI status updated");
  });


/* =========================
   UI RESET
========================= */

document
  .getElementById("reset")
  ?.addEventListener("click", async () => {

    for (const card of cards) {
      await command(card, false);
    }

    toast("System OFF");
  });


/* =========================
   DEMO
========================= */

document
  .getElementById("demo")
  ?.addEventListener("click", async () => {

    toast("Demo started");

    /*
     * Demo now controls the real ESP8266.
     */

    for (const card of cards) {
      await command(card, true);

      await new Promise(resolve =>
        setTimeout(resolve, 350)
      );
    }

    await new Promise(resolve =>
      setTimeout(resolve, 400)
    );

    for (const card of cards) {
      await command(card, false);

      await new Promise(resolve =>
        setTimeout(resolve, 350)
      );
    }

    activity(
      "Demo sequence",
      "OFF"
    );

    toast("Demo complete");
  });


/* =========================
   THEME
========================= */

document
  .getElementById("themeButton")
  ?.addEventListener("click", () => {

    document.body.classList.toggle("light");

    const light =
      document.body.classList.contains("light");

    const button =
      document.getElementById("themeButton");

    if (button) {
      button.textContent =
        light ? "☀" : "☾";
    }

    localStorage.setItem(
      "pruthvi-theme",
      light ? "light" : "dark"
    );
  });


/* =========================
   LOAD SAVED THEME
========================= */

if (
  localStorage.getItem("pruthvi-theme")
  === "light"
) {

  document.body.classList.add("light");

  const button =
    document.getElementById("themeButton");

  if (button) {
    button.textContent = "☀";
  }
}


/* =========================
   START PRUTHVI
========================= */

render();

/*
 * Read actual ESP8266 state
 * when dashboard starts.
 */

loadStatus();
