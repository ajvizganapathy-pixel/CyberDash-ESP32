# Graph Report - .  (2026-05-14)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 38 nodes · 44 edges · 11 communities
- Extraction: 82% EXTRACTED · 18% INFERRED · 0% AMBIGUOUS · INFERRED: 8 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]

## God Nodes (most connected - your core abstractions)
1. `displayRender()` - 6 edges
2. `stateUpdate()` - 6 edges
3. `jsonState()` - 5 edges
4. `stateSnapshot()` - 4 edges
5. `storageLog()` - 4 edges
6. `audioSampleOnce()` - 2 edges
7. `audioBeep()` - 2 edges
8. `audioStartupChime()` - 2 edges
9. `cardFrame()` - 2 edges
10. `drawHeader()` - 2 edges

## Surprising Connections (you probably didn't know these)
- `audioSampleOnce()` --calls--> `stateUpdate()`  [INFERRED]
  audio.cpp → state.h
- `storageInit()` --calls--> `stateUpdate()`  [INFERRED]
  storage.cpp → state.h
- `displayRender()` --calls--> `stateSnapshot()`  [INFERRED]
  display.cpp → state.h
- `sensorsInit()` --calls--> `stateUpdate()`  [INFERRED]
  sensors.cpp → state.h
- `sensorsRead()` --calls--> `stateUpdate()`  [INFERRED]
  sensors.cpp → state.h

## Communities (11 total, 0 thin omitted)

### Community 0 - "Community 0"
Cohesion: 0.31
Nodes (7): cardFrame(), displayBootAnimation(), displayRender(), drawHeader(), drawSoundBar(), drawSparkline(), drawValue()

### Community 1 - "Community 1"
Cohesion: 0.4
Nodes (4): sensorsInit(), sensorsRead(), stateSnapshot(), stateUpdate()

### Community 2 - "Community 2"
Cohesion: 0.4
Nodes (3): rotateIfNeeded(), storageInit(), storageLog()

### Community 3 - "Community 3"
Cohesion: 0.5
Nodes (3): audioBeep(), audioSampleOnce(), audioStartupChime()

### Community 4 - "Community 4"
Cohesion: 0.7
Nodes (4): jsonState(), onWsEvent(), webBroadcastState(), webInit()

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `stateSnapshot()` connect `Community 1` to `Community 0`, `Community 2`, `Community 4`?**
  _High betweenness centrality (0.438) - this node is a cross-community bridge._
- **Why does `stateUpdate()` connect `Community 1` to `Community 2`, `Community 3`?**
  _High betweenness centrality (0.322) - this node is a cross-community bridge._
- **Why does `displayRender()` connect `Community 0` to `Community 1`?**
  _High betweenness centrality (0.302) - this node is a cross-community bridge._
- **Are the 5 inferred relationships involving `stateUpdate()` (e.g. with `audioSampleOnce()` and `sensorsInit()`) actually correct?**
  _`stateUpdate()` has 5 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `stateSnapshot()` (e.g. with `displayRender()` and `storageLog()`) actually correct?**
  _`stateSnapshot()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 2 inferred relationships involving `storageLog()` (e.g. with `stateSnapshot()` and `stateUpdate()`) actually correct?**
  _`storageLog()` has 2 INFERRED edges - model-reasoned connections that need verification._