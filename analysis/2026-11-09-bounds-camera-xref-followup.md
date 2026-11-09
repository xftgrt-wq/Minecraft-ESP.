# 2026-11-09 — Follow-up: bounds/camera references

## Additional scan
A fresh scan of the uploaded 1.26.33 ARM64 library found many references to the strings `AABBShapeComponent` and `StateVectorComponent`, fewer references to `ActorDataBoundingBoxComponent`, and one reference to `CurrentViewProjectionMatrixUniform`.

The most obvious AABB + StateVector pair at code address `0x0c60004c` / `0x0c600148` is inside a JNI-side static metadata/registration initializer: it copies component type-name strings into a local descriptor and passes that descriptor to a registration helper. It is not an actor iteration routine or a live component accessor.

The sole direct reference to `CurrentViewProjectionMatrixUniform` at `0x1290cf2c` is also in a lazy static metadata initializer (guard acquire, pass string to a metadata helper, guard release). It does not expose the live GPU matrix.

A scan for readable strings also found no useful exported engine accessor names such as `getInterpolatedPosition` or `getActorBounds`. The text `getBoundingBox` appears in two places and `getAABB` once, but those names alone do not establish callable native methods or their ABI.

## Updated conclusion
The binary confirms that the engine registers and uses ECS component types for actor bounds and state, but the inspected string cross-references are metadata, not runtime access points. No component offsets, entity iteration function, render callback, or live view-projection matrix pointer have been verified. It would be misleading to turn these strings into guessed offsets or a pretend working ESP.

## Next meaningful route
Continue from the ECS registration metadata to the concrete system update functions and component storage APIs, then identify the active render pipeline's matrix upload path. This requires tracing actual call/data flow, not copying offsets from a different Bedrock version.
