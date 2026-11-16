"""
KIND: offline tool, RETIRED for sending (the author, Sat 14 Nov 21:00: fal is for 2D images only; 3D models and
rigging go through the Meshy API, Tools/Art/meshy_models.py). Kept as the record of the one fal comparison round
(Art/Models/round1_jobs.json: boss, katana, shuriken, cash_stack) and for --stats.

Makes 3D models through fal: image-to-3D (Meshy 7.1, Hunyuan3D v3) and the Meshy rig with library clips. Jobs run in
parallel; every request is booked in Art/Models/ledger.json under a lock, and a round that would take the booked total
over the cap is refused before anything is sent.

    python Tools/Art/fal_models.py <jobs.json> [--max-usd 12] [--dry-run]
    python Tools/Art/fal_models.py --stats             # polycount and rig of every GLB under Art/Models

jobs.json: [{"name": "hero", "endpoint": "meshy/v7.1/image-to-3d", "image": "Art/Concepts/r1_hero_a.png",
             "est_usd": 1.2, "input": {"target_polycount": 20000, "pose_mode": "t-pose"}},
            {"name": "hero_rig", "endpoint": "fal-ai/meshy/rigging/multi-animation", "model_from": "hero",
             "est_usd": 1.04, "input": {"animation_action_ids": [0, 219]}}]
"image" is a local file sent as a data URI in the endpoint's image field; "model_from" takes the GLB of an earlier job.
fal returns no cost: "est_usd" is the price on the model's fal page, booked as is. Every file of the result is saved
to Art/Models/<name>/, with result.json; a job whose result.json exists is skipped. The key is FAL_KEY in .env.
"""
import argparse
import base64
import json
import os
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor

from model_files import files_in, print_stats

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "Art", "Models")
LEDGER = os.path.join(OUT, "ledger.json")
# The field each endpoint reads its picture from.
IMAGE_FIELD = {"meshy/v7.1/image-to-3d": "image_url", "fal-ai/hunyuan3d-v3/image-to-3d": "input_image_url"}
LOCK = threading.Lock()


def over_cap(spent, planned, cap):
    return spent + planned > cap + 1e-9


def read_key():
    with open(os.path.join(ROOT, ".env"), encoding="utf-8") as h:
        for line in h:
            if line.startswith("FAL_KEY="):
                return line.split("=", 1)[1].strip()
    sys.exit("[fal] FAL_KEY is missing in .env")


def book(row):
    with LOCK:
        rows = []
        if os.path.exists(LEDGER):
            with open(LEDGER, encoding="utf-8") as h:
                rows = json.load(h)
        rows.append(row)
        with open(LEDGER, "w", encoding="utf-8", newline="\n") as h:
            json.dump(rows, h, indent=1)


def spent():
    if not os.path.exists(LEDGER):
        return 0.0
    with open(LEDGER, encoding="utf-8") as h:
        return sum(r.get("est_usd", 0.0) for r in json.load(h) if r.get("status") == "ok")


def data_uri(path):
    with open(os.path.join(ROOT, path), "rb") as h:
        return "data:image/png;base64," + base64.b64encode(h.read()).decode("ascii")


def build_input(job):
    payload = dict(job.get("input", {}))
    if "image" in job:
        payload[IMAGE_FIELD[job["endpoint"]]] = data_uri(job["image"])
    if "model_from" in job:
        with open(os.path.join(OUT, job["model_from"], "result.json"), encoding="utf-8") as h:
            payload["model_url"] = json.load(h)["model_glb"]["url"]
    return payload


def run_job(job, key):
    import requests

    name = job["name"]
    folder = os.path.join(OUT, name)
    headers = {"Authorization": "Key " + key, "Content-Type": "application/json"}
    started = time.time()
    row = {"name": name, "endpoint": job["endpoint"], "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
    try:
        r = requests.post("https://queue.fal.run/" + job["endpoint"], headers=headers, json=build_input(job), timeout=120)
        if r.status_code >= 400:
            raise RuntimeError("submit HTTP %d: %s" % (r.status_code, r.text[:300]))
        queued = r.json()
        row["request_id"] = queued.get("request_id", "")
        while True:
            time.sleep(5)
            status = requests.get(queued["status_url"], headers=headers, timeout=60).json()
            if status.get("status") == "COMPLETED":
                break
            if status.get("status") not in ("IN_QUEUE", "IN_PROGRESS"):
                raise RuntimeError("status: %s" % json.dumps(status)[:300])
            if time.time() - started > 1500:
                raise RuntimeError("timeout after 1500 s")
        result = requests.get(queued["response_url"], headers=headers, timeout=60)
        if result.status_code >= 400:
            raise RuntimeError("result HTTP %d: %s" % (result.status_code, result.text[:300]))
        body = result.json()
        os.makedirs(folder, exist_ok=True)
        for _, url, file_name in files_in(body):
            got = requests.get(url, timeout=300)
            got.raise_for_status()
            with open(os.path.join(folder, file_name), "wb") as h:
                h.write(got.content)
        # result.json last: its presence marks the job done.
        with open(os.path.join(folder, "result.json"), "w", encoding="utf-8", newline="\n") as h:
            json.dump(body, h, indent=1)
        row.update(status="ok", est_usd=job["est_usd"], seconds=round(time.time() - started), input=job.get("input", {}))
        book(row)
        return name, "ok in %d s" % row["seconds"]
    except Exception as e:  # one failed job must not stop the round
        row.update(status="failed", error=str(e)[:300], est_usd=0.0)
        book(row)
        return name, "FAILED: " + str(e)[:300]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("jobs", nargs="?")
    ap.add_argument("--max-usd", type=float, default=12.0)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--stats", action="store_true")
    args = ap.parse_args()
    if args.stats:
        print_stats(OUT)
        return
    sys.exit("[fal] retired: 3D models and rigging go through Tools/Art/meshy_models.py")
    with open(args.jobs, encoding="utf-8") as h:
        jobs = json.load(h)
    os.makedirs(OUT, exist_ok=True)
    todo = [j for j in jobs if not os.path.exists(os.path.join(OUT, j["name"], "result.json"))]
    planned = sum(j["est_usd"] for j in todo)
    already = spent()
    print("[fal] %d jobs, %d to run, estimate %.2f USD, booked so far %.2f USD, cap %.2f USD" % (len(jobs), len(todo), planned, already, args.max_usd))
    if over_cap(already, planned, args.max_usd):
        sys.exit("[fal] the round would exceed the cap, nothing sent")
    if args.dry_run or not todo:
        return
    key = read_key()
    with ThreadPoolExecutor(max_workers=6) as pool:
        for name, outcome in pool.map(lambda j: run_job(j, key), todo):
            print("[fal] %s: %s" % (name, outcome))
    print("[fal] booked total %.2f USD (estimate)" % spent())


if __name__ == "__main__":
    main()
