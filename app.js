const viewInfo = {
  overview: ["Overview", "Live restaurant inventory, batch expiry, and order queue status."],
  menu: ["Food Menu (Features 1, 2 & 3)", "Dishes kept sorted by ID so they can be found with binary search."],
  orders: ["Orders Queue (Features 4, 5, 6, 7 & 8)", "Place, confirm, and cancel orders in Circular FIFO Queue with unexpired stock checks."],
  recipes: ["Recipes & Demand (Features 9, 10, 11, 12 & 13)", "Complete recipe book, demand calculation, and daily demand forecasting."],
  inventory: ["Inventory & FEFO (Features 14, 15, 16, 17, 18, 19, 20 & 21)", "Ingredient stocks, batch receiving, Min-Heap FEFO deduction, and wastage tracking."],
  reports: ["Reports & Persistence (Features 22, 23 & 24)", "All 6 console analytical reports and 2-way disk text file persistence."]
};

const API = location.protocol === "file:" ? "http://localhost:8080" : "";

let offlineDemo = false;
let currentUserRole = "ADMIN";

const rolePermissions = {
  ADMIN: {
    title: "Administrator",
    views: ["overview", "menu", "orders", "recipes", "inventory", "reports"],
    canAddFood: true,
    canUpdateFood: true,
    canOrder: true,
    canManageOrders: true,
    canManageRecipes: true,
    canReceiveStock: true,
    canUseStock: true,
    canWasteStock: true,
    canExpireStock: true,
    canUndoStock: true
  },
  STAFF: {
    title: "Staff Member",
    views: ["overview", "menu", "orders", "recipes", "reports"],
    canAddFood: true,
    canUpdateFood: false,
    canOrder: true,
    canManageOrders: true,
    canManageRecipes: true,
    canReceiveStock: false,
    canUseStock: false,
    canWasteStock: false,
    canExpireStock: false,
    canUndoStock: false
  },
  STOCK_FILLER: {
    title: "Stock Filler",
    views: ["overview", "menu", "inventory", "reports"],
    canAddFood: false,
    canUpdateFood: false,
    canOrder: false,
    canManageOrders: false,
    canManageRecipes: false,
    canReceiveStock: true,
    canUseStock: true,
    canWasteStock: true,
    canExpireStock: true,
    canUndoStock: false
  }
};

