"""
KIND: offline tool (calls the Meshy API, writes models and a ledger under Art/Models).

Makes the game's 3D models through the Meshy API directly (the author, Sat 14 Nov 21:00: fal is for 2D images only):
image-to-3D, then the automatic rig, then library clips on that rig. Every task is booked in Art/Models/ledger.json
with the credits Meshy reports it consumed; a round whose planned credits would take the booked Meshy total over the
cap, or past the account balance, is refused before anything is sent.

    python Tools/Art/meshy_models.py <jobs.json> [--max-credits 200] [--dry-run]
    python Tools/Art/meshy_models.py --balance
    python Tools/Art/meshy_models.py --stats

jobs.json, in dependency order:
    {"name": "hero", "kind": "image-to-3d", "image": "Art/Concepts/r1_hero_a.png",
     "input": {"ai_model": "meshy-7.1", "pose_mode": "t-pose", "should_remesh": true, "target_polycount": 20000}}
    {"name": "hero_rig", "kind": "rigging", "from": "hero", "input": {"height_meters": 1.8}}
    {"name": "hero_clips", "kind": "animations", "from": "hero_rig", "input": {"action_ids": [0, 219, 178, 8]}}
"image" is sent as a data URI; "model_file" sends a local GLB to the rig as its model_url; "from" is an earlier job whose Meshy task id the job builds on (input_task_id for the
rig, rig_task_id for the clips). A job waits for its "from" job and is skipped when Art/Models/<name>/task.json exists.
Every file of the finished task is saved to Art/Models/<name>/. The key is MESHY_API_KEY in .env and is never printed.
"""
import argparse
import base64
import json
import os
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor

import model_files

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "Art", "Models")
LEDGER = os.path.join(OUT, "ledger.json")
API = "https://api.meshy.ai/openapi/v1/"
ROUTE = {"image-to-3d": "image-to-3d", "rigging": "rigging", "animations": "animations"}
FROM_FIELD = {"rigging": "input_task_id", "animations": "rig_task_id"}
LOCK = threading.Lock()


def planned_credits(job):
    """Credits of a job by the Meshy price page (docs.meshy.ai/en/api/pricing, read Sat 14 Nov): meshy-7.1 and
    meshy-6 image-to-3D 20 for the mesh, 30 with a 2K or 4K texture, 35 at 8K, +5 for 2k/4k geometry; a rig 5; 3 per clip."""
    data = job.get("input", {})
    if job["kind"] == "image-to-3d":
        credits = 20
        if data.get("should_texture", True):
            credits = 35 if data.get("texture_resolution") == "8k" else 30
        if data.get("geometry_resolution") in ("2k", "4k"):
            credits += 5
        return credits
    if job["kind"] == "rigging":
        return 5
    return 3 * len(data.get("action_ids", [data.get("action_id")]))


def refusal(booked, planned, cap, balance):
    """Why a round must not be sent, or "" when it may."""
    if booked + planned > cap:
        return "the round would take the booked %d credits to %d, over the cap of %d" % (booked, booked + planned, cap)
    if planned > balance:
        return "the round needs %d credits, the account has %d" % (planned, balance)
    return ""


def read_key():
    with open(os.path.join(ROOT, ".env"), encoding="utf-8") as h:
        for line in h:
            if line.startswith("MESHY_API_KEY="):
                return line.split("=", 1)[1].strip()
    sys.exit("[meshy] MESHY_API_KEY is missing in .env")


def headers(key):
    return {"Authorization": "Bearer " + key, "Content-Type": "application/json"}


def balance(key):
    import requests

    r = requests.get(API + "balance", headers=headers(key), timeout=60)
    r.raise_for_status()
    return r.json()["balance"]


def book(row):
    with LOCK:
        rows = []
        if os.path.exists(LEDGER):
            with open(LEDGER, encoding="utf-8") as h:
                rows = json.load(h)
        rows.append(row)
        with open(LEDGER, "w", encoding="utf-8", newline="\n") as h:
            json.dump(rows, h, indent=1)


def booked():
    """Meshy credits booked so far; the fal rows of the same ledger are in USD and are not counted."""
    if not os.path.exists(LEDGER):
        return 0
    with open(LEDGER, encoding="utf-8") as h:
        return sum(r.get("credits", 0) for r in json.load(h) if r.get("tool") == "meshy")


def task_path(name):
    return os.path.join(OUT, name, "task.json")


