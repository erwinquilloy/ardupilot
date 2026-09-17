# Parameter documentation — `master_custom_4.6.3` fork

Machine-generated parameter metadata for **this fork** of ArduPlane.
Regenerated from source via:

```bash
python Tools/autotest/param_metadata/param_parse.py --vehicle ArduPlane
```

The files in this directory cover **every parameter** that the firmware
exposes, including the fork-specific additions:

- `FLIGHT_OPTIONS` bits 22 (`RTL_CLIMB_FIRST_ONLY_IN_FS`), 23
  (`ALLOW_GLIDING_IN_AUTO_THR_MODES`), 24 (`RTL_MANUAL_ALT_CONTROL`)
- `RC_OPTIONS` bit 22 (`PLANE_DISABLE_MAN_BAT_COMP`)
- `OSD_OPTIONS` bits 18 / 21 / 23 (fork OSD polish)
- `AILERON_DIFF`, `ELEVATOR_DIFF`, `KFF_THRAT2ELEV`, `KFF_FLAP2ELEV`
- `FS_ELAND_*` emergency-landing state machine
- `TKOFF_THR_IDLE` family, `THR_DZ`, `RTL_ALT_HOME`
- All OSD additions (loiter radius, AoA, peak rates, stats grid, etc.)

…and any others described in the top-level [`README_CUSTOM.md`](../../README_CUSTOM.md).

## Files

| File | Use case | Size |
|---|---|---|
| `apm.pdef.json` | Best for tooling — easy to parse in JavaScript / Python | ~2.1 MB |
| `apm.pdef.xml`  | Alternative structured format, schema matches upstream | ~2.5 MB |
| `Parameters.html` | Browsable in any web browser | ~2.0 MB |
| `Parameters.md` | GitHub-rendered Markdown | ~1.2 MB |
| `Parameters.rst` | Sphinx-flavoured reStructuredText | ~3.5 MB |
| `ParametersLatex.rst` | Book-style RST tuned for LaTeX rendering | ~3.2 MB |

## Bitmask calculator

A standalone calculator that loads `apm.pdef.json` and lets you tick bits
to compute integer values (or paste a value to see which bits are set)
lives at [`docs/bitmask_calculator.html`](../bitmask_calculator.html).
It's a single self-contained HTML file — no server, no analytics, runs
in your browser. Defaults to fetching the JSON from the light branch's
raw URL; can also load from the full branch, a custom URL, or a local
file.

## Stable URLs for tooling

For a bitmask calculator or similar app, fetch the JSON directly:

```
https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3/docs/parameters/apm.pdef.json
https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/parameters/apm.pdef.json
```

Both URLs serve identical content — parameter table definitions are
the same on the full and light branches. Pick whichever matches the
firmware variant your users flash.

## Bitmask params worth knowing

For a calculator UI, the params most users care about toggling bit-by-bit:

| Param | Description |
|---|---|
| `FLIGHT_OPTIONS` | Mode-specific flight behaviours (rudder mix, takeoff arming, RTL behaviours, fork bits 22/23/24) |
| `OSD_OPTIONS` | OSD rendering options (decimal precision, plus-code shortening, etc.) |
| `OSD1_..OSD7` element `_EN` flags | Per-element enable / position params (use `apm.pdef.json` to enumerate) |
| `RC_OPTIONS` | RC protocol + receiver behaviour bits (including fork bit 22 in MANUAL voltage comp) |
| `BATTn_OPTIONS` | Per-monitor battery behaviour (fork-extended to uint32 for bit 23 = Wh-based remaining %) |
| `TKOFF_OPTIONS` | Takeoff behaviour |
| `LAND_OPTIONS` | Landing behaviour |
| `AHRS_OPTIONS` | EKF / AHRS behaviour |
| `FENCE_TYPE` | Geofence type bits |
| `ARMING_CHECK` | Pre-arm check toggles |

…plus the fork-added `RTL_MANUAL_ALT_CONTROL` block, `FS_ELAND_*`
family, and so on — extract from the JSON.

## Regenerating

These files should be regenerated whenever a fork commit adds /
modifies a parameter doc block. Two compounding gotchas to avoid (see
`README_CUSTOM.md`'s migration table and the [skip-tecs-pid-ports
feedback memory](#)):

1. Every line of a `@Param:` doc block must match `// @<word>: ...`.
   No bare comments inside the block — `param_parse.py:46` regex
   silently drops the whole param.
2. No literal commas in `@Bitmask:` or `@Values:` description text.
   `jsonemit.py` splits descriptions on `,` then on `:`, and a comma
   in the description makes the parse `ValueError` out.

Both were tripped during the fork's initial port and fixed in commit
`5c39354d56`.