// Initial state synchronized directly with data/*.txt files
let state = {
  today: 20261118,
  foods: [
    { id: 1, name: "Paneer_Tikka", price: 240 },
    { id: 2, name: "Chicken_Biryani", price: 320 },
    { id: 3, name: "Dal_Makhani", price: 190 },
    { id: 4, name: "Butter_Naan", price: 45 },
    { id: 5, name: "Masala_Dosa", price: 140 },
    { id: 6, name: "Shahi_Paneer", price: 250 },
    { id: 7, name: "khadhi_Paneer", price: 250 },
    { id: 8, name: "malai_dal", price: 250 },
    { id: 9, name: "Mutton_Rogan_Josh", price: 420 },
    { id: 10, name: "Tandoori_Chicken", price: 350 },
    { id: 11, name: "Palak_Paneer", price: 220 },
    { id: 12, name: "Garlic_Naan", price: 60 },
    { id: 13, name: "Jeera_Rice", price: 150 },
    { id: 14, name: "Veg_Biryani", price: 210 },
    { id: 15, name: "Fish_Curry", price: 380 },
    { id: 16, name: "Prawn_Masala", price: 450 },
    { id: 17, name: "Chicken_Tikka_Masala", price: 340 },
    { id: 18, name: "Aloo_Gobi", price: 160 },
    { id: 19, name: "Chole_Bhature", price: 180 },
    { id: 20, name: "Gulab_Jamun", price: 90 },
    { id: 21, name: "Rasmalai", price: 110 },
    { id: 22, name: "Kadhai_Chicken", price: 330 },
    { id: 23, name: "Lassi_Sweet", price: 70 },
    { id: 24, name: "Mango_Lassi", price: 90 },
    { id: 25, name: "Paneer_Butter_Masala", price: 260 },
    { id: 26, name: "Egg_Curry", price: 180 },
    { id: 27, name: "Veg_Spring_Roll", price: 150 },
    { id: 28, name: "Chicken_Soup", price: 140 },
    { id: 29, name: "Raita_Boondi", price: 80 },
    { id: 30, name: "Kheer", price: 100 }
  ],
  ingredients: [
    { id: 1, name: "Basmati_Rice", unit: "g", usable: 5000, min: 2000, cost: 120 },
    { id: 2, name: "Paneer", unit: "g", usable: 2500, min: 1000, cost: 380 },
    { id: 3, name: "Chicken", unit: "g", usable: 3500, min: 1500, cost: 280 },
    { id: 4, name: "Butter", unit: "g", usable: 1500, min: 500, cost: 500 },
    { id: 5, name: "Black_Lentils", unit: "g", usable: 2500, min: 1000, cost: 140 },
    { id: 6, name: "capcisum", unit: "g", usable: 2500, min: 2000, cost: 200 },
    { id: 7, name: "Mutton", unit: "g", usable: 2000, min: 1500, cost: 650 },
    { id: 8, name: "Fish_Fillet", unit: "g", usable: 0, min: 1000, cost: 450 }, // Batch 109 is EXPIRED (20261015)
    { id: 9, name: "Prawns", unit: "g", usable: 1500, min: 800, cost: 700 },
    { id: 10, name: "Wheat_Flour", unit: "g", usable: 8000, min: 5000, cost: 40 },
    { id: 11, name: "Garlic", unit: "g", usable: 1000, min: 500, cost: 180 },
    { id: 12, name: "Spinach", unit: "g", usable: 1500, min: 1000, cost: 60 },
    { id: 13, name: "Cumin_Seeds", unit: "g", usable: 800, min: 500, cost: 300 },
    { id: 14, name: "Potatoes", unit: "g", usable: 4000, min: 3000, cost: 30 },
    { id: 15, name: "Cauliflower", unit: "g", usable: 3000, min: 2000, cost: 45 },
    { id: 16, name: "Chickpeas", unit: "g", usable: 3500, min: 2000, cost: 90 },
    { id: 17, name: "Milk", unit: "ml", usable: 6000, min: 5000, cost: 60 },
    { id: 18, name: "Sugar", unit: "g", usable: 5000, min: 3000, cost: 45 },
    { id: 19, name: "Yogurt", unit: "g", usable: 4000, min: 3000, cost: 70 },
    { id: 20, name: "Eggs", unit: "pcs", usable: 60, min: 30, cost: 8 },
    { id: 21, name: "Tomatoes", unit: "g", usable: 4500, min: 3000, cost: 40 },
    { id: 22, name: "Onions", unit: "g", usable: 5000, min: 4000, cost: 35 },
    { id: 23, name: "Cooking_Oil", unit: "ml", usable: 6000, min: 5000, cost: 150 },
    { id: 24, name: "Cream", unit: "g", usable: 1800, min: 1000, cost: 220 },
    { id: 25, name: "Green_Chili", unit: "g", usable: 800, min: 500, cost: 80 },
    { id: 26, name: "Ginger", unit: "g", usable: 800, min: 500, cost: 120 },
    { id: 27, name: "Mango_Pulp", unit: "g", usable: 1500, min: 1000, cost: 160 },
    { id: 28, name: "Cardamom", unit: "g", usable: 300, min: 200, cost: 1800 },
    { id: 29, name: "Spring_Roll_Sheets", unit: "pcs", usable: 80, min: 50, cost: 5 },
    { id: 30, name: "Coriander", unit: "g", usable: 800, min: 500, cost: 50 }
  ],
  batches: [
    { id: 101, ingId: 2, ingName: "Paneer", qty: 300, expiry: 20261101 }, // Expired
    { id: 102, ingId: 2, ingName: "Paneer", qty: 2500, expiry: 20270315 },
    { id: 103, ingId: 1, ingName: "Basmati_Rice", qty: 5000, expiry: 20270620 },
    { id: 104, ingId: 3, ingName: "Chicken", qty: 3500, expiry: 20270410 },
    { id: 105, ingId: 4, ingName: "Butter", qty: 1500, expiry: 20270225 },
    { id: 106, ingId: 5, ingName: "Black_Lentils", qty: 2500, expiry: 20270510 },
    { id: 107, ingId: 6, ingName: "capcisum", qty: 2500, expiry: 20270115 },
    { id: 108, ingId: 7, ingName: "Mutton", qty: 2000, expiry: 20270220 },
    { id: 109, ingId: 8, ingName: "Fish_Fillet", qty: 400, expiry: 20261015 }, // Expired - Cannot be used
    { id: 110, ingId: 9, ingName: "Prawns", qty: 1500, expiry: 20270301 },
    { id: 111, ingId: 10, ingName: "Wheat_Flour", qty: 8000, expiry: 20270815 },
    { id: 112, ingId: 11, ingName: "Garlic", qty: 1000, expiry: 20270120 },
    { id: 113, ingId: 12, ingName: "Spinach", qty: 1500, expiry: 20270110 },
    { id: 114, ingId: 13, ingName: "Cumin_Seeds", qty: 800, expiry: 20270920 },
    { id: 115, ingId: 14, ingName: "Potatoes", qty: 4000, expiry: 20270215 },
    { id: 116, ingId: 15, ingName: "Cauliflower", qty: 3000, expiry: 20270125 },
    { id: 117, ingId: 16, ingName: "Chickpeas", qty: 3500, expiry: 20270710 },
    { id: 118, ingId: 17, ingName: "Milk", qty: 6000, expiry: 20261122 },
    { id: 119, ingId: 18, ingName: "Sugar", qty: 5000, expiry: 20271230 },
    { id: 120, ingId: 19, ingName: "Yogurt", qty: 4000, expiry: 20270105 },
    { id: 121, ingId: 20, ingName: "Eggs", qty: 60, expiry: 20261124 },
    { id: 122, ingId: 21, ingName: "Tomatoes", qty: 4500, expiry: 20261123 },
    { id: 123, ingId: 22, ingName: "Onions", qty: 5000, expiry: 20270310 },
    { id: 124, ingId: 23, ingName: "Cooking_Oil", qty: 6000, expiry: 20271115 },
    { id: 125, ingId: 24, ingName: "Cream", qty: 1800, expiry: 20270118 },
    { id: 126, ingId: 25, ingName: "Green_Chili", qty: 800, expiry: 20261120 },
    { id: 127, ingId: 26, ingName: "Ginger", qty: 800, expiry: 20270205 },
    { id: 128, ingId: 27, ingName: "Mango_Pulp", qty: 1500, expiry: 20270615 },
    { id: 129, ingId: 28, ingName: "Cardamom", qty: 300, expiry: 20271225 },
    { id: 130, ingId: 29, ingName: "Spring_Roll_Sheets", qty: 80, expiry: 20270420 },
    { id: 131, ingId: 30, ingName: "Coriander", qty: 800, expiry: 20261121 },
    { id: 132, ingId: 7, ingName: "Mutton", qty: 300, expiry: 20261105 } // Expired
  ],
  recipes: [
    { foodId: 1, ingId: 2, qty: 200 }, { foodId: 1, ingId: 4, qty: 30 }, { foodId: 1, ingId: 6, qty: 50 }, { foodId: 1, ingId: 19, qty: 50 },
    { foodId: 2, ingId: 1, qty: 250 }, { foodId: 2, ingId: 3, qty: 300 }, { foodId: 2, ingId: 22, qty: 80 }, { foodId: 2, ingId: 23, qty: 40 },
    { foodId: 3, ingId: 5, qty: 150 }, { foodId: 3, ingId: 4, qty: 40 }, { foodId: 3, ingId: 24, qty: 30 },
    { foodId: 4, ingId: 10, qty: 120 }, { foodId: 4, ingId: 4, qty: 25 }, { foodId: 4, ingId: 17, qty: 30 },
    { foodId: 5, ingId: 1, qty: 100 }, { foodId: 5, ingId: 14, qty: 120 }, { foodId: 5, ingId: 22, qty: 30 }, { foodId: 5, ingId: 23, qty: 20 },
    { foodId: 6, ingId: 2, qty: 180 }, { foodId: 6, ingId: 21, qty: 100 }, { foodId: 6, ingId: 24, qty: 40 }, { foodId: 6, ingId: 4, qty: 30 },
    { foodId: 7, ingId: 2, qty: 180 }, { foodId: 7, ingId: 6, qty: 60 }, { foodId: 7, ingId: 21, qty: 80 }, { foodId: 7, ingId: 22, qty: 50 },
    { foodId: 8, ingId: 5, qty: 140 }, { foodId: 8, ingId: 24, qty: 40 }, { foodId: 8, ingId: 4, qty: 20 },
    { foodId: 9, ingId: 7, qty: 250 }, { foodId: 9, ingId: 19, qty: 80 }, { foodId: 9, ingId: 22, qty: 80 }, { foodId: 9, ingId: 23, qty: 40 },
    { foodId: 10, ingId: 3, qty: 350 }, { foodId: 10, ingId: 19, qty: 100 }, { foodId: 10, ingId: 25, qty: 20 }, { foodId: 10, ingId: 26, qty: 20 },
    { foodId: 11, ingId: 2, qty: 180 }, { foodId: 11, ingId: 12, qty: 200 }, { foodId: 11, ingId: 21, qty: 60 }, { foodId: 11, ingId: 4, qty: 20 },
    { foodId: 12, ingId: 10, qty: 120 }, { foodId: 12, ingId: 11, qty: 25 }, { foodId: 12, ingId: 4, qty: 25 },
    { foodId: 13, ingId: 1, qty: 180 }, { foodId: 13, ingId: 13, qty: 15 }, { foodId: 13, ingId: 4, qty: 20 },
    { foodId: 14, ingId: 1, qty: 200 }, { foodId: 14, ingId: 14, qty: 60 }, { foodId: 14, ingId: 15, qty: 60 }, { foodId: 14, ingId: 6, qty: 40 },
    { foodId: 15, ingId: 8, qty: 250 }, { foodId: 15, ingId: 21, qty: 100 }, { foodId: 15, ingId: 22, qty: 60 }, { foodId: 15, ingId: 23, qty: 30 },
    { foodId: 16, ingId: 9, qty: 220 }, { foodId: 16, ingId: 21, qty: 80 }, { foodId: 16, ingId: 22, qty: 70 }, { foodId: 16, ingId: 23, qty: 30 },
    { foodId: 17, ingId: 3, qty: 250 }, { foodId: 17, ingId: 21, qty: 100 }, { foodId: 17, ingId: 24, qty: 40 }, { foodId: 17, ingId: 4, qty: 30 },
    { foodId: 18, ingId: 14, qty: 120 }, { foodId: 18, ingId: 15, qty: 150 }, { foodId: 18, ingId: 21, qty: 50 }, { foodId: 18, ingId: 23, qty: 25 },
    { foodId: 19, ingId: 16, qty: 180 }, { foodId: 19, ingId: 10, qty: 120 }, { foodId: 19, ingId: 22, qty: 50 }, { foodId: 19, ingId: 23, qty: 40 },
    { foodId: 20, ingId: 17, qty: 150 }, { foodId: 20, ingId: 18, qty: 100 }, { foodId: 20, ingId: 10, qty: 30 },
    { foodId: 21, ingId: 17, qty: 250 }, { foodId: 21, ingId: 18, qty: 80 }, { foodId: 21, ingId: 28, qty: 5 },
    { foodId: 22, ingId: 3, qty: 250 }, { foodId: 22, ingId: 6, qty: 70 }, { foodId: 22, ingId: 22, qty: 60 }, { foodId: 22, ingId: 21, qty: 70 },
    { foodId: 23, ingId: 19, qty: 200 }, { foodId: 23, ingId: 18, qty: 40 }, { foodId: 23, ingId: 17, qty: 50 },
    { foodId: 24, ingId: 19, qty: 180 }, { foodId: 24, ingId: 27, qty: 80 }, { foodId: 24, ingId: 18, qty: 30 },
    { foodId: 25, ingId: 2, qty: 200 }, { foodId: 25, ingId: 4, qty: 45 }, { foodId: 25, ingId: 21, qty: 90 }, { foodId: 25, ingId: 24, qty: 35 },
    { foodId: 26, ingId: 20, qty: 2 }, { foodId: 26, ingId: 21, qty: 70 }, { foodId: 26, ingId: 22, qty: 60 }, { foodId: 26, ingId: 23, qty: 25 },
    { foodId: 27, ingId: 29, qty: 3 }, { foodId: 27, ingId: 14, qty: 50 }, { foodId: 27, ingId: 6, qty: 40 }, { foodId: 27, ingId: 23, qty: 30 },
    { foodId: 28, ingId: 3, qty: 150 }, { foodId: 28, ingId: 26, qty: 15 }, { foodId: 28, ingId: 11, qty: 15 }, { foodId: 28, ingId: 4, qty: 15 },
    { foodId: 29, ingId: 19, qty: 180 }, { foodId: 29, ingId: 13, qty: 10 }, { foodId: 29, ingId: 30, qty: 15 },
    { foodId: 30, ingId: 1, qty: 60 }, { foodId: 30, ingId: 17, qty: 300 }, { foodId: 30, ingId: 18, qty: 60 }, { foodId: 30, ingId: 28, qty: 5 }
  ],
  orders: [
    { id: 1001, status: "CONFIRMED", date: 20261114, items: "Chicken_Biryani x2, Butter_Naan x4", total: 820 },
    { id: 1002, status: "PENDING", date: 20261114, items: "Paneer_Tikka x1, Dal_Makhani x1", total: 430 }
  ],
  transactions: [
    { type: "RECEIVED", ingId: 2, qty: 300, date: 20261025, note: "Old batch" },
    { type: "RECEIVED", ingId: 2, qty: 2500, date: 20261110, note: "Fresh batch" },
    { type: "RECEIVED", ingId: 1, qty: 5000, date: 20261105, note: "New batch" },
    { type: "USED", ingId: 5, qty: 2, date: 20261115, note: "FEFO stock use" },
    { type: "USED", ingId: 2, qty: 1200, date: 20261115, note: "FEFO stock use" }
  ]
};

const $ = (id) => document.getElementById(id);

