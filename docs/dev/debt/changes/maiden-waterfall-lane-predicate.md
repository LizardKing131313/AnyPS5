# shader

## add Functional

- [Waterfall descriptor lane predicates](../../../core/shader/recompiler/Optimization/src/ResourceTracker.cpp) are proven for wave32 through conjunctions, loop-carried masks, lane-bit tests and the low ballot word. A stale descriptor is ignored only when the image-read predicate proves its lane executed the descriptor load. Equality hidden behind ballot masks must prove every descriptor word before removing a cross-lane read. Other predicate forms and wave64 retain the existing refusals. The lane-mask proof and the resulting RE Engine shader execution have not been measured on PS5 hardware.
