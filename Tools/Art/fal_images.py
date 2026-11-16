"""
KIND: offline tool (calls the fal queue API, writes images and a ledger under Art/).

Generates concept images through fal with OpenAI GPT Image 2.6 (endpoint openai/gpt-image-2.6/flare/text-to-image).
Jobs run in parallel; every request is written to a ledger under a lock, so the spend of a round is on record.

    python Tools/Art/fal_images.py <jobs.json> [--out Art/Concepts] [--max-usd 5] [--dry-run]

jobs.json: [{"name": "hero_a", "prompt": "...", "image_size": {"width": 1024, "height": 1536}, "quality": "high"}]
A job with "refs" (local image paths) goes to the edit endpoint with those images, sent inline, as references.
A job whose image already exists is skipped, so a round can be rerun after a failure.
The key is read from FAL_KEY in .env and is never printed.
"""
import argparse
import base64
import json
import os
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor

import requests

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
ENDPOINT = "openai/gpt-image-2.6/flare/text-to-image"
EDIT_ENDPOINT = "openai/gpt-image-2.6/flare/edit"
# fal bills this model by tokens and does not return the cost. The launch page quotes 0.04 to 0.053 USD per image
# at high quality; the ledger books a pessimistic flat estimate.
EST_USD = {"low": 0.03, "medium": 0.06, "high": 0.10}
LOCK = threading.Lock()


def read_key():
    with open(os.path.join(ROOT, ".env"), encoding="utf-8") as h:
        for line in h:
            if line.startswith("FAL_KEY="):
                return line.split("=", 1)[1].strip()
    sys.exit("[fal] FAL_KEY is missing in .env")


def book(ledger_path, row):
    with LOCK:
        rows = []
        if os.path.exists(ledger_path):
            with open(ledger_path, encoding="utf-8") as h:
                rows = json.load(h)
        rows.append(row)
        with open(ledger_path, "w", encoding="utf-8", newline="\n") as h:
            json.dump(rows, h, indent=1)


def spent(ledger_path):
    if not os.path.exists(ledger_path):
        return 0.0
    with open(ledger_path, encoding="utf-8") as h:
        return sum(r.get("est_usd", 0.0) for r in json.load(h) if r.get("status") == "ok")


def data_uri(path):
    with open(path, "rb") as h:
        return "data:image/png;base64," + base64.b64encode(h.read()).decode("ascii")


def run_job(job, key, out, ledger_path):
    name = job["name"]
    target = os.path.join(out, name + ".png")
    if os.path.exists(target):
        return name, "skipped (exists)"
    quality = job.get("quality", "high")
    payload = {
        "prompt": job["prompt"],
        "image_size": job.get("image_size", "square_hd"),
        "quality": quality,
        "reasoning_effort": job.get("reasoning_effort", "low"),
        "num_images": 1,
        "output_format": "png",
    }
    endpoint = ENDPOINT
    if job.get("refs"):
        endpoint = EDIT_ENDPOINT
        payload["image_urls"] = [data_uri(path) for path in job["refs"]]
    headers = {"Authorization": "Key " + key, "Content-Type": "application/json"}
    started = time.time()
    row = {"name": name, "endpoint": endpoint, "refs": job.get("refs", []), "quality": quality, "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
    try:
        r = requests.post("https://queue.fal.run/" + endpoint, headers=headers, json=payload, timeout=60)
        if r.status_code >= 400:
            raise RuntimeError("submit HTTP %d: %s" % (r.status_code, r.text[:300]))
        queued = r.json()
        row["request_id"] = queued.get("request_id", "")
        while True:
            time.sleep(3)
            status = requests.get(queued["status_url"], headers=headers, timeout=60).json()
            if status.get("status") == "COMPLETED":
                break
            if status.get("status") not in ("IN_QUEUE", "IN_PROGRESS"):
                raise RuntimeError("status: %s" % json.dumps(status)[:300])
            if time.time() - started > 600:
                raise RuntimeError("timeout after 600 s")
        result = requests.get(queued["response_url"], headers=headers, timeout=60)
        if result.status_code >= 400:
            raise RuntimeError("result HTTP %d: %s" % (result.status_code, result.text[:300]))
        image = requests.get(result.json()["images"][0]["url"], timeout=120)
        image.raise_for_status()
        with open(target, "wb") as h:
            h.write(image.content)
        row.update(status="ok", est_usd=EST_USD.get(quality, 0.10), seconds=round(time.time() - started), prompt=job["prompt"])
        book(ledger_path, row)
        return name, "ok in %d s" % row["seconds"]
    except Exception as e:  # one failed job must not stop the round
        row.update(status="failed", error=str(e)[:300], est_usd=0.0)
        book(ledger_path, row)
        return name, "FAILED: " + str(e)[:300]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("jobs")
    ap.add_argument("--out", default=os.path.join(ROOT, "Art", "Concepts"))
    ap.add_argument("--max-usd", type=float, default=5.0)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    with open(args.jobs, encoding="utf-8") as h:
        jobs = json.load(h)
    os.makedirs(args.out, exist_ok=True)
    ledger_path = os.path.join(args.out, "ledger.json")
    todo = [j for j in jobs if not os.path.exists(os.path.join(args.out, j["name"] + ".png"))]
    planned = sum(EST_USD.get(j.get("quality", "high"), 0.10) for j in todo)
    already = spent(ledger_path)
    print("[fal] %d jobs, %d to run, estimate %.2f USD, booked so far %.2f USD, cap %.2f USD" % (len(jobs), len(todo), planned, already, args.max_usd))
    if already + planned > args.max_usd:
        sys.exit("[fal] the round would exceed the cap, nothing sent")
    if args.dry_run or not todo:
        return
    key = read_key()
    with ThreadPoolExecutor(max_workers=6) as pool:
        for name, outcome in pool.map(lambda j: run_job(j, key, args.out, ledger_path), todo):
            print("[fal] %s: %s" % (name, outcome))
    print("[fal] booked total %.2f USD (estimate)" % spent(ledger_path))


if __name__ == "__main__":
    main()
