// web/static/game.js

let selectedFrom = null;
let lastFen = null;

const PIECE_GLYPHS = {
  P: "♙", N: "♘", B: "♗", R: "♖", Q: "♕", K: "♔",
  p: "♟", n: "♞", b: "♝", r: "♜", q: "♛", k: "♚",
};

function $(id) {
  return document.getElementById(id);
}

function updateOutput(text) {
  const out = $("output");
  out.textContent = text || "";
  out.scrollTop = out.scrollHeight;
}

function focusMoveInput() {
  const el = $("moveInput");
  el.focus();
  el.select();
}

function fenToBoardArray(fen) {
  const placement = (fen || "").split(" ")[0];
  const ranks = placement.split("/");
  if (ranks.length !== 8) return null;

  const board = Array.from({ length: 8 }, () => Array(8).fill(null));
  for (let r = 0; r < 8; r++) {
    let f = 0;
    for (const ch of ranks[r]) {
      if (ch >= "1" && ch <= "8") {
        f += Number(ch);
      } else {
        if (f < 0 || f > 7) return null;
        board[r][f] = ch;
        f++;
      }
    }
    if (f !== 8) return null;
  }
  return board; // rank 8 at index 0
}

function clearSelectionStyles() {
  document.querySelectorAll(".square.selected").forEach(el => el.classList.remove("selected"));
}

function setSelectedSquare(square) {
  clearSelectionStyles();
  const el = document.querySelector(`.square[data-square="${square}"]`);
  if (el) el.classList.add("selected");
}

function getPieceAtSquareFromFen(fen, square) {
  // Uses fenToBoardArray for correctness
  const b = fenToBoardArray(fen);
  if (!b) return null;

  const file = square.charCodeAt(0) - "a".charCodeAt(0); // 0..7
  const rank = square.charCodeAt(1) - "1".charCodeAt(0); // 0..7 (rank 1 => 0)
  const r = 7 - rank; // rank 8 => r=0
  const f = file;

  return b[r][f]; // FEN piece letter or null
}

function maybeAddPromotion(moveStr, fen) {
  // If a pawn moves to last rank, ask promotion.
  const toSq = moveStr.slice(2, 4);
  const piece = getPieceAtSquareFromFen(fen, moveStr.slice(0, 2));
  if (!piece) return moveStr;

  const isPawn = (piece === "P" || piece === "p");
  if (!isPawn) return moveStr;

  const toRank = toSq[1];
  const isPromotion = (piece === "P" && toRank === "8") || (piece === "p" && toRank === "1");
  if (!isPromotion) return moveStr;

  const promo = prompt("Promote to (q/r/b/n). Default q:", "q");
  const p = (promo || "q").trim().toLowerCase();
  if (!["q", "r", "b", "n"].includes(p)) return moveStr + "q";
  return moveStr + p;
}

function renderBoardFromFen(fen) {
  const boardEl = $("board");
  const b = fenToBoardArray(fen);

  boardEl.innerHTML = "";

  for (let r = 0; r < 8; r++) {
    for (let f = 0; f < 8; f++) {
      const sq = document.createElement("div");
      sq.className = "square " + (((r + f) % 2 === 0) ? "light" : "dark");

      // Map (r,f) where r=0 is rank 8, to algebraic
      const fileChar = String.fromCharCode("a".charCodeAt(0) + f);
      const rankChar = String.fromCharCode("8".charCodeAt(0) - r);
      const alg = fileChar + rankChar;
      sq.dataset.square = alg;

      const piece = b ? b[r][f] : null;
      sq.textContent = piece ? (PIECE_GLYPHS[piece] || piece) : "";

      boardEl.appendChild(sq);
    }
  }
}

function updateStatusFromEngine(engine) {
  const s = $("status");

  if (!engine) {
    s.textContent = "No engine state.";
    return;
  }
  if (engine.event === "error") {
    s.textContent = "Error: " + (engine.message || "Unknown");
    return;
  }

  const stm = engine.side_to_move === "w" ? "White" : "Black";

  if (engine.status === "ongoing") {
    s.textContent = `Side to move: ${stm}`;
  } else {
    s.textContent = `Game over: ${engine.status}`;
  }
}

async function apiNewGame(humanColor = "w") {
  const res = await fetch(`/api/new-game?human_color=${encodeURIComponent(humanColor)}`, { method: "POST" });
  const data = await res.json();
  if (!data.ok) throw new Error(data.detail || "new-game failed");
  return data.engine;
}

async function apiMove(move) {
  const res = await fetch("/api/move", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ move }),
  });
  const data = await res.json();
  if (!data.ok) throw new Error(data.detail || "move failed");
  return data.engine;
}

async function newGame() {
  $("status").textContent = "Starting...";
  updateOutput("");

  try {
    const eng = await apiNewGame("w"); // you can make this selectable later
    if (eng.event === "error") throw new Error(eng.message || "Engine error");

    lastFen = eng.fen;
    renderBoardFromFen(lastFen);
    updateStatusFromEngine(eng);

    // Debug payload
    updateOutput(JSON.stringify(eng, null, 2));
  } catch (e) {
    $("status").textContent = "Error: " + e.message;
  }

  focusMoveInput();
}

async function sendMove() {
  const moveInput = $("moveInput");
  const move = moveInput.value.trim();
  if (!move) return;

  try {
    const eng = await apiMove(move);

    if (eng.event === "error") {
      $("status").textContent = "Illegal move: " + (eng.message || "");
      return;
    }

    lastFen = eng.fen;
    renderBoardFromFen(lastFen);
    updateStatusFromEngine(eng);

    // Debug payload
    updateOutput(JSON.stringify(eng, null, 2));

    moveInput.value = "";
  } catch (e) {
    $("status").textContent = "Error: " + e.message;
  }

  focusMoveInput();
}

document.addEventListener("DOMContentLoaded", () => {
  $("backBtn").addEventListener("click", () => (window.location.href = "/"));
  $("newGameBtn").addEventListener("click", newGame);
  $("submitBtn").addEventListener("click", sendMove);

  $("moveInput").addEventListener("keydown", (e) => {
    if (e.key === "Enter") sendMove();
  });

  $("board").addEventListener("click", async (e) => {
    const sqEl = e.target.closest(".square");
    if (!sqEl) return;

    const sq = sqEl.dataset.square;
    if (!sq) return;

    // If no game loaded yet, ignore
    if (!lastFen) return;

    if (!selectedFrom) {
      selectedFrom = sq;
      setSelectedSquare(sq);
      $("moveInput").value = sq; // optional
      return;
    }

    const moveBase = selectedFrom + sq; // e2e4
    const move = maybeAddPromotion(moveBase, lastFen);

    selectedFrom = null;
    clearSelectionStyles();
    $("moveInput").value = move;

    // Send immediately
    await sendMove(); // uses moveInput.value
  });


  // Render an empty board initially
  renderBoardFromFen("8/8/8/8/8/8/8/8 w - - 0 1");
  focusMoveInput();
});