function esc(v) {
  return String(v ?? "").replace(/[&<>"']/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
}

function dayNumber(d) {
  const y = Math.floor(d / 10000), m = Math.floor(d / 100) % 100, day = d % 100;
  return Math.round(Date.UTC(y, m - 1, day) / 86400000);
}

function showAlert(msg, ok = true) {
  const el = $("alert");
  if (!el) return;
  el.textContent = msg;
  el.className = "alert " + (ok ? "ok" : "err");
  el.classList.remove("hidden");
  setTimeout(() => {
    // Keep errors visible a bit longer
    if (ok) setTimeout(() => el.classList.add("hidden"), 4000);
  }, 100);
}

async function postApi(url) {
  const res = await fetch(API + url, { method: "POST" });
  const data = await res.json();
  if (!data.ok) throw new Error(data.error || "Operation failed");
  return data;
}

// Parses order item string like "Chicken_Biryani x2, Butter_Naan x4" into [{ foodId, qty }]
function parseOrderItems(itemsStr) {
  const result = [];
  if (!itemsStr) return result;
  const parts = itemsStr.split(", ");
  for (const part of parts) {
    const m = part.match(/^(.*) x(\d+)$/);
    if (m) {
      const name = m[1].trim();
      const qty = parseInt(m[2], 10);
      const food = (state.foods || []).find(f => f.name === name || f.name.replace(/_/g, " ") === name.replace(/_/g, " "));
      if (food) {
        result.push({ foodId: food.id, name: food.name, price: food.price, qty });
      }
    }
  }
  return result;
}

// Recalculates demand Hash Table for all confirmed and pending orders
function recalculateDemand() {
  const demandMap = {};
  (state.orders || []).forEach(o => {
    if (o.status === "CONFIRMED" || o.status === "PENDING") {
      const lines = parseOrderItems(o.items);
      lines.forEach(line => {
        (state.recipes || []).filter(r => r.foodId === line.foodId).forEach(r => {
          demandMap[r.ingId] = (demandMap[r.ingId] || 0) + (r.qty * line.qty);
        });
      });
    }
  });

  return (state.ingredients || []).map(g => {
    const needed = demandMap[g.id] || 0;
    const usable = getUsableIngredientStock(g.id);
    return { ingId: g.id, name: g.name, unit: g.unit, needed, usable };
  }).filter(d => d.needed > 0);
}

// Returns ONLY non-expired usable stock for an ingredient (batch.expiry >= today and qty > 0)
function getUsableIngredientStock(ingId) {
  return (state.batches || [])
    .filter(b => b.ingId === ingId && b.qty > 0 && b.expiry >= state.today)
    .reduce((sum, b) => sum + b.qty, 0);
}

// Feature 13: Average daily demand forecast calculation
function calculateAverageDailyDemand() {
  const confirmed = (state.orders || []).filter(o => o.status === "CONFIRMED");
  if (confirmed.length === 0) return [];

  let firstDate = 99999999, lastDate = 0;
  confirmed.forEach(o => {
    if (o.date < firstDate) firstDate = o.date;
    if (o.date > lastDate) lastDate = o.date;
  });

  let days = dayNumber(lastDate) - dayNumber(firstDate) + 1;
  if (days < 1) days = 1;

  const totalUsage = {};
  confirmed.forEach(o => {
    const lines = parseOrderItems(o.items);
    lines.forEach(l => {
      (state.recipes || []).filter(r => r.foodId === l.foodId).forEach(r => {
        totalUsage[r.ingId] = (totalUsage[r.ingId] || 0) + (r.qty * l.qty);
      });
    });
  });

  return (state.ingredients || []).map(g => {
    const total = totalUsage[g.id] || 0;
    const avgDaily = Math.round((total / days) * 10) / 10;
    const usable = getUsableIngredientStock(g.id);
    const coverageDays = avgDaily > 0 ? Math.floor(usable / avgDaily) : "—";
    return { id: g.id, name: g.name, unit: g.unit, avgDaily, usable, coverageDays };
  }).filter(d => d.avgDaily > 0);
}

// Sales ranking using manual Merge Sort (Feature 22.2)
function salesFromOrders() {
  const sales = (state.foods || []).map(f => {
    let plates = 0;
    (state.orders || []).filter(o => o.status === "CONFIRMED").forEach(o => {
      parseOrderItems(o.items).forEach(line => {
        if (line.foodId === f.id) plates += line.qty;
      });
    });
    return { name: f.name, plates, amount: plates * f.price };
  });

  // Manual Merge Sort
  function merge(left, right) {
    const res = [];
    let l = 0, r = 0;
    while (l < left.length && r < right.length) {
      if (left[l].amount >= right[r].amount) res.push(left[l++]);
      else res.push(right[r++]);
    }
    return res.concat(left.slice(l)).concat(right.slice(r));
  }

  function mergeSort(arr) {
    if (arr.length <= 1) return arr;
    const mid = Math.floor(arr.length / 2);
    return merge(mergeSort(arr.slice(0, mid)), mergeSort(arr.slice(mid)));
  }

  return mergeSort(sales).filter(s => s.plates > 0);
}

/* ==========================================================================
   CRITICAL BUG FIX: Validate & Deduct Stock with STRICT EXPIRY CHECKING
   ========================================================================== */
function validateAndConsumeOrderStock(order) {
  const lines = parseOrderItems(order.items);
  if (lines.length === 0) {
    return { ok: false, error: "Order has no items" };
  }

  // 1. Calculate required ingredients per dish
  const need = {};
  for (const line of lines) {
    const dishRecipes = (state.recipes || []).filter(r => r.foodId === line.foodId);
    if (dishRecipes.length === 0) {
      return { ok: false, error: `Cannot confirm: "${line.name}" (Food #${line.foodId}) has no recipe defined!` };
    }
    for (const r of dishRecipes) {
      need[r.ingId] = (need[r.ingId] || 0) + (r.qty * line.qty);
    }
  }

  // 2. Strict Expiry & Usable Stock Verification
  for (const [ingIdStr, reqQty] of Object.entries(need)) {
    const ingId = parseInt(ingIdStr, 10);
    const ing = (state.ingredients || []).find(g => g.id === ingId);
    const ingName = ing ? ing.name : `Ingredient #${ingId}`;
    const unit = ing ? ing.unit : "units";

    let totalStock = 0;
    let usableUnexpired = 0;
    let expiredQty = 0;

    (state.batches || []).forEach(b => {
      if (b.ingId === ingId && b.qty > 0) {
        totalStock += b.qty;
        if (b.expiry >= state.today) {
          usableUnexpired += b.qty;
        } else {
          expiredQty += b.qty;
        }
      }
    });

    // If unexpired stock is less than needed, reject confirmation!
    if (usableUnexpired < reqQty) {
      if (expiredQty > 0) {
        return {
          ok: false,
          error: `CANNOT CONFIRM ORDER: Ingredient "${ingName}" is EXPIRED! (${expiredQty} ${unit} expired, only ${usableUnexpired} ${unit} unexpired vs ${reqQty} ${unit} required). Food cannot be prepared with expired ingredients!`
        };
      } else {
        return {
          ok: false,
          error: `Cannot confirm order: Insufficient stock for "${ingName}" (Available unexpired: ${usableUnexpired} ${unit}, Required: ${reqQty} ${unit}).`
        };
      }
    }
  }

  // 3. Stock is verified unexpired and sufficient: deduct using FEFO (First-Expired, First-Out)
  for (const [ingIdStr, reqQty] of Object.entries(need)) {
    const ingId = parseInt(ingIdStr, 10);
    let remaining = reqQty;
    const validBatches = (state.batches || [])
      .filter(b => b.ingId === ingId && b.qty > 0 && b.expiry >= state.today)
      .sort((a, b) => a.expiry - b.expiry);

    for (const b of validBatches) {
      if (remaining <= 0) break;
      const take = Math.min(b.qty, remaining);
      b.qty -= take;
      remaining -= take;
    }

    // Refresh usable stock count on ingredient model
    const ing = (state.ingredients || []).find(g => g.id === ingId);
    if (ing) {
      ing.usable = getUsableIngredientStock(ingId);
    }

    // Record stock usage transaction
    state.transactions.push({
      type: "USED",
      ingId: ingId,
      qty: reqQty,
      date: state.today,
      note: `FIFO confirm Order #${order.id}`
    });
  }

  return { ok: true };
}

/* ==========================================================================
   RENDER FUNCTION (Updates all 24 features across the dashboard)
   ========================================================================== */
function render() {
  $("today-label").textContent = "Today: " + state.today;
  $("stat-foods").textContent = (state.foods || []).length;
  $("stat-pending").textContent = (state.orders || []).filter(o => o.status === "PENDING").length;

  const expBatches = (state.batches || []).filter(b => b.qty > 0 && dayNumber(b.expiry) - dayNumber(state.today) <= 7);
  $("stat-expiring").textContent = expBatches.length;

  const salesTotal = (state.orders || []).filter(o => o.status === "CONFIRMED").reduce((s, o) => s + (o.total || 0), 0);
  $("stat-sales").textContent = "Rs. " + salesTotal;

  // Overview Batch Alerts table
  $("overview-batches").innerHTML = (state.batches || []).filter(b => b.qty > 0).map(b => {
    const expired = b.expiry < state.today;
    const soon = !expired && dayNumber(b.expiry) - dayNumber(state.today) <= 7;
    const badge = expired ? `<span class="pill pill-bad">EXPIRED</span>`
                : soon ? `<span class="pill pill-warn">EXPIRING SOON</span>`
                : `<span class="pill pill-ok">USABLE</span>`;
    const ing = (state.ingredients || []).find(i => i.id === b.ingId);
    const ingName = ing ? ing.name : (b.ingName || `Ing #${b.ingId}`);
    return `<tr><td>#${b.id}</td><td>${b.ingId}</td><td>${esc(ingName)}</td><td>${b.qty}</td><td>${b.expiry}</td><td>${badge}</td></tr>`;
  }).join("");

  // Feature 1: Menu List
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  $("menu-body").innerHTML = (state.foods || []).map(f => `
    <tr>
      <td>${f.id}</td>
      <td>${esc(f.name)}</td>
      <td>Rs. ${f.price}</td>
      <td>${perms.canOrder ? `<button class="btn-primary btn-sm" onclick="quickOrder(${f.id})">+ Order 1</button>` : `<span class="muted" style="font-size:12px;">View Only</span>`}</td>
    </tr>
  `).join("");

  // Feature 5, 7, 8: Orders Queue
  $("orders-body").innerHTML = (state.orders || []).map(o => {
    const cls = o.status === "CONFIRMED" ? "pill-ok" : (o.status === "PENDING" ? "pill-warn" : "pill-bad");
    const btns = o.status === "PENDING"
      ? `<button class="btn-primary btn-sm" onclick="orderAct(${o.id},'confirm')">Confirm (F7)</button>
         <button class="btn-danger btn-sm" onclick="orderAct(${o.id},'cancel')">Cancel (F8)</button>`
      : "—";
    return `<tr><td>#${o.id}</td><td>${o.date}</td><td>${esc(o.items)}</td><td>Rs. ${o.total}</td>
            <td><span class="pill ${cls}">${o.status}</span></td><td>${btns}</td></tr>`;
  }).join("");

  // Populate Dropdown for Order Placement (Feature 4)
  const orderFoodSel = $("order-food-select");
  if (orderFoodSel && state.foods && state.foods.length > 0) {
    const curVal = orderFoodSel.value;
    orderFoodSel.innerHTML = state.foods.map(f =>
      `<option value="${f.id}">${esc(f.name)} (Rs. ${f.price})</option>`
    ).join("");
    if (curVal && state.foods.some(f => f.id == curVal)) orderFoodSel.value = curVal;
    updateOrderSubtotal();
  }

  // Populate Dropdowns for Update Food (Feature 3)
  const updateFoodSel = $("u-food-id");
  if (updateFoodSel && state.foods) {
    const curVal = updateFoodSel.value;
    updateFoodSel.innerHTML = state.foods.map(f => `<option value="${f.id}">${f.id}: ${esc(f.name)} (Rs. ${f.price})</option>`).join("");
    if (curVal && state.foods.some(f => f.id == curVal)) updateFoodSel.value = curVal;
  }

  // Feature 9: Show Recipes Book
  $("recipes-book-body").innerHTML = (state.foods || []).map(f => {
    const dishRecs = (state.recipes || []).filter(r => r.foodId === f.id);
    const recStr = dishRecs.length > 0 ? dishRecs.map(r => {
      const ing = (state.ingredients || []).find(i => i.id === r.ingId);
      return `${ing ? ing.name : `Ing #${r.ingId}`}: ${r.qty}${ing ? ing.unit : "g"}`;
    }).join(", ") : `<span class="muted">No recipe defined</span>`;
    return `<tr><td>#${f.id}</td><td><strong>${esc(f.name)}</strong></td><td>${esc(recStr)}</td></tr>`;
  }).join("");

  // Populate Recipe Dropdowns (Features 10 & 11)
  const rFidSel = $("r-fid-select");
  if (rFidSel && state.foods) {
    rFidSel.innerHTML = state.foods.map(f => `<option value="${f.id}">#${f.id} - ${esc(f.name)}</option>`).join("");
  }
  const rGidSel = $("r-gid-select");
  if (rGidSel && state.ingredients) {
    rGidSel.innerHTML = state.ingredients.map(g => `<option value="${g.id}">#${g.id} - ${esc(g.name)} (${esc(g.unit)})</option>`).join("");
  }
  const delRFidSel = $("del-r-fid-select");
  if (delRFidSel && state.foods) {
    delRFidSel.innerHTML = state.foods.map(f => `<option value="${f.id}">#${f.id} - ${esc(f.name)}</option>`).join("");
  }

  // Feature 12: Calculate Demand Table
  const demandList = recalculateDemand();
  $("demand-body").innerHTML = demandList.length > 0 ? demandList.map(d => {
    const ok = d.usable >= d.needed;
    return `<tr><td>${esc(d.name)}</td><td>${d.needed} ${esc(d.unit)}</td><td>${d.usable} ${esc(d.unit)}</td>
            <td><span class="pill ${ok ? "pill-ok" : "pill-bad"}">${ok ? "SUFFICIENT" : "DEFICIT"}</span></td></tr>`;
  }).join("") : `<tr><td colspan="4" class="muted" style="text-align:center;">No active ingredient demand</td></tr>`;

  // Feature 13: Average Daily Demand Forecast Table
  const dailyDemandList = calculateAverageDailyDemand();
  $("daily-demand-body").innerHTML = dailyDemandList.length > 0 ? dailyDemandList.map(d => {
    const cov = d.coverageDays === "—" ? "—" : `${d.coverageDays} days`;
    const low = typeof d.coverageDays === "number" && d.coverageDays <= 3;
    return `<tr><td>${esc(d.name)}</td><td>${d.avgDaily} ${esc(d.unit)}/day</td><td>${d.usable} ${esc(d.unit)}</td>
            <td><span class="pill ${low ? "pill-warn" : "pill-ok"}">${cov}</span></td></tr>`;
  }).join("") : `<tr><td colspan="4" class="muted" style="text-align:center;">No confirmed sales history for forecast</td></tr>`;

  // Feature 14: Usable Stock Table
  $("ing-body").innerHTML = (state.ingredients || []).map(g => {
    const usable = getUsableIngredientStock(g.id);
    const low = usable < g.min;
    return `<tr><td>${g.id}</td><td>${esc(g.name)}</td><td>${usable} ${esc(g.unit)}</td><td>${g.min} ${esc(g.unit)}</td>
            <td>Rs. ${g.cost}</td><td><span class="pill ${low ? "pill-bad" : "pill-ok"}">${low ? "LOW STOCK" : "OK"}</span></td></tr>`;
  }).join("");

  // Feature 17: All Stock Batches Table
  $("all-batches-body").innerHTML = (state.batches || []).map(b => {
    const expired = b.expiry < state.today;
    const soon = !expired && dayNumber(b.expiry) - dayNumber(state.today) <= 7;
    const cond = expired ? `<span class="pill pill-bad">EXPIRED (BLOCKED)</span>`
               : soon ? `<span class="pill pill-warn">EXPIRING SOON</span>`
               : `<span class="pill pill-ok">USABLE</span>`;
    const ing = (state.ingredients || []).find(i => i.id === b.ingId);
    return `<tr><td>#${b.id}</td><td>${b.ingId}</td><td>${esc(ing ? ing.name : `Ing #${b.ingId}`)}</td><td>${b.qty}</td><td>${b.expiry}</td><td>${cond}</td></tr>`;
  }).join("");

  // Populate Stock Action Dropdowns (Features 16, 18, 19)
  const bGidSel = $("b-gid-select");
  if (bGidSel && state.ingredients) {
    bGidSel.innerHTML = state.ingredients.map(g => `<option value="${g.id}">#${g.id} - ${esc(g.name)} (${esc(g.unit)})</option>`).join("");
  }
  const uGidSel = $("u-gid-select");
  if (uGidSel && state.ingredients) {
    uGidSel.innerHTML = state.ingredients.map(g => `<option value="${g.id}">#${g.id} - ${esc(g.name)} (Usable: ${getUsableIngredientStock(g.id)} ${esc(g.unit)})</option>`).join("");
  }
  const wGidSel = $("w-gid-select");
  if (wGidSel && state.ingredients) {
    wGidSel.innerHTML = state.ingredients.map(g => `<option value="${g.id}">#${g.id} - ${esc(g.name)}</option>`).join("");
  }

  // Feature 22: Reports Suite
  renderReports();
}

function renderReports() {
  // Report 2: Sales Per Food
  const foodSales = salesFromOrders();
  $("sales-body").innerHTML = foodSales.length > 0 ? foodSales.map((s, i) =>
    `<tr><td>#${i + 1}</td><td><strong>${esc(s.name)}</strong></td><td>${s.plates}</td><td>Rs. ${s.amount}</td></tr>`
  ).join("") : `<tr><td colspan="4" class="muted" style="text-align:center;">No sales recorded yet</td></tr>`;

  // Report 3: Stock Valuation
  let totalValuation = 0;
  $("stock-val-body").innerHTML = (state.ingredients || []).map(g => {
    let totalQty = 0, usableQty = 0;
    (state.batches || []).forEach(b => {
      if (b.ingId === g.id && b.qty > 0) {
        totalQty += b.qty;
        if (b.expiry >= state.today) usableQty += b.qty;
      }
    });
    const val = Math.round(usableQty * g.cost);
    totalValuation += val;
    return `<tr><td>${g.id}</td><td>${esc(g.name)}</td><td>${totalQty} ${esc(g.unit)}</td><td>${usableQty} ${esc(g.unit)}</td><td>Rs. ${g.cost}</td><td>Rs. ${val}</td></tr>`;
  }).join("");
  $("rep-stock-total-value").textContent = "Rs. " + totalValuation;

  // Report 4: Expiry Tracking
  const expBatches = (state.batches || []).filter(b => b.qty > 0 && (b.expiry < state.today || dayNumber(b.expiry) - dayNumber(state.today) <= 7));
  $("rep-expiry-body").innerHTML = expBatches.length > 0 ? expBatches.map(b => {
    const expired = b.expiry < state.today;
    const cond = expired ? `<span class="pill pill-bad">EXPIRED</span>` : `<span class="pill pill-warn">EXPIRING SOON</span>`;
    const ing = (state.ingredients || []).find(i => i.id === b.ingId);
    return `<tr><td>#${b.id}</td><td>${b.ingId}</td><td>${esc(ing ? ing.name : "")}</td><td>${b.qty}</td><td>${b.expiry}</td><td>${cond}</td></tr>`;
  }).join("") : `<tr><td colspan="6" class="muted" style="text-align:center;">No expired or soon-to-expire batches!</td></tr>`;

  // Report 5: Low Stock
  const lowList = (state.ingredients || []).map(g => {
    const usable = getUsableIngredientStock(g.id);
    const deficit = g.min - usable;
    return { g, usable, deficit };
  }).filter(item => item.deficit > 0);

  $("rep-low-stock-body").innerHTML = lowList.length > 0 ? lowList.map(item => {
    return `<tr><td>${item.g.id}</td><td>${esc(item.g.name)}</td><td>${item.usable} ${esc(item.g.unit)}</td><td>${item.g.min} ${esc(item.g.unit)}</td>
            <td class="warn">-${item.deficit} ${esc(item.g.unit)}</td><td><span class="pill pill-bad">REORDER NEEDED</span></td></tr>`;
  }).join("") : `<tr><td colspan="6" class="muted" style="text-align:center;">All ingredients are above minimum thresholds!</td></tr>`;

  // Report 6: Wastage Log
  let totalWastedUnits = 0;
  $("txn-body").innerHTML = (state.transactions || []).slice().reverse().map(t => {
    if (t.type === "WASTED") totalWastedUnits += t.qty;
    const ing = (state.ingredients || []).find(i => i.id === t.ingId);
    return `<tr><td><strong>${esc(t.type)}</strong></td><td>${t.ingId}</td><td>${esc(ing ? ing.name : "-")}</td><td>${t.qty}</td><td>${t.date}</td><td>${esc(t.note)}</td></tr>`;
  }).join("");
  $("rep-wastage-total").textContent = totalWastedUnits + " units";

  // Report 1: Daily Sales calculation
  runDailySalesReport();
}

function runDailySalesReport() {
  const dVal = +$("daily-rep-date").value || state.today;
  const dayOrders = (state.orders || []).filter(o => o.date === dVal && o.status === "CONFIRMED");
  const dayTotal = dayOrders.reduce((sum, o) => sum + (o.total || 0), 0);
  $("daily-rep-result").textContent = `Sales on ${dVal} = Rs. ${dayTotal} (${dayOrders.length} confirmed orders)`;
  $("daily-rep-body").innerHTML = dayOrders.length > 0 ? dayOrders.map(o => `
    <tr><td>#${o.id}</td><td>${o.date}</td><td>${esc(o.items)}</td><td>Rs. ${o.total}</td></tr>
  `).join("") : `<tr><td colspan="4" class="muted" style="text-align:center;">No confirmed sales recorded for date ${dVal}</td></tr>`;
}

$("daily-rep-btn").addEventListener("click", runDailySalesReport);

// Sub-Tab Switcher in Reports View (Feature 22)
document.querySelectorAll("#report-tabs .tab-btn").forEach(btn => {
  btn.addEventListener("click", () => {
    document.querySelectorAll("#report-tabs .tab-btn").forEach(b => b.classList.remove("active"));
    btn.classList.add("active");
    const target = btn.dataset.report;
    document.querySelectorAll(".report-sec").forEach(sec => sec.classList.toggle("hidden", sec.id !== "rep-sec-" + target));
  });
});

/* ==========================================================================
   SYNC & AUTOMATIC DATA FETCHING (Connected to C++ Backend Server)
   ========================================================================== */
async function sync() {
  try {
    const res = await fetch(API + "/api/state");
    const data = await res.json();
    if (data.ok) {
      state = data;
      offlineDemo = false;
      $("conn-mode").textContent = "Connected to C++ Backend Server (Live)";
    }
  } catch (e) {
    offlineDemo = true;
    $("conn-mode").textContent = "Browser Preview Mode (Offline Storage Active)";
  }
  render();
}

/* ==========================================================================
   LOGIN & USER SWITCHING
   ========================================================================== */
$("login-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const u = $("login-user").value.trim();
  const p = $("login-pass").value.trim();
  const demoUsers = { admin: "ADMIN", staff: "STAFF", stock: "STOCK_FILLER" };

  let data = null;
  try {
    const res = await fetch(`${API}/api/login?user=${encodeURIComponent(u)}&pass=${encodeURIComponent(p)}`, { method: "POST" });
    data = await res.json();
  } catch (err) {
    const demoPass = { admin: "admin123", staff: "staff123", stock: "stock123" };
    if (demoUsers[u] && demoPass[u] === p) data = { ok: true, role: demoUsers[u] };
    else data = { ok: false };
  }
  if (data && data.ok) {
    currentUserRole = data.role || "ADMIN";
    $("login-err").classList.add("hidden");
    $("login-modal").classList.add("hidden");
    applyRolePermissions(currentUserRole);
    sync();
    const roleTitle = (rolePermissions[currentUserRole] || {}).title || currentUserRole;
    showAlert(`Logged in as ${roleTitle} (${currentUserRole}). Unlocked permissible features.`);
  } else {
    $("login-err").textContent = "Invalid credentials (try admin / admin123)";
    $("login-err").classList.remove("hidden");
  }
});

$("logout-btn").addEventListener("click", () => $("login-modal").classList.remove("hidden"));

/* ==========================================================================
   ORDER TICKET BUILDER & QUICK ORDERS (Features 4, 6, 7 & 8)
   ========================================================================== */
let currentTicket = [];

function updateOrderSubtotal() {
  const select = $("order-food-select");
  const qtyInput = $("order-food-qty");
  const subtotalEl = $("order-item-subtotal");
  if (!select || !qtyInput || !subtotalEl) return;
  const food = state.foods.find(f => f.id == select.value);
  const qty = parseInt(qtyInput.value, 10) || 1;
  const total = food ? food.price * qty : 0;
  subtotalEl.textContent = "Rs. " + total;
}

if ($("order-food-select")) $("order-food-select").addEventListener("change", updateOrderSubtotal);
if ($("order-food-qty")) $("order-food-qty").addEventListener("input", updateOrderSubtotal);

function renderTicket() {
  const container = $("order-ticket-items");
  const section = $("order-ticket-section");
  const totalEl = $("order-ticket-total");
  if (!container || !section) return;

  if (currentTicket.length === 0) {
    section.classList.add("hidden");
    container.innerHTML = "";
    return;
  }

  section.classList.remove("hidden");
  let total = 0;
  container.innerHTML = currentTicket.map((item, index) => {
    const itemTotal = item.price * item.qty;
    total += itemTotal;
    return `
      <div style="display:flex; justify-content:space-between; align-items:center; background:#222630; padding:6px 10px; border-radius:6px; font-size:13px;">
        <span><strong>${esc(item.name)}</strong> × ${item.qty}</span>
        <span>
          <span style="margin-right:10px; color:#fbbf24;">Rs. ${itemTotal}</span>
          <button type="button" class="btn-danger btn-sm" onclick="removeTicketItem(${index})" style="padding:2px 6px;">✕</button>
        </span>
      </div>
    `;
  }).join("");
  if (totalEl) totalEl.textContent = "Rs. " + total;
}

window.removeTicketItem = (index) => {
  currentTicket.splice(index, 1);
  renderTicket();
};

async function placeTicketOrder(ticket) {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canOrder) {
    showAlert(`Permission denied: ${currentUserRole} cannot place orders.`, false);
    return;
  }
  if (!ticket || ticket.length === 0) {
    showAlert("Order is empty. Please select a dish.", false);
    return;
  }

  const itemsDesc = ticket.map(t => `${t.name} x${t.qty}`).join(", ");
  const totalAmount = ticket.reduce((sum, t) => sum + t.price * t.qty, 0);

  if (offlineDemo) {
    const maxId = (state.orders || []).reduce((m, o) => Math.max(m, o.id || 0), 1000);
    const newId = maxId + 1;
    state.orders.push({
      id: newId,
      status: "PENDING",
      date: state.today,
      items: itemsDesc,
      total: totalAmount
    });
    autoPersistToFile("orders.txt");
    currentTicket = [];
    renderTicket();
    showAlert(`Order #${newId} placed into Circular FIFO Queue!`);
    render();
    return;
  }

  try {
    let url = "";
    if (ticket.length === 1) {
      url = `/api/order/place?foodId=${ticket[0].foodId}&qty=${ticket[0].qty}`;
    } else {
      const itemsParam = ticket.map(t => `${t.foodId}:${t.qty}`).join(",");
      url = `/api/order/place?items=${encodeURIComponent(itemsParam)}`;
    }
    const res = await postApi(url);
    currentTicket = [];
    renderTicket();
    showAlert(`Order #${res.id || "ticket"} placed into Circular FIFO Queue!`);
    sync();
  } catch (err) {
    showAlert(err.message, false);
  }
}

