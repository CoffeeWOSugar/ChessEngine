from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from fastapi.responses import HTMLResponse
import subprocess
import os

app = FastAPI()

# Serve static files
app.mount("/static", StaticFiles(directory="web/static"), name="static")


@app.get("/", response_class=HTMLResponse)
def index():
    index_path = os.path.join("web", "static", "index.html")
    with open(index_path, "r", encoding="utf-8") as f:
        return f.read()


@app.get("/perft")
def perft(depth: int = 3):
    """
    Call the C++ engine: chess --perft <depth>
    and return {"depth": depth, "nodes": <value>}
    """
    try:
        result = subprocess.run(
            ["chess", "--perft", str(depth)],
            capture_output=True,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        raise HTTPException(
            status_code=500,
            detail=f"Engine error: {e.stderr or e.stdout or 'no output'}",
        )

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

