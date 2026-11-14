const viewInfo = {
  overview: ["Overview"],
  menu: ["Food Menu"],
  orders: ["Orders (FIFO Queue)"],
  recipes: ["Recipes & Demand"],
  inventory: ["Inventory & FEFO"],
  reports: ["Reports & DSA Map"]
};

let offlineDemo = false;
let state = {
  today: 20261114,
  foods: [
    { id: 1, name: "Paneer_Tikka", price: 240 },
    { id: 2, name: "Chicken_Biryani", price: 320 },
    { id: 3, name: "Dal_Makhani", price: 190 },
    { id: 4, name: "Butter_Naan", price: 45 }
  ],
  orders: [
    { id: 1001, status: "CONFIRMED", date: 20261114, items: "Chicken_Biryani x2, Butter_Naan x4", total: 820 },
    { id: 1002, status: "PENDING", date: 20261114, items: "Paneer_Tikka x1, Dal_Makhani x1", total: 430 }
  ],
  ingredients: [
    { id: 1, name: "Basmati_Rice", unit: "g", usable: 4500, min: 2000, cost: 120 },
    { id: 2, name: "Paneer", unit: "g", usable: 1200, min: 1000, cost: 380 },
    { id: 3, name: "Chicken", unit: "g", usable: 2800, min: 1500, cost: 280 }
  ],
  batches: [
    { id: 101, ingId: 2, ingName: "Paneer", qty: 300, expiry: 20261101 },
    { id: 102, ingId: 2, ingName: "Paneer", qty: 1200, expiry: 20261118 },
    { id: 103, ingId: 1, ingName: "Basmati_Rice", qty: 4500, expiry: 20261220 }
  ],
  demand: [
    { ingId: 1, name: "Basmati_Rice", unit: "g", needed: 500, usable: 4500 },
    { ingId: 3, name: "Chicken", unit: "g", needed: 600, usable: 2800 }
  ],
  transactions: [
    { type: "RECEIVED", ingId: 2, qty: 1200, date: 20261110, note: "Fresh batch" }
  ]
};

const $ = (id) => document.getElementById(id);

function showAlert(msg, ok = true) {
  const el = $("alert");
  el.textContent = msg;
  el.className = "alert " + (ok ? "ok" : "err");
}

async function postApi(url) {
  const res = await fetch(url, { method: "POST" });
  const data = await res.json();
  if (!data.ok) throw new Error(data.error || "Operation failed");
  return data;
}