window.quickOrder = async (foodId) => {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canOrder) {
    return showAlert(`Permission denied: ${currentUserRole} cannot place orders.`, false);
  }
  const f = (state.foods || []).find(x => x.id === foodId);
  if (!f) return;
  placeTicketOrder([{ foodId: f.id, name: f.name, price: f.price, qty: 1 }]);
};

// Feature 7 & 8: Confirm or Cancel Order
window.orderAct = async (id, act) => {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canManageOrders) {
    return showAlert(`Permission denied: ${currentUserRole} cannot modify orders.`, false);
  }

  if (offlineDemo) {
    const o = (state.orders || []).find(x => x.id === id);
    if (!o) return showAlert("Order not found", false);

    if (act === "confirm") {
      // STRICT EXPIRY AND STOCK CHECK
      const check = validateAndConsumeOrderStock(o);
      if (!check.ok) {
        return showAlert(check.error, false);
      }
      o.status = "CONFIRMED";
      showAlert(`Order #${id} confirmed and unexpired stock deducted (FEFO)!`);
      autoPersistToFile("orders.txt");
      autoPersistToFile("batches.txt");
      autoPersistToFile("transactions.txt");
    } else {
      o.status = "CANCELLED";
      showAlert(`Order #${id} cancelled.`);
      autoPersistToFile("orders.txt");
    }
    return render();
  }

  try {
    await postApi(`/api/order/action?id=${id}&act=${act}`);
    showAlert("Order " + (act === "confirm" ? "confirmed and unexpired stock deducted (FIFO)!" : "cancelled in C++!"));
    sync();
  } catch (e) {
    showAlert(e.message, false);
  }
};

