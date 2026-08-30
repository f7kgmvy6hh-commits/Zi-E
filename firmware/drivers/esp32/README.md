# ESP32-S3 drivers

Concrete Display, Audio, Camera, and BellyLightMatrix adapters belong here. Each
adapter may include its vendor SDK privately and must expose only the stable HAL
interface. Exact panel/touch bindings, pins, buses, addresses, and current caps are
TODO pending verification; no production driver is claimed yet.

The ESP-IDF composition boundary is `firmware/targets/esp32` and is deliberately
`BOARD_BINDING_DISABLED` until a physically reviewed board profile exists. `HW-002`
remains ordered. Seller camera/GPIO maps and workbook firmware defines cannot populate
this directory as production facts.
