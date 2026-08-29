# Purchased-Parts / Google Sheet Intake Contract

## 0.04 working-copy workflow

Google Sheet or the HUD form is intake convenience. Export CSV, preview and explicitly
confirm it in Control Center, use the contained inventory as the engineering working
copy, review evidence, then export an approved repository snapshot when appropriate.
Google credentials stay outside the repository; do not treat two mutable sources as
simultaneously authoritative.

Use the exact columns in `docs/templates/PURCHASED_PARTS_INTAKE_TEMPLATE.csv`. The
template intentionally contains no invented inventory rows. One row represents one
exact purchasable variant or uniquely identified assembly; split rows when revision,
source, controller ownership or disposition differs.

## Field contract

| Field | Rule |
|---|---|
| `inventory_id` | Stable user-assigned ID; never reuse after replacement |
| `part_name`, `manufacturer`, `model_exact_variant` | Transcribe labels/order record; do not normalize away suffix/revision |
| `quantity`, `purchase_status` | Numeric count and controlled procurement state: existing/required/ordered/received/quarantined/returned |
| `procurement_role` | Controlled planning role: existing/required/substitute_candidate; this does not approve a substitute |
| `purchase_link_reference`, `photo_reference` | Provenance links/IDs; photos must show label and relevant connectors/dimensions |
| `controller_ownership` | Closed intent: ESP32-S3/STM32/host/passive; ambiguity stays blank/UNDECIDED |
| `function`, `interface_documented` | Semantic role and datasheet interface, not guessed pins |
| `voltage_documented`, `current_documented` | Datasheet values with source/conditions/units |
| `voltage_measured`, `current_measured` | Blank until instrumented measurement with condition/log |
| `connector` | Exact family/part/cavity/key/orientation when verified; not “JST” alone |
| `zie_logical_slot` | Stable semantic slot such as `cliff.downward.front_left` |
| `driver_extension` | Protected embedded driver or semantic extension responsibility; never raw plugin authority |
| `hardware_profile_mapping` | Exact profile requirement/capability candidate; activation still requires registry/commissioning |
| `cad_status` | absent/envelope/modelled/fit_coupon_passed/received_fit_verified/rework |
| `dimensions_documented`, `weight_documented` | Datasheet/listing value plus source and units |
| `dimensions_measured`, `weight_measured` | Blank until received-part measurement |
| `safety_criticality` | critical/high/medium/low with rationale in notes |
| `verify_on_arrival` | yes until identity, condition, fit, interface and ratings are reconciled |
| `decision_keep_replace_undecided` | Product Owner disposition; substitute candidates remain UNDECIDED |
| `notes`, `evidence_source`, `reviewer`, `date_received` | Conditions, conflicts, provenance and audit trail |

## Intake workflow

1. Export/copy the sheet without dropping columns; retain original revision/time.
2. Validate required IDs, controlled procurement states/roles, quantities and duplicate exact items.
3. Link order evidence and label photos; quarantine mismatches/counterfeit uncertainty.
4. Compare the row to `HARDWARE_SOURCE_OF_TRUTH_RECONCILIATION.md`; record conflicts.
5. Measure only with recorded method/instrument/condition; never copy CAD into measured
   fields.
6. Product Owner decides KEEP/REPLACE/UNDECIDED where evidence changes a candidate.
7. Update Device Identity, Hardware Profile candidate and CAD together; no silent
   rebinding/substitution.

The Sheet becomes authoritative for physical inventory only after review. It does not
by itself prove electrical suitability, safety, commissioning or execution authority.

## Control Center 0.03 compatibility decision

The 32 columns above remain unchanged and authoritative for CSV interchange. Runtime
metadata is intentionally separate in ignored `runtime/inventory/active.json`:
`physical_status`, `storage_location`, evidence/review lifecycle, explicit candidate
match, driver state, commissioning/measurement requirements, conflicts, verification,
tombstone/history, and revision timestamps. Export retains exact canonical ordering.

For ordinary entry, users need only: `inventory_id`, `part_name`, `manufacturer`,
`model_exact_variant`, `quantity`, `physical_status` (`RECEIVED`, `ORDERED`,
`NOT_BOUGHT`, `UNKNOWN`), purchase reference, photo reference, storage location, and
notes. Missing engineering values remain unknown and move to the separate review.

### Google Sheet without API credentials

1. Maintain the ten simple user columns in the Sheet.
2. Export as CSV and place values into the matching canonical template columns.
3. Preview the canonical CSV in Control Center; resolve validation/conflicts.
4. Explicitly confirm persistence with the current inventory revision.
5. Perform engineering review, reconciliation, and evidence-backed decisions.

Google API synchronization and credentials are deliberately absent.
