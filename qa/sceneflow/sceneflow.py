"""VisionOps SceneFlow v0.1 - privacy-first, deterministic event rule engine.

Prototype core. This module NEVER opens cameras, calls cloud APIs, identifies
faces or invents unobserved events. It can be wired to VisionOps' existing
per-zone detector once real camera data is calibrated.

Terms:
- observation coverage is the time for which a camera provided usable frames,
  NOT a model's detection accuracy;
- lack of an event means zero only when the requested observation window has
  adequate coverage. Other cases are UNKNOWN;
- ActionLab before/after is descriptive and never establishes causality.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
from hashlib import sha256
import json
import math
import re
from typing import Iterable, Literal, Mapping

Module = Literal["cafe", "pet", "farm"]
Subject = Literal["person", "animal", "area"]
EventType = Literal["zone_enter", "zone_exit", "area_occupied", "object_visit"]
Condition = Literal["at_least", "at_most"]
Verdict = Literal["triggered", "not_triggered", "unknown"]
MODULE_SUBJECTS = {"cafe": {"person", "area"}, "pet": {"animal", "area"}, "farm": {"animal", "area"}}
ZONE = re.compile(r"^[a-zA-Z0-9_-]{1,64}$")
CAPTURE = re.compile(r"^[a-zA-Z0-9_-]{1,80}$")
EVENT_TYPES = {"zone_enter", "zone_exit", "area_occupied", "object_visit"}


@dataclass(frozen=True)
class Rule:
    rule_id: str
    module: Module
    camera_id: str
    zone_id: str
    subject: Subject
    event: EventType
    condition: Condition
    threshold: int
    window_seconds: int
    minimum_coverage: float = 0.9

    @classmethod
    def from_dict(cls, value: Mapping[str, object]) -> "Rule":
        if not isinstance(value, Mapping):
            raise ValueError("Rule must be an object.")
        fields = set(cls.__dataclass_fields__)
        if set(value) - fields:
            raise ValueError("Unknown rule fields are not permitted.")
        rule = cls(**value)  # type: ignore[arg-type]
        rule.validate()
        return rule

    def validate(self) -> None:
        if not ZONE.fullmatch(self.rule_id) or not ZONE.fullmatch(self.zone_id):
            raise ValueError("Invalid rule or zone identifier.")
        if not CAPTURE.fullmatch(self.camera_id):
            raise ValueError("Invalid camera identifier.")
        if self.module not in MODULE_SUBJECTS or self.subject not in MODULE_SUBJECTS[self.module]:
            raise ValueError("Human tracking must be restricted to Café; unsupported module/subject.")
        if self.event not in EVENT_TYPES:
            raise ValueError("Unsupported observation type.")
        if self.condition not in ("at_least", "at_most"):
            raise ValueError("Unsupported comparison.")
        if type(self.threshold) is not int or not 0 <= self.threshold <= 100000:
            raise ValueError("Threshold must be a bounded integer.")
        if type(self.window_seconds) is not int or not 30 <= self.window_seconds <= 86400:
            raise ValueError("Window must be 30 seconds to 24 hours.")
        if not isinstance(self.minimum_coverage, (float, int)) or isinstance(self.minimum_coverage, bool):
            raise ValueError("Coverage requirement must be numeric.")
        if not math.isfinite(self.minimum_coverage) or not 0.5 <= self.minimum_coverage <= 1.0:
            raise ValueError("Coverage requirement must be between 50% and 100%.")
        if self.subject == "person" and self.event == "object_visit":
            raise ValueError("Object visits are not human identity events.")


@dataclass(frozen=True)
class Event:
    timestamp: float
    camera_id: str
    module: Module
    zone_id: str
    subject: Subject
    kind: EventType

    def validate(self) -> None:
        if not math.isfinite(self.timestamp):
            raise ValueError("Invalid event time.")
        if not CAPTURE.fullmatch(self.camera_id) or not ZONE.fullmatch(self.zone_id):
            raise ValueError("Invalid event source.")
        if self.module not in MODULE_SUBJECTS or self.subject not in MODULE_SUBJECTS[self.module]:
            raise ValueError("Human events are not recorded in Pet/Farm.")
        if self.kind not in EVENT_TYPES:
            raise ValueError("Unknown event type.")


@dataclass(frozen=True)
class Coverage:
    start: float
    end: float
    camera_id: str
    usable: bool

    def validate(self) -> None:
        if not math.isfinite(self.start) or not math.isfinite(self.end) or self.end <= self.start:
            raise ValueError("Invalid coverage interval.")
        if not CAPTURE.fullmatch(self.camera_id) or type(self.usable) is not bool:
            raise ValueError("Invalid camera or coverage status.")


@dataclass(frozen=True)
class Receipt:
    rule_id: str
    module: Module
    camera_id: str
    window_start: float
    window_end: float
    observed_events: int
    covered_seconds: float
    coverage_ratio: float
    verdict: Verdict
    validated_accuracy: None
    reason: str
    proof_sha256: str

    def public(self) -> dict:
        return asdict(self)


def _canonical_receipt(fields: dict) -> str:
    """Tamper-evident local fingerprint, NOT a signature or legal proof."""
    return sha256(json.dumps(fields, sort_keys=True, ensure_ascii=False, separators=(",", ":")).encode("utf-8")).hexdigest()


def merged_intervals(coverage: Iterable[Coverage], camera_id: str, start: float, end: float) -> list[tuple[float, float]]:
    intervals = []
    for part in coverage:
        part.validate()
        if part.camera_id != camera_id or not part.usable:
            continue
        lo, hi = max(start, part.start), min(end, part.end)
        if lo < hi:
            intervals.append((lo, hi))
    intervals.sort()
    merged: list[list[float]] = []
    for lo, hi in intervals:
        if merged and lo <= merged[-1][1]:
            merged[-1][1] = max(hi, merged[-1][1])
        else:
            merged.append([lo, hi])
    return [(a, b) for a, b in merged]


def evaluate(rule: Rule, events: Iterable[Event], coverage: Iterable[Coverage],
             end: float, *, validated_accuracy: None = None) -> Receipt:
    """Evaluate a bounded time window without treating missing camera time as zero.

    Events only count when their timestamps are inside a known usable interval.
    Reject unexpected manual accuracy data: labeled calibration is a separate
    future feature, not inferred from classification confidence or coverage.
    """
    rule.validate()
    if validated_accuracy is not None:
        raise ValueError("Do not infer measured accuracy without an independent audit.")
    if not math.isfinite(end):
        raise ValueError("Invalid window timestamp.")
    start = end - rule.window_seconds
    ranges = merged_intervals(coverage, rule.camera_id, start, end)
    covered = sum(b - a for a, b in ranges)
    ratio = min(1.0, covered / rule.window_seconds)
    count = 0
    for event in events:
        event.validate()
        if (event.module == rule.module and event.camera_id == rule.camera_id
                and event.zone_id == rule.zone_id and event.subject == rule.subject
                and event.kind == rule.event and start <= event.timestamp < end
                and any(lo <= event.timestamp < hi for lo, hi in ranges)):
            count += 1

    if ratio + 1e-9 < rule.minimum_coverage:
        verdict: Verdict = "unknown"
        reason = "Kamera gözlem süresi yetersiz; olayın gerçekleşmediği varsayılamaz."
    else:
        matched = count >= rule.threshold if rule.condition == "at_least" else count <= rule.threshold
        verdict = "triggered" if matched else "not_triggered"
        reason = "Yalnızca gözlenebilen olaylara göre koşul değerlendirildi; sınıflandırma doğruluğu ayrıca ölçülmedi."

    fields = dict(rule_id=rule.rule_id, module=rule.module, camera_id=rule.camera_id,
                  window_start=start, window_end=end, observed_events=count,
                  covered_seconds=round(covered, 4), coverage_ratio=round(ratio, 4),
                  verdict=verdict, validated_accuracy=None, reason=reason)
    return Receipt(**fields, proof_sha256=_canonical_receipt(fields))


@dataclass(frozen=True)
class Experiment:
    """Evidence-backed descriptive ActionLab period comparison."""
    before: Receipt
    after: Receipt
    relative_change: float | None
    enough_evidence: bool
    interpretation: str


def compare(before: Receipt, after: Receipt) -> Experiment:
    if (before.rule_id, before.module, before.camera_id) != (after.rule_id, after.module, after.camera_id):
        raise ValueError("Comparisons must use the same rule/module/camera.")
    if before.window_end > after.window_start:
        raise ValueError("Experiment periods cannot overlap.")
    if before.verdict == "unknown" or after.verdict == "unknown":
        return Experiment(before, after, None, False, "Eksik gözlem nedeniyle karşılaştırma yapılamaz.")
    if before.covered_seconds <= 0 or after.covered_seconds <= 0:
        return Experiment(before, after, None, False, "Gözlem süresi yok.")
    # Normalize by observed usable seconds, never compare raw counts when
    # exposure differs; zero-baseline cannot have meaningful relative %.
    base = before.observed_events / before.covered_seconds
    next_rate = after.observed_events / after.covered_seconds
    change = ((next_rate - base) / base * 100) if base > 0 else None
    return Experiment(before, after, round(change, 2) if change is not None else None,
                      True, "Gözlemsel önce/sonra farkıdır. Tek başına nedensel etki veya gelir artışı kanıtı değildir.")
