# web/app.py
from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
import subprocess
import threading
import os

app = FastAPI()

# Serve static files (HTML/CSS/JS)
app.mount("/static", StaticFiles(directory="web/static"), name="static")


def read_html(name: str) -> str:
    path = os.path.join("web", "static", name)
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


# --------- PAGES ---------

@app.get("/", response_class=HTMLResponse)
def index():
    return read_html("index.html")


@app.get("/game", response_class=HTMLResponse)
def game_page():
    return read_html("game.html")


@app.get("/tests", response_class=HTMLResponse)
def tests_page():
    return read_html("tests.html")


# --------- ENGINE PROCESS MANAGEMENT ---------

engine_proc: subprocess.Popen | None = None
engine_lock = threading.Lock()

ENGINE_PATH = "/usr/local/bin/chess"  # where Docker copies it


def _kill_engine():
    global engine_proc
    if engine_proc and engine_proc.poll() is None:
        try:
            engine_proc.terminate()
        except Exception:
            pass
    engine_proc = None


def _start_engine(human_color: str = "w") -> str:
    """
    Start a fresh 'chess' process, send chosen color immediately,
    read until the first 'Your move' (or game-over) appears,
    and return all text output.
    """
    global engine_proc
    _kill_engine()

    try:
        engine_proc = subprocess.Popen(
            [ENGINE_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
    except FileNotFoundError:
        raise RuntimeError(f"Engine binary not found at {ENGINE_PATH}")
    except Exception as e:
        raise RuntimeError(f"Failed to start engine: {e}")

    if not engine_proc.stdin or not engine_proc.stdout:
        raise RuntimeError("Failed to open engine stdio")

    # Send color immediately ("w" or "b")
    color_char = "w" if human_color.lower().startswith("w") else "b"
    engine_proc.stdin.write(color_char + "\n")
    engine_proc.stdin.flush()

    lines: list[str] = []

    # Read lines until we see "Your move" or a terminal message
    while True:
        line = engine_proc.stdout.readline()
        if line == "":
            # Process ended
            break
        lines.append(line)

        if "Your move" in line:
            break
        if "Checkmate" in line or "Stalemate" in line or "Goodbye" in line:
            break

        # Safety: don't read unbounded in case something is wrong
        if len(lines) > 500:
            break

    return "".join(lines)


def _send_move_to_engine(move_str: str) -> str:
    """
    Send a move line (e.g. 'e2e4') to the running engine,
    then read output until next 'Your move' or game-over.
    """
    if not engine_proc or engine_proc.poll() is not None:
        raise RuntimeError("Engine process is not running")

    if not engine_proc.stdin or not engine_proc.stdout:
        raise RuntimeError("Engine stdio not available")

    # Send the move
    engine_proc.stdin.write(move_str.strip() + "\n")
    engine_proc.stdin.flush()

    lines: list[str] = []

    while True:
        line = engine_proc.stdout.readline()
        if line == "":
            # Engine exited
            break
        lines.append(line)

        if "Your move" in line:
            break
        if "Checkmate" in line or "Stalemate" in line or "Goodbye" in line:
            break

        if len(lines) > 500:
            break

    return "".join(lines)


# --------- API MODELS ---------

class MoveRequest(BaseModel):
    move: str


# --------- API ENDPOINTS ---------

@app.post("/api/new-game")
def api_new_game(human_color: str = "w"):
    """
    Start a new game in the engine and return the initial text output:
    board + engine messages + first 'Your move' prompt.
    """
    with engine_lock:
        try:
            text = _start_engine(human_color)
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))

    return {"ok": True, "output": text}


@app.post("/api/move")
def api_move(req: MoveRequest):
    """
    Send a move like 'e2e4' to the engine, return resulting text:
    updated board, engine reply, and next prompt or game-over message.
    """
    with engine_lock:
        if not engine_proc or engine_proc.poll() is not None:
            raise HTTPException(status_code=400, detail="No active game. Start a new game first.")
        try:
            text = _send_move_to_engine(req.move)
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))

    return {"ok": True, "output": text}


@app.post("/api/run-tests")
def run_tests():
    """
    Run the engine's built-in tests: chess --run-tests
    """
    try:
        result = subprocess.run(
            [ENGINE_PATH, "--run-tests"],
            capture_output=True,
            text=True,
            check=False,
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to run tests: {e}")

    output = (result.stdout or "") + (result.stderr or "")
    return {
        "returncode": result.returncode,
        "output": output,
        "success": result.returncode == 0,
    }


@app.get("/perft")
def perft(depth: int = 3):
    """
    Debug endpoint: chess --perft <depth>
    """
    try:
        result = subprocess.run(
            [ENGINE_PATH, "--perft", str(depth)],
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        raise HTTPException(
            status_code=500,
            detail=f"Engine error: {e.stderr or e.stdout or 'no output'}",
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to run engine: {e}")

    out = result.stdout.strip()
    if not out:
        raise HTTPException(status_code=500, detail="Empty engine output")

    last_line = out.splitlines()[-1]
    try:
        nodes = int(last_line)
    except ValueError:
        raise HTTPException(
            status_code=500,
            detail=f"Unexpected engine output: {out!r}",
        )

    return {"depth": depth, "nodes": nodes}
