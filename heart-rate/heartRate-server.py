# server.py
import os
from pathlib import Path
from datetime import datetime
import io, wave

from fastapi import FastAPI, Body, Request
from fastapi.responses import FileResponse, JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from fastapi.openapi.utils import get_openapi
from dotenv import load_dotenv

# ── Gemini ─────────────────────────────────────────
import google.generativeai as genai

# Forzar ffmpeg embebido (para pydub)
try:
    import imageio_ffmpeg
    _ffmpeg_path = imageio_ffmpeg.get_ffmpeg_exe()
    os.environ["PATH"] = os.pathsep.join([os.path.dirname(_ffmpeg_path), os.environ.get("PATH", "")])
    os.environ["FFMPEG_BINARY"] = _ffmpeg_path
    os.environ["FFPROBE_BINARY"] = _ffmpeg_path
    print(f"[FFMPEG] Using { _ffmpeg_path }")
except Exception as e:
    print(f"[FFMPEG][WARN] {e}")

from pydub import AudioSegment
from pydub.utils import which as _which
import pyttsx3

# pydub -> ffmpeg
try:
    _ff = os.environ.get("FFMPEG_BINARY") or _which("ffmpeg")
    if _ff:
        AudioSegment.converter = _ff
        AudioSegment.ffmpeg = _ff
        print(f"[FFMPEG] pydub using: {AudioSegment.ffmpeg}")
except Exception as e:
    print(f"[FFMPEG][WARN] {e}")

# ── Config ─────────────────────────────────────────
BASE_DIR = Path(__file__).parent.resolve()
OUT_DIR = BASE_DIR / "out"
OUT_DIR.mkdir(exist_ok=True)

load_dotenv()
API_KEY = (os.getenv("GEMINI_API_KEY") or "").strip()
if not API_KEY:
    raise RuntimeError("Falta GEMINI_API_KEY en .env")

genai.configure(api_key=API_KEY)
TXT_MODEL_ID = "models/gemini-1.5-flash"

app = FastAPI(title="ExpoCENFO Voice Tutor Server", version="0.4.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], allow_credentials=True,
    allow_methods=["*"], allow_headers=["*"],
)

def ok(d: dict): return JSONResponse({"ok": True, **d})
def err(msg: str, code: int = 400): return JSONResponse({"ok": False, "error": msg}, status_code=code)

# ── Audio utils ────────────────────────────────────
def tts_to_wav8(text: str) -> Path:
    """
    TTS con pyttsx3 -> WAV temporal -> remuestrea a 8kHz/8-bit/mono (para ESP32 DAC).
    """
    ts = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    tmp_wav = OUT_DIR / f"tmp_{ts}.wav"
    out_wav = OUT_DIR / f"tts8k_{ts}.wav"

    engine = pyttsx3.init()
    try:
        for v in engine.getProperty("voices"):
            if "span" in v.name.lower():  # voz en español si existe
                engine.setProperty("voice", v.id)
                break
    except Exception:
        pass
    engine.setProperty("rate", 170)   # 160–190 suena bien
    engine.setProperty("volume", 1.0)

    engine.save_to_file(text, str(tmp_wav))
    engine.runAndWait()

    snd = AudioSegment.from_file(tmp_wav)
    snd = snd.set_channels(1).set_frame_rate(8000).set_sample_width(1)  # 8-bit mono 8k
    snd.export(out_wav, format="wav")
    try: tmp_wav.unlink(missing_ok=True)
    except Exception: pass
    return out_wav

# ── Rutas existentes ───────────────────────────────
@app.get("/ping")
def ping(): return ok({"msg": "pong"})

@app.post("/reply")
async def reply(payload: dict = Body(...)):
    user_text = (payload.get("text") or "").strip()
    if not user_text:
        return err("Falta 'text'")
    try:
        wav_path = tts_to_wav8(user_text)
        return ok({"audio_url": f"/out/{wav_path.name}"})
    except Exception as e:
        return err(f"TTS falló: {e}", 500)

@app.get("/out/{fname}")
def serve_audio(fname: str):
    path = OUT_DIR / fname
    if not path.exists(): return err("Archivo no encontrado", 404)
    return FileResponse(str(path), media_type="audio/wav")

# ── NUEVO: /coach (genera texto con Gemini según BPM + TTS) ─────────────
@app.post("/coach")
async def coach(payload: dict = Body(...)):
    bpm = int(payload.get("bpm", 0))
    avg = int(payload.get("avg", bpm or 0))
    scenario = "normal"
    if avg <= 0:
        scenario = "desconocido"
    elif avg < 50:
        scenario = "bajo"
    elif avg > 110:
        scenario = "alto"

    sys = (
        "Eres un acompañante cálido en español latino. "
        "Habla en 2–3 frases (6–10 segundos), con voz calmada, sin términos médicos, "
        "evita dar diagnósticos; guía una respiración sencilla (4-2-6), "
        "y valida emociones. Mantén tono sereno y seguro."
    )
    user = (
        f"El pulso promedio actual es {avg} latidos por minuto, considerado '{scenario}'. "
        "Genera un mensaje breve y afectuoso para la persona en primera persona plural."
    )

    try:
        model = genai.GenerativeModel(TXT_MODEL_ID)
        resp = model.generate_content([sys, user])
        text = (resp.text or "").strip()
    except Exception as e:
        text = ""

    if not text:
        if scenario == "alto":
            text = "Respiremos juntas. Inhala cuatro, sostén dos, exhala seis. Estás a salvo, yo te acompaño. Todo va a estar bien."
        elif scenario == "bajo":
            text = "Vamos con calma. Inhala suave por cuatro, sostén dos, exhala por seis. Si te sientes rara, siéntate y descansa. Estoy contigo."
        else:
            text = "Todo va bien. Sigamos con una respiración tranquila: inhala cuatro, sostén dos, exhala seis. Estoy aquí contigo."

    wav_path = tts_to_wav8(text)
    return ok({"text": text, "audio_url": f"/out/{wav_path.name}"})

# ── OpenAPI ────────────────────────────────────────
def custom_openapi():
    if app.openapi_schema: return app.openapi_schema
    app.openapi_schema = get_openapi(
        title=app.title, version=app.version,
        description="STT + LLM + TTS para ESP32 (PCM crudo o WAV) + Coach BPM",
        routes=app.routes
    )
    return app.openapi_schema
app.openapi = custom_openapi

print(f"[BOOT] {__file__}")
for r in app.routes:
    if getattr(r, "path", None): print("[ROUTE]", r.path)
