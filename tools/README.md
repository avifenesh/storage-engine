# Metrics and Reporting

This folder contains a minimal, study‑friendly way to record and share your sprint results without giving away solutions.

- `metrics_template.json`: A template with fields aligned to the “Quality Gates & Targets” in the sprint docs.
- `metrics.json`: Your actual metrics (created from the template on first `make report`).
- `generate_report.sh`: Prints a readable report from `metrics.json`.

How to use
- Run your harnesses and record measurements (throughput, p50/p99, correctness flags).
- Edit `metrics.json` to fill in the measured values.
- Generate the report: `make report` or `bash tools/generate_report.sh`.

Tips for accurate numbers
- Pin CPU cores and disable turbo if you want stability.
- Warm up caches first; report steady‑state numbers.
- Report both single‑thread and multi‑thread results where requested.
- Note hardware and kernel versions alongside numbers.

No solutions are provided here; this only helps you quantify what you’ve achieved.
