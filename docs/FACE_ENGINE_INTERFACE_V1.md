# Zi-E Face Engine Interface V1

Status: host-testable data/model foundation implemented in `app/presence`; real ESP32
rendering, assets, storage, display identity, and memory budgets remain deferred.

Layers are `verified display driver -> LVGL/render backend -> Zi-E Face Engine ->
data-only Face Pack/Asset Store`. Host sends semantic intent (`state`, `emotion`,
`intensity`, `gaze`, speech level, notification/overlay, sleep/wake), never frames.
ESP32 owns blink, micro-saccades, gaze/mouth interpolation, transition/easing, and
small in-state variation, so network lag does not freeze the face.

States are `IDLE`, `LISTENING`, `THINKING`, `SPEAKING`, `SLEEPING`, `ALERT`, `ERROR`.
In `ONLINE_CONTROLLED`, current Host semantic intent dominates and autonomous
full-face random switching is forbidden; only micro-behaviors are local. In
`OFFLINE_AUTONOMOUS`, bounded contextual weighted pools may select local faces, avoid
immediate repetition where alternatives exist, and remain non-motion.

Face Packs declare pack/version/engine compatibility, states/variants, abstract
`asset.*` handles, online/offline eligibility, weights, storage class, content hash,
license, and provenance. They contain no code, entrypoint, permissions, URL execution,
or authority. Invalid/unbounded/incompatible/unlicensed packs fail closed. Storage
classes are builtin, internal, downloaded cache, and disabled future external storage;
partition sizes await verified HW-002 flash/PSRAM evidence.

XiaoZhi/LVGL patterns are rendering references only. XiaoZhi emoji/font/image assets
are not Zi-E Face Packs and are not copied without independent license/provenance review.
