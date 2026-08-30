# Reviewed physical board profiles

This directory intentionally contains no board profile. A profile may be added only
after arrival/physical review and must include `ZIE_BOARD_PROFILE_VERIFIED=TRUE` plus
reviewed provenance. Never add seller-only pins or placeholder values to satisfy a
build. `scripts/firmware-workflow.ps1` refuses target operations without such a file.