// Feature 6: Confirm Next Order (Circular Queue Pop)
$("confirm-next-btn").addEventListener("click", async () => {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canManageOrders) {
    return showAlert(`Permission denied: ${currentUserRole} cannot confirm orders.`, false);
  }
  if (offlineDemo) {
    const next = (state.orders || []).find(o => o.status === "PENDING");
    if (!next) return showAlert("No pending orders in queue.", false);

    const check = validateAndConsumeOrderStock(next);
    if (!check.ok) {
      return showAlert(check.error, false);
    }
    next.status = "CONFIRMED";
    showAlert(`Confirmed Order #${next.id} via Circular FIFO Queue!`);
    autoPersistToFile("orders.txt");
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    return render();
  }
  try {
    const res = await postApi("/api/order/confirmNext");
    showAlert(`Confirmed order #${res.id || "next"} from Circular Queue!`);
    sync();
  } catch (e) {
    showAlert(e.message, false);
    sync();
  }
});

$("order-add-item-btn").addEventListener("click", () => {
  const select = $("order-food-select");
  const qtyInput = $("order-food-qty");
  const foodId = parseInt(select.value, 10);
  const qty = parseInt(qtyInput.value, 10) || 1;
  const food = state.foods.find(f => f.id === foodId);
  if (!food) return showAlert("Please select a dish from the menu", false);

  const existing = currentTicket.find(t => t.foodId === foodId);
  if (existing) {
    existing.qty += qty;
  } else {
    currentTicket.push({ foodId: food.id, name: food.name, price: food.price, qty });
  }
  qtyInput.value = 1;
  renderTicket();
  showAlert(`Added ${qty}x ${food.name} to ticket!`);
});

