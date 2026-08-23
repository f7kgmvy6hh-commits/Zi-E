# Legacy Material Status

## Policy

Everything in `legacy/` is retained to avoid losing previous work, but it is **not approved engineering**.

## Known categories of problems already noticed

The older documents and firmware include contradictions such as:

- inconsistent motor-power routing;
- inconsistent pin mappings between audio, TFT, SD, and other functions;
- claims that an onboard peripheral means no pin/resource conflict;
- outdated or revision-dependent camera assumptions;
- AI/cloud features described as complete while some implementations are placeholders;
- mechanical dimensions presented as final without complete real-part fit verification;
- older firmware architecture that mixed prototype code and production assumptions.

These are examples, not a complete audit.

## Rule for reuse

Before reusing anything from legacy:
1. Identify the claim.
2. Check current product requirements.
3. Check the actual target component revision.
4. Verify primary datasheet/vendor documentation.
5. Check mechanical/power/pin/resource budget.
6. Record the decision in the new design docs.
7. Only then implement it.

## Files currently preserved

See `legacy/guides/` and `legacy/firmware/`.