def wait_for(name, failed, seconds=1800):
    started = time.time()
    while not os.path.exists(task_path(name)):
        if name in failed:
            raise RuntimeError("its 'from' job %s failed" % name)
        if time.time() - started > seconds:
            raise RuntimeError("%s did not finish" % name)
        time.sleep(5)
    with open(task_path(name), encoding="utf-8") as h:
        return json.load(h)["id"]


def build_input(job, failed):
    payload = dict(job.get("input", {}))
    if "image" in job:
        with open(os.path.join(ROOT, job["image"]), "rb") as h:
            payload["image_url"] = "data:image/png;base64," + base64.b64encode(h.read()).decode("ascii")
    if "model_file" in job:
        with open(os.path.join(ROOT, job["model_file"]), "rb") as h:
            payload["model_url"] = "data:application/octet-stream;base64," + base64.b64encode(h.read()).decode("ascii")
    if "from" in job:
        payload[FROM_FIELD[job["kind"]]] = wait_for(job["from"], failed)
    return payload


def run_job(job, key, failed):
    import requests

    name = job["name"]
    route = API + ROUTE[job["kind"]]
    started = time.time()
    row = {"tool": "meshy", "name": name, "kind": job["kind"], "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
    try:
        r = requests.post(route, headers=headers(key), json=build_input(job, failed), timeout=120)
        if r.status_code >= 400:
            raise RuntimeError("create HTTP %d: %s" % (r.status_code, r.text[:300]))
        row["task_id"] = r.json()["result"]
        while True:
            time.sleep(10)
            task = requests.get(route + "/" + row["task_id"], headers=headers(key), timeout=60).json()
            if task.get("status") == "SUCCEEDED":
                break
            if task.get("status") not in ("PENDING", "IN_PROGRESS"):
                raise RuntimeError("status %s: %s" % (task.get("status"), json.dumps(task.get("task_error"))[:300]))
            if time.time() - started > 1800:
                raise RuntimeError("timeout after 1800 s")
        folder = os.path.join(OUT, name)
        os.makedirs(folder, exist_ok=True)
        for _, url, file_name in model_files.files_in(task):
            got = requests.get(url, timeout=300)
            got.raise_for_status()
            with open(os.path.join(folder, file_name), "wb") as h:
                h.write(got.content)
        # task.json last: its presence marks the job done for a rerun and for the jobs built on it.
        with open(task_path(name), "w", encoding="utf-8", newline="\n") as h:
            json.dump(task, h, indent=1)
        row.update(status="ok", credits=task.get("consumed_credits", planned_credits(job)), seconds=round(time.time() - started), input=job.get("input", {}))
        book(row)
        return name, "ok in %d s, %d credits" % (row["seconds"], row["credits"])
    except Exception as e:  # one failed job must not stop the round
        failed.add(name)
        row.update(status="failed", error=str(e)[:300], credits=0)
        book(row)
        return name, "FAILED: " + str(e)[:300]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("jobs", nargs="?")
    ap.add_argument("--max-credits", type=int, required=False)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--balance", action="store_true")
    ap.add_argument("--stats", action="store_true")
    args = ap.parse_args()
    if args.stats:
        model_files.print_stats(OUT)
        return
    key = read_key()
    if args.balance:
        print("[meshy] balance %d credits, booked in the ledger %d" % (balance(key), booked()))
        return
    if args.max_credits is None:
        sys.exit("[meshy] --max-credits is required: the author sets the cap")
    with open(args.jobs, encoding="utf-8") as h:
        jobs = json.load(h)
    os.makedirs(OUT, exist_ok=True)
    todo = [j for j in jobs if not os.path.exists(task_path(j["name"]))]
    planned = sum(planned_credits(j) for j in todo)
    have = balance(key)
    print("[meshy] %d jobs, %d to run, plan %d credits, booked %d, cap %d, balance %d" % (len(jobs), len(todo), planned, booked(), args.max_credits, have))
    why = refusal(booked(), planned, args.max_credits, have)
    if why:
        sys.exit("[meshy] nothing sent: " + why)
    if args.dry_run or not todo:
        return
    failed = set()
    with ThreadPoolExecutor(max_workers=max(1, len(todo))) as pool:
        for name, outcome in pool.map(lambda j: run_job(j, key, failed), todo):
            print("[meshy] %s: %s" % (name, outcome))
    print("[meshy] booked %d credits in all, balance %d" % (booked(), balance(key)))


if __name__ == "__main__":
    main()
