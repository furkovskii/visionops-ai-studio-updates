# VisionOps SceneFlow / TruthMeter / ActionLab - prototype v0.1

EXPERIMENTAL deterministic rules core only. Not wired to live camera loop or public updater. No false claim of a deployed product.

Users define a camera-observable process rule. TruthMeter treats missing observation as UNKNOWN, and ActionLab compares observed before/after periods without claims of causality.

Example rule:
{
  "rule_id": "queue_exit_v1", "module": "cafe", "camera_id": "cam_01",
  "zone_id": "queue", "subject": "person", "event": "zone_exit",
  "condition": "at_least", "threshold": 2, "window_seconds": 120,
  "minimum_coverage": 0.9
}

This detects zone exits only, not purchases, abandoned orders, or exits from a building.

Security and correctness rules:
- Café: anonymous person/area; Pet and Farm: animal/area only.
- No faces, identities, employee ratings, credentials or camera URLs.
- No arbitrary user expressions. Counts require usable observed camera windows.
- SHA256 digest is a reproducible local fingerprint, NOT a signature, measured model accuracy or forensic legal proof.
- Before/after comparison is descriptive, NOT proof of revenue impact, causality or animal health.

Before release:
1. Adapt existing camera/zone events and camera-health windows.
2. Calibrate physical-camera detection against independently labeled examples.
3. Bind owner-scoped rules to the Windows UI with auth, retention and audit trail.
4. Add cloud derived event receipts only after RLS/security/KVKK checks.
5. Connect mobile to approved receipts; mobile currently displays real authorized snapshots with explicit unknown status.
6. Test four real cameras and NVR, camera outages and module-specific access controls.

Run: python qa/sceneflow/test_sceneflow.py
