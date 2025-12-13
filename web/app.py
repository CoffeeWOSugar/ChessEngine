# web/app.py
from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
import os
import json
import subprocess
import threading


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

def _start_engine_protocol():
    global engine_proc
    _kill_engine()

    engine_proc = subprocess.Popen(
        [ENGINE_PATH, "--protocol"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    if not engine_proc.stdin or not engine_proc.stdout:
        raise RuntimeError("Failed to open engine stdio")

def _rpc(cmd: dict) -> dict:
    if not engine_proc or engine_proc.poll() is not None:
        raise RuntimeError("Engine not running")

    assert engine_proc.stdin and engine_proc.stdout

    engine_proc.stdin.write(json.dumps(cmd) + "\n")
    engine_proc.stdin.flush()

    line = engine_proc.stdout.readline()
    if not line:
        raise RuntimeError("Engine terminated")
    if line == "":
        rc = engine_proc.poll()
        raise RuntimeError(f"Engine terminated or produced no output (returncode={rc}).")
    try:
        return json.loads(line)
    except json.JSONDecodeError:
        raise RuntimeError(f"Engine returned non-JSON: {line!r}")


# --------- API MODELS ---------

class MoveRequest(BaseModel):
    move: str

# --------- API ENDPOINTS ---------

@app.post("/api/new-game")
def api_new_game(human_color: str = "w"):
    with engine_lock:
        try:
            _start_engine_protocol()
            res = _rpc({"cmd": "new-game", "human_color": human_color})
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))
    return {"ok": True, "engine": res}

@app.post("/api/move")
def api_move(req: MoveRequest):
    with engine_lock:
        if not engine_proc or engine_proc.poll() is not None:
            raise HTTPException(status_code=400, detail="No active game. Start a new game first.")
        try:
            res = _rpc({"cmd": "move", "move": req.move})
        except Exception as e:
            raise HTTPException(status_code=500, detail=str(e))
    return {"ok": True, "engine": res}

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