function render() {
  $("today-label").textContent = "Today: " + state.today;
  $("stat-foods").textContent = state.foods.length;
  $("stat-pending").textContent = state.orders.filter(o => o.status === "PENDING").length;

  const expBatches = state.batches.filter(b => b.qty > 0 && b.expiry <= state.today + 7);
  $("stat-expiring").textContent = expBatches.length;

  const salesTotal = state.orders.filter(o => o.status === "CONFIRMED").reduce((s, o) => s + o.total, 0);
  $("stat-sales").textContent = "Rs. " + salesTotal;

  $("overview-batches").innerHTML = state.batches.filter(b => b.qty > 0).map(b => {
    const expired = b.expiry < state.today;
    const badge = expired ? `<span class="pill pill-bad">EXPIRED</span>` : `<span class="pill pill-ok">USABLE</span>`;
    return `<tr><td>#${b.id}</td><td>${b.ingName}</td><td>${b.qty}</td><td>${b.expiry}</td><td>${badge}</td></tr>`;
  }).join("");

  $("menu-body").innerHTML = state.foods.map(f => `
    <tr>
      <td>${f.id}</td>
      <td>${f.name}</td>
      <td>Rs. ${f.price}</td>
      <td><button class="btn-primary btn-sm" onclick="quickOrder(${f.id})">+ Order 1</button></td>
    </tr>
  `).join("");

  $("orders-body").innerHTML = state.orders.map(o => {
    const cls = o.status === "CONFIRMED" ? "pill-ok" : (o.status === "PENDING" ? "pill-warn" : "pill-bad");
    const btns = o.status === "PENDING"
      ? `<button class="btn-primary btn-sm" onclick="orderAct(${o.id},'confirm')">Confirm</button>
         <button class="btn-danger btn-sm" onclick="orderAct(${o.id},'cancel')">Cancel</button>`
      : "—";
    return `<tr><td>#${o.id}</td><td>${o.date}</td><td>${o.items}</td><td>Rs. ${o.total}</td>
            <td><span class="pill ${cls}">${o.status}</span></td><td>${btns}</td></tr>`;
  }).join("");

  $("demand-body").innerHTML = state.demand.map(d => {
    const ok = d.usable >= d.needed;
    return `<tr><td>${d.name}</td><td>${d.needed} ${d.unit}</td><td>${d.usable} ${d.unit}</td>
            <td><span class="pill ${ok ? "pill-ok" : "pill-bad"}">${ok ? "OK" : "LOW"}</span></td></tr>`;
  }).join("");

  $("ing-body").innerHTML = state.ingredients.map(g => {
    const low = g.usable < g.min;
    return `<tr><td>${g.id}</td><td>${g.name}</td><td>${g.usable} ${g.unit}</td><td>${g.min} ${g.unit}</td>
            <td>Rs. ${g.cost}</td><td><span class="pill ${low ? "pill-bad" : "pill-ok"}">${low ? "LOW STOCK" : "OK"}</span></td></tr>`;
  }).join("");

  const foodSales = state.foods.map(f => {
    let plates = 0;
    state.orders.filter(o => o.status === "CONFIRMED").forEach(o => {
      if (o.items.includes(f.name)) plates += 1;
    });
    return { name: f.name, plates, amount: plates * f.price };
  }).sort((a, b) => b.amount - a.amount);

  $("sales-body").innerHTML = foodSales.map((s, i) =>
    `<tr><td>#${i + 1}</td><td>${s.name}</td><td>${s.plates}</td><td>Rs. ${s.amount}</td></tr>`
  ).join("");

  $("txn-body").innerHTML = state.transactions.slice().reverse().map(t =>
    `<tr><td>${t.type}</td><td>${t.ingId}</td><td>${t.qty}</td><td>${t.date}</td><td>${t.note}</td></tr>`
  ).join("");
}

async function sync() {
  try {
    const res = await fetch("/api/state");
    const data = await res.json();
    if (data.ok) {
      state = data;
      offlineDemo = false;
      $("conn-mode").textContent = "Connected to C++ Server";
    }
  } catch (e) {
    offlineDemo = true;
    $("conn-mode").textContent = "Browser Preview Mode";
  }
  render();
}

$("login-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const u = $("login-user").value.trim();
  const p = $("login-pass").value.trim();
  const demoUsers = { admin: "ADMIN", staff: "STAFF", stock: "STOCK_FILLER" };

  try {
    const res = await fetch(`/api/login?user=${encodeURIComponent(u)}&pass=${encodeURIComponent(p)}`, { method: "POST" });
    const data = await res.json();
    if (!data.ok) throw new Error(data.error);
    $("user-badge").textContent = "Role: " + data.role;
    $("login-modal").classList.add("hidden");
    sync();
  } catch (err) {
    if (demoUsers[u]) {
      $("user-badge").textContent = "Role: " + demoUsers[u];
      $("login-modal").classList.add("hidden");
      sync();
    } else {
      $("login-err").textContent = "Invalid credentials (try admin / admin123)";
      $("login-err").classList.remove("hidden");
    }
  }
});

$("logout-btn").addEventListener("click", () => $("login-modal").classList.remove("hidden"));