$("order-form").addEventListener("submit", (e) => {
  e.preventDefault();
  const select = $("order-food-select");
  const qtyInput = $("order-food-qty");
  const foodId = parseInt(select.value, 10);
  const qty = parseInt(qtyInput.value, 10) || 1;
  const food = state.foods.find(f => f.id === foodId);
  if (!food) return showAlert("Please select a dish from the menu", false);

  if (currentTicket.length > 0) {
    placeTicketOrder(currentTicket);
  } else {
    placeTicketOrder([{ foodId: food.id, name: food.name, price: food.price, qty }]);
  }
});

$("order-submit-ticket-btn").addEventListener("click", () => placeTicketOrder(currentTicket));

/* ==========================================================================
   MENU FORMS: ADD & UPDATE (Features 2 & 3)
   ========================================================================== */
$("add-food-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canAddFood) return showAlert(`Permission denied: ${currentUserRole} cannot add food items.`, false);

  const id = +$("f-id").value, name = $("f-name").value.trim(), price = +$("f-price").value;
  if ((state.foods || []).some(f => f.id === id)) return showAlert(`Food ID ${id} already exists!`, false);

  if (offlineDemo) {
    state.foods.push({ id, name, price });
    state.foods.sort((a, b) => a.id - b.id);
    autoPersistToFile("foods.txt");
    showAlert("Food added to sorted menu!");
    $("f-id").value = id + 1;
    return render();
  }
  try {
    await postApi(`/api/food/add?id=${id}&name=${encodeURIComponent(name)}&price=${price}`);
    showAlert("Food added & saved to foods.txt!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("update-food-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canUpdateFood) return showAlert(`Permission denied: Only ADMIN can update dishes.`, false);

  const id = +$("u-food-id").value;
  const name = $("u-food-name").value.trim();
  const price = +$("u-food-price").value;

  if (offlineDemo) {
    const f = (state.foods || []).find(x => x.id === id);
    if (!f) return showAlert("Food not found", false);
    f.name = name;
    f.price = price;
    autoPersistToFile("foods.txt");
    showAlert(`Dish #${id} successfully updated!`);
    return render();
  }
  try {
    await postApi(`/api/food/update?id=${id}&name=${encodeURIComponent(name)}&price=${price}`);
    showAlert(`Dish #${id} successfully updated in C++ & foods.txt!`);
    sync();
  } catch (e) { showAlert(e.message, false); }
});

/* ==========================================================================
   RECIPE FORMS: ADD & DELETE (Features 10 & 11)
   ========================================================================== */
$("add-recipe-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canManageRecipes) return showAlert(`Permission denied: ${currentUserRole} cannot add recipes.`, false);

  const fid = +$("r-fid-select").value, gid = +$("r-gid-select").value, qty = +$("r-qty").value;
  if (offlineDemo) {
    state.recipes.push({ foodId: fid, ingId: gid, qty });
    autoPersistToFile("recipes.txt");
    showAlert("Recipe ingredient saved!");
    return render();
  }
  try {
    await postApi(`/api/recipe/add?foodId=${fid}&ingId=${gid}&qty=${qty}`);
    showAlert("Recipe saved to recipes.txt!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("delete-recipe-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canManageRecipes) return showAlert(`Permission denied: Only ADMIN can delete recipes.`, false);

  const fid = +$("del-r-fid-select").value;
  if (offlineDemo) {
    state.recipes = (state.recipes || []).filter(r => r.foodId !== fid);
    autoPersistToFile("recipes.txt");
    showAlert(`Recipe for Food #${fid} deleted!`);
    return render();
  }
  try {
    await postApi(`/api/recipe/delete?foodId=${fid}`);
    showAlert(`Recipe for Food #${fid} deleted in C++ & recipes.txt!`);
    sync();
  } catch (e) { showAlert(e.message, false); }
});

/* ==========================================================================
   INVENTORY FORMS: ADD INGREDIENT, BATCH, USE, WASTE, EXPIRE, UNDO
   (Features 15, 16, 18, 19, 20 & 21)
   ========================================================================== */