window.quickOrder = async (foodId) => {
  if (offlineDemo) {
    const f = state.foods.find(x => x.id === foodId);
    state.orders.push({ id: 1000 + state.orders.length + 1, status: "PENDING", date: state.today, items: f.name + " x1", total: f.price });
    showAlert("Order added to Circular Queue!");
    return render();
  }
  await postApi(`/api/order/place?foodId=${foodId}&qty=1`);
  showAlert("Order placed in C++ Circular Queue!");
  sync();
};

window.orderAct = async (id, act) => {
  if (offlineDemo) {
    const o = state.orders.find(x => x.id === id);
    if (o) o.status = act === "confirm" ? "CONFIRMED" : "CANCELLED";
    showAlert("Order updated!");
    return render();
  }
  await postApi(`/api/order/action?id=${id}&act=${act}`);
  showAlert("Order " + act + "ed in C++!");
  sync();
};

$("confirm-next-btn").addEventListener("click", async () => {
  if (offlineDemo) {
    const next = state.orders.find(o => o.status === "PENDING");
    if (!next) return showAlert("No pending orders.", false);
    next.status = "CONFIRMED";
    showAlert("Confirmed Order #" + next.id + " via FIFO Queue!");
    return render();
  }
  try {
    await postApi("/api/order/confirmNext");
    showAlert("Confirmed next order from Circular Queue!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("add-food-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const id = +$("f-id").value, name = $("f-name").value.trim(), price = +$("f-price").value;
  if (offlineDemo) { state.foods.push({ id, name, price }); showAlert("Food added!"); return render(); }
  await postApi(`/api/food/add?id=${id}&name=${encodeURIComponent(name)}&price=${price}`);
  showAlert("Food added & saved to foods.txt!");
  sync();
});

$("add-recipe-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const fid = +$("r-fid").value, gid = +$("r-gid").value, qty = +$("r-qty").value;
  if (offlineDemo) { showAlert("Recipe saved!"); return; }
  await postApi(`/api/recipe/add?foodId=${fid}&ingId=${gid}&qty=${qty}`);
  showAlert("Recipe saved to recipes.txt!");
  sync();
});

$("receive-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const bid = +$("b-id").value, gid = +$("b-gid").value, qty = +$("b-qty").value, exp = +$("b-exp").value;
  if (offlineDemo) { showAlert("Batch received!"); return; }
  try {
    await postApi(`/api/stock/receive?batchId=${bid}&ingId=${gid}&qty=${qty}&expiry=${exp}`);
    showAlert("Batch added & pushed to Undo Stack!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("use-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const gid = +$("u-gid").value, qty = +$("u-qty").value;
  if (offlineDemo) { showAlert("Stock used with FEFO Min-Heap!"); return; }
  try {
    await postApi(`/api/stock/use?ingId=${gid}&qty=${qty}`);
    showAlert("Stock deducted using FEFO Min-Heap!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("waste-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const gid = +$("w-gid").value, qty = +$("w-qty").value, note = $("w-note").value.trim();
  if (offlineDemo) { showAlert("Wastage logged!"); return; }
  try {
    await postApi(`/api/stock/waste?ingId=${gid}&qty=${qty}&note=${encodeURIComponent(note)}`);
    showAlert("Wastage recorded!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("expire-btn").addEventListener("click", async () => {
  if (offlineDemo) return showAlert("Expired batches written off!");
  await postApi("/api/stock/expire");
  showAlert("Expired batches checked and written off!");
  sync();
});

$("undo-btn").addEventListener("click", async () => {
  if (offlineDemo) return showAlert("Undid last stock receipt!");
  try {
    await postApi("/api/stock/undo");
    showAlert("Last stock receipt undone via LIFO Stack!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

document.querySelectorAll(".nav-btn").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".nav-btn").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    const v = btn.dataset.view;
    document.querySelectorAll(".view").forEach(sec => sec.classList.toggle("hidden", sec.id !== "view-" + v));
    $("page-title").textContent = viewInfo[v][0];
    $("page-sub").textContent = viewInfo[v][1];
  });
});

$("refresh-btn").addEventListener("click", () => { sync(); showAlert("Data synced!"); });
sync();