$("add-ing-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canReceiveStock) return showAlert(`Permission denied: Staff cannot add ingredients.`, false);

  const id = +$("ing-id").value, name = $("ing-name").value.trim(), unit = $("ing-unit").value.trim();
  const min = +$("ing-min").value, cost = +$("ing-cost").value;

  if ((state.ingredients || []).some(g => g.id === id)) return showAlert(`Ingredient ID ${id} already exists!`, false);

  if (offlineDemo) {
    state.ingredients.push({ id, name, unit, usable: 0, min, cost });
    autoPersistToFile("ingredients.txt");
    showAlert(`Ingredient "${name}" added!`);
    $("ing-id").value = id + 1;
    return render();
  }
  try {
    await postApi(`/api/ingredient/add?id=${id}&name=${encodeURIComponent(name)}&unit=${encodeURIComponent(unit)}&min=${min}&cost=${cost}`);
    showAlert("Ingredient added & saved to ingredients.txt!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("receive-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canReceiveStock) return showAlert(`Permission denied: ${currentUserRole} cannot receive stock.`, false);

  const bid = +$("b-id").value, gid = +$("b-gid-select").value, qty = +$("b-qty").value, exp = +$("b-exp").value;
  if (offlineDemo) {
    state.batches.push({ id: bid, ingId: gid, ingName: "", qty, expiry: exp });
    state.transactions.push({ type: "RECEIVED", ingId: gid, qty, date: state.today, note: "Fresh batch" });
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    showAlert("Batch received and pushed to Undo Stack!");
    $("b-id").value = bid + 1;
    return render();
  }
  try {
    await postApi(`/api/stock/receive?batchId=${bid}&ingId=${gid}&qty=${qty}&expiry=${exp}`);
    showAlert("Batch added & pushed to Undo Stack!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("use-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canUseStock) return showAlert(`Permission denied: ${currentUserRole} cannot use stock.`, false);

  const gid = +$("u-gid-select").value, qty = +$("u-qty").value;
  if (offlineDemo) {
    const usable = getUsableIngredientStock(gid);
    if (usable < qty) return showAlert(`Not enough unexpired usable stock! (Available: ${usable})`, false);

    let rem = qty;
    const sorted = (state.batches || []).filter(b => b.ingId === gid && b.qty > 0 && b.expiry >= state.today).sort((a,b)=>a.expiry-b.expiry);
    for (const b of sorted) {
      if (rem <= 0) break;
      const take = Math.min(b.qty, rem);
      b.qty -= take;
      rem -= take;
    }
    state.transactions.push({ type: "USED", ingId: gid, qty, date: state.today, note: "FEFO manual stock use" });
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    showAlert("Stock deducted using Min-Heap FEFO (earliest expiry first)!");
    return render();
  }
  try {
    await postApi(`/api/stock/use?ingId=${gid}&qty=${qty}`);
    showAlert("Stock deducted using FEFO Min-Heap!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

$("waste-form").addEventListener("submit", async (e) => {
  e.preventDefault();
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canWasteStock) return showAlert(`Permission denied: ${currentUserRole} cannot record wastage.`, false);

  const gid = +$("w-gid-select").value, qty = +$("w-qty").value, note = $("w-note").value.trim();
  if (offlineDemo) {
    let rem = qty;
    for (const b of (state.batches || [])) {
      if (b.ingId === gid && b.qty > 0) {
        const take = Math.min(b.qty, rem);
        b.qty -= take;
        rem -= take;
        if (rem <= 0) break;
      }
    }
    state.transactions.push({ type: "WASTED", ingId: gid, qty, date: state.today, note });
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    showAlert("Wastage recorded and logged!");
    return render();
  }
  try {
    await postApi(`/api/stock/waste?ingId=${gid}&qty=${qty}&note=${encodeURIComponent(note)}`);
    showAlert("Wastage recorded!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

async function writeOffExpiredAction() {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canExpireStock) return showAlert(`Permission denied: ${currentUserRole} cannot write off expired stock.`, false);

  if (offlineDemo) {
    let expiredCount = 0;
    (state.batches || []).forEach(b => {
      if (b.qty > 0 && b.expiry < state.today) {
        state.transactions.push({ type: "WASTED", ingId: b.ingId, qty: b.qty, date: state.today, note: "Expired batch write-off" });
        b.qty = 0;
        expiredCount++;
      }
    });
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    showAlert(`Checked batches: ${expiredCount} expired batch(es) written off to wastage!`);
    return render();
  }
  try {
    await postApi("/api/stock/expire");
    showAlert("Expired batches checked and written off to wastage log!");
    sync();
  } catch (e) { showAlert(e.message, false); }
}

$("expire-btn").addEventListener("click", writeOffExpiredAction);
if ($("quick-expire-btn")) $("quick-expire-btn").addEventListener("click", writeOffExpiredAction);

$("undo-btn").addEventListener("click", async () => {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.canUndoStock) return showAlert(`Permission denied: Only ADMIN can undo stock receipt.`, false);

  if (offlineDemo) {
    const lastRec = (state.batches || []).pop();
    if (!lastRec) return showAlert("Undo stack is empty", false);
    state.transactions.push({ type: "UNDO", ingId: lastRec.ingId, qty: lastRec.qty, date: state.today, note: "Undo receipt" });
    autoPersistToFile("batches.txt");
    autoPersistToFile("transactions.txt");
    showAlert(`Undid last batch #${lastRec.id} receipt via LIFO Stack!`);
    return render();
  }
  try {
    await postApi("/api/stock/undo");
    showAlert("Last stock receipt undone via LIFO Stack!");
    sync();
  } catch (e) { showAlert(e.message, false); }
});

/* ==========================================================================
   ROLE PERMISSIONS & VIEW SWITCHING
   ========================================================================== */
function switchToView(v) {
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;
  if (!perms.views.includes(v)) {
    showAlert(`Access restricted: ${currentUserRole} role cannot access "${v}".`, false);
    v = perms.views[0] || "overview";
  }
  document.querySelectorAll(".nav-btn").forEach(b => {
    b.classList.toggle("active", b.dataset.view === v);
  });
  document.querySelectorAll(".view").forEach(sec => {
    sec.classList.toggle("hidden", sec.id !== "view-" + v);
  });
  if (viewInfo[v]) {
    $("page-title").textContent = viewInfo[v][0];
    $("page-sub").textContent = viewInfo[v][1];
  }
}

function applyRolePermissions(role) {
  currentUserRole = role || "ADMIN";
  const perms = rolePermissions[currentUserRole] || rolePermissions.ADMIN;

  document.querySelectorAll(".nav-btn").forEach(btn => {
    const v = btn.dataset.view;
    btn.classList.toggle("hidden", !perms.views.includes(v));
  });

  const activeBtn = document.querySelector(".nav-btn.active");
  const currentView = activeBtn ? activeBtn.dataset.view : "overview";
  if (!perms.views.includes(currentView)) {
    switchToView(perms.views[0] || "overview");
  }

  // Hide add/update panels for unauthorized roles
  const addFoodPanel = $("add-food-panel");
  const menuLayout = $("menu-layout");
  if (addFoodPanel) addFoodPanel.classList.toggle("hidden", !perms.canAddFood);
  if (menuLayout) menuLayout.style.gridTemplateColumns = perms.canAddFood ? "" : "1fr";

  const delRecipePanel = $("delete-recipe-panel");
  if (delRecipePanel) delRecipePanel.classList.toggle("hidden", !perms.canManageRecipes || currentUserRole !== "ADMIN");

  const addIngPanel = $("add-ing-panel");
  if (addIngPanel) addIngPanel.classList.toggle("hidden", !perms.canReceiveStock);

  const undoBtn = $("undo-btn");
  if (undoBtn) undoBtn.classList.toggle("hidden", !perms.canUndoStock);

  if ($("user-badge")) $("user-badge").textContent = `Role: ${currentUserRole}`;
}

document.querySelectorAll(".nav-btn").forEach(btn => {
  btn.addEventListener("click", () => switchToView(btn.dataset.view));
});

/* ==========================================================================
   DATA SERIALIZER & PERSISTENCE TO DATA/*.TXT (Features 23 & 24)
   ========================================================================== */
function serializeDataFiles() {
  const foodsTxt = (state.foods || []).map(f => `${f.id}|${f.name}|${f.price}`).join("\n") + "\n";
  const ingredientsTxt = (state.ingredients || []).map(g => `${g.id}|${g.name}|${g.unit}|${g.min}|${g.cost}`).join("\n") + "\n";
  const batchesTxt = (state.batches || []).map(b => `${b.id}|${b.ingId}|${b.qty}|${b.expiry}`).join("\n") + "\n";

  const ordersTxt = (state.orders || []).map(o => {
    const lines = parseOrderItems(o.items);
    const lineParts = lines.length > 0 ? "|" + lines.map(l => `${l.foodId}|${l.qty}`).join("|") : "|0";
    return `${o.id}|${o.status}|${o.date}|${lines.length}${lineParts}`;
  }).join("\n") + "\n";

  const recipesTxt = (state.recipes || []).map(r => `${r.foodId}|${r.ingId}|${r.qty}`).join("\n") + "\n";
  const transactionsTxt = (state.transactions || []).map(t => `${t.type}|${t.ingId}|${t.qty}|${t.date}|${t.note || ""}`).join("\n") + "\n";

  return {
    "foods.txt": foodsTxt,
    "ingredients.txt": ingredientsTxt,
    "batches.txt": batchesTxt,
    "orders.txt": ordersTxt,
    "recipes.txt": recipesTxt,
    "transactions.txt": transactionsTxt
  };
}

let dataDirHandle = null;

function downloadFile(filename, content) {
  const blob = new Blob([content], { type: "text/plain;charset=utf-8" });
  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = filename;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  setTimeout(() => URL.revokeObjectURL(link.href), 1000);
}

// Feature 23: Save All Data to text files
async function saveToDataFolderFiles() {
  if (!offlineDemo) {
    try {
      await postApi("/api/data/save");
      showAlert("✓ All current website data saved directly to data/*.txt files on disk!");
      return;
    } catch (err) {
      console.log("Server save returned:", err);
    }
  }

  // Browser mode: Use File System Access API if supported
  const files = serializeDataFiles();
  if (window.showDirectoryPicker) {
    try {
      if (!dataDirHandle) {
        showAlert("Select the project 'data' folder to save files directly into it...");
        dataDirHandle = await window.showDirectoryPicker({ id: "restaurant-data-folder", mode: "readwrite" });
      }
      for (const [name, content] of Object.entries(files)) {
        const fileHandle = await dataDirHandle.getFileHandle(name, { create: true });
        const writable = await fileHandle.createWritable();
        await writable.write(content);
        await writable.close();
      }
      showAlert("✓ Successfully saved directly into data/ folder text files (foods.txt, ingredients.txt, batches.txt, orders.txt, recipes.txt, transactions.txt)!");
      return;
    } catch (e) {
      if (e.name === "AbortError") return showAlert("Folder selection was cancelled.", false);
    }
  }

  for (const [name, content] of Object.entries(files)) {
    downloadFile(name, content);
  }
  showAlert("Downloaded updated data files to place inside your data/ folder!");
}

// Feature 24: Reload All Data from text files
async function loadDataFromFiles() {
  if (!offlineDemo) {
    try {
      await postApi("/api/data/load");
      await sync();
      showAlert("✓ All data reloaded directly from data/*.txt files!");
      return;
    } catch (err) {
      console.log("Server reload returned:", err);
    }
  }
  await sync();
  showAlert("Data refreshed!");
}

async function autoPersistToFile(filename) {
  if (dataDirHandle && window.showDirectoryPicker) {
    try {
      const files = serializeDataFiles();
      if (files[filename]) {
        const fileHandle = await dataDirHandle.getFileHandle(filename, { create: true });
        const writable = await fileHandle.createWritable();
        await writable.write(files[filename]);
        await writable.close();
      }
    } catch (err) {}
  }
}

function padRight(str, len) {
  str = String(str ?? "");
  return str.length >= len ? str + " " : str + " ".repeat(len - str.length);
}

function generateWebsiteDataText() {
  const now = new Date();
  const timeStr = now.toLocaleString();
  const foods = state.foods || [];
  const orders = state.orders || [];
  const ingredients = state.ingredients || [];
  const batches = state.batches || [];
  const demand = recalculateDemand();
  const transactions = state.transactions || [];
  const foodSales = salesFromOrders();

  const expBatches = batches.filter(b => b.qty > 0 && dayNumber(b.expiry) - dayNumber(state.today) <= 7);
  const salesTotal = orders.filter(o => o.status === "CONFIRMED").reduce((s, o) => s + (o.total || 0), 0);
  const pendingOrders = orders.filter(o => o.status === "PENDING").length;
  const roleText = $("user-badge") ? $("user-badge").textContent.replace("Role: ", "").trim() : "ADMIN";

  let out = "";
  out += "================================================================================\n";
  out += "       SMART RESTAURANT INVENTORY SYSTEM - COMPLETE 24-FEATURE EXPORT\n";
  out += "================================================================================\n";
  out += `Export Timestamp : ${timeStr}\n`;
  out += `System Date      : ${state.today}\n`;
  out += `Connection Mode  : ${offlineDemo ? "Browser Storage Mode" : "Connected to C++ Backend"}\n`;
  out += `Current User Role: ${roleText}\n`;
  out += "================================================================================\n\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "1. KEY PERFORMANCE INDICATORS & SUMMARY\n";
  out += "--------------------------------------------------------------------------------\n";
  out += `• Total Dishes on Menu   : ${foods.length}\n`;
  out += `• Pending Orders (Queue) : ${pendingOrders}\n`;
  out += `• Expiring / Expired     : ${expBatches.length} batch(es)\n`;
  out += `• Confirmed Sales Total  : Rs. ${salesTotal}\n\n`;

  out += "--------------------------------------------------------------------------------\n";
  out += "2. FOOD MENU (Features 1, 2, 3: Sorted by ID - Binary Search)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("ID", 8) + padRight("Dish Name", 28) + padRight("Price (Rs.)", 12) + "\n";
  out += "-".repeat(50) + "\n";
  foods.forEach(f => {
    out += padRight(f.id, 8) + padRight(f.name, 28) + padRight("Rs. " + f.price, 12) + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "3. CUSTOMER ORDERS & STATUS (Features 4, 5, 6, 7, 8: Circular FIFO Queue)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("Order ID", 10) + padRight("Date", 12) + padRight("Status", 14) + padRight("Total", 12) + "Ordered Items\n";
  out += "-".repeat(78) + "\n";
  orders.forEach(o => {
    out += padRight("#" + o.id, 10) + padRight(o.date, 12) + padRight(o.status, 14) + padRight("Rs. " + o.total, 12) + (o.items || "—") + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "4. INGREDIENT INVENTORY & USABLE STOCK (Features 14, 15)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("ID", 6) + padRight("Ingredient Name", 22) + padRight("Usable", 14) + padRight("Min Stock", 14) + padRight("Cost/Unit", 12) + "Status\n";
  out += "-".repeat(78) + "\n";
  ingredients.forEach(g => {
    const usable = getUsableIngredientStock(g.id);
    const isLow = usable < g.min;
    const status = isLow ? "LOW STOCK" : "OK";
    out += padRight(g.id, 6) + padRight(g.name, 22) + padRight(usable + " " + g.unit, 14) + padRight(g.min + " " + g.unit, 14) + padRight("Rs. " + g.cost, 12) + status + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "5. INGREDIENT BATCHES (Features 16, 17, 18, 20, 21: FEFO Heap & Expiry)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("Batch ID", 10) + padRight("Ing ID", 8) + padRight("Ingredient", 20) + padRight("Qty", 10) + padRight("Expiry", 12) + "Condition\n";
  out += "-".repeat(78) + "\n";
  batches.filter(b => b.qty > 0).forEach(b => {
    const expired = b.expiry < state.today;
    const soon = !expired && dayNumber(b.expiry) - dayNumber(state.today) <= 7;
    const cond = expired ? "EXPIRED (BLOCKED)" : (soon ? "EXPIRING SOON" : "USABLE");
    const ing = ingredients.find(i => i.id === b.ingId);
    out += padRight("#" + b.id, 10) + padRight(b.ingId, 8) + padRight(ing ? ing.name : "", 20) + padRight(b.qty, 10) + padRight(b.expiry, 12) + cond + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "6. RECIPES & INGREDIENT DEMAND (Features 9, 10, 11, 12, 13)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("Ingredient", 24) + padRight("Required", 16) + padRight("Available Usable", 18) + "Status\n";
  out += "-".repeat(70) + "\n";
  demand.forEach(d => {
    const ok = d.usable >= d.needed;
    out += padRight(d.name, 24) + padRight(d.needed + " " + d.unit, 16) + padRight(d.usable + " " + d.unit, 18) + (ok ? "SUFFICIENT" : "DEFICIT") + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "7. SALES PERFORMANCE RANKING (Feature 22.2: Merge Sort)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("Rank", 8) + padRight("Dish Name", 28) + padRight("Plates Sold", 14) + "Total Sales\n";
  out += "-".repeat(65) + "\n";
  foodSales.forEach((s, i) => {
    out += padRight("#" + (i + 1), 8) + padRight(s.name, 28) + padRight(s.plates, 14) + "Rs. " + s.amount + "\n";
  });
  out += "\n";

  out += "--------------------------------------------------------------------------------\n";
  out += "8. STOCK TRANSACTION HISTORY LOG (Feature 22.6)\n";
  out += "--------------------------------------------------------------------------------\n";
  out += padRight("Type", 12) + padRight("Ing ID", 8) + padRight("Quantity", 12) + padRight("Date", 12) + "Note\n";
  out += "-".repeat(65) + "\n";
  transactions.slice().reverse().forEach(t => {
    out += padRight(t.type, 12) + padRight(t.ingId, 8) + padRight(t.qty, 12) + padRight(t.date, 12) + (t.note || "-") + "\n";
  });
  out += "\n";

  out += "================================================================================\n";
  out += "9. RAW BACKUP DATA (Pipe-Delimited Records - Features 23 & 24)\n";
  out += "================================================================================\n\n";

  out += "[FOODS: id|name|price]\n";
  foods.forEach(f => { out += `${f.id}|${f.name}|${f.price}\n`; });

  out += "\n[INGREDIENTS: id|name|unit|minimum|cost]\n";
  ingredients.forEach(g => { out += `${g.id}|${g.name}|${g.unit}|${g.min}|${g.cost}\n`; });

  out += "\n[BATCHES: id|ingredientId|quantity|expiry]\n";
  batches.forEach(b => { out += `${b.id}|${b.ingId}|${b.qty}|${b.expiry}\n`; });

  out += "\n[ORDERS: id|status|date|total|items]\n";
  orders.forEach(o => { out += `${o.id}|${o.status}|${o.date}|${o.total}|${o.items}\n`; });

  out += "\n[TRANSACTIONS: type|ingredientId|quantity|date|note]\n";
  transactions.forEach(t => { out += `${t.type}|${t.ingId}|${t.qty}|${t.date}|${t.note || ""}\n`; });

  out += "\n================================================================================\n";
  out += "                             END OF DATA EXPORT\n";
  out += "================================================================================\n";
  return out;
}

function saveDataToTextFile() {
  try {
    const textData = generateWebsiteDataText();
    const dateStr = state.today || new Date().toISOString().slice(0, 10).replace(/-/g, "");
    const filename = `restaurant_data_${dateStr}.txt`;

    downloadFile(filename, textData);

    if (!offlineDemo) {
      fetch(`${API}/api/export/save`, {
        method: "POST",
        headers: { "Content-Type": "text/plain;charset=utf-8" },
        body: textData
      }).catch(err => console.log("Server file save info:", err));
    }

    showAlert(`Website data successfully saved & downloaded as "${filename}"!`);
  } catch (err) {
    showAlert("Failed to save data: " + err.message, false);
  }
}

// Global Event Listeners for Top Actions and Reports
$("refresh-btn").addEventListener("click", () => { sync(); showAlert("Data synced!"); });
if ($("load-data-btn")) $("load-data-btn").addEventListener("click", loadDataFromFiles);
if ($("load-data-report-btn")) $("load-data-report-btn").addEventListener("click", loadDataFromFiles);
if ($("save-data-folder-btn")) $("save-data-folder-btn").addEventListener("click", saveToDataFolderFiles);
if ($("save-data-folder-report-btn")) $("save-data-folder-report-btn").addEventListener("click", saveToDataFolderFiles);
if ($("save-txt-btn")) $("save-txt-btn").addEventListener("click", saveDataToTextFile);
if ($("export-reports-btn")) $("export-reports-btn").addEventListener("click", saveDataToTextFile);

applyRolePermissions(currentUserRole);
sync();
