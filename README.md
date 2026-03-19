# Alkamel Real-Time Classification Client (Qt/C++)

## Project Overview
Desktop Qt Widgets application that connects to the Alkamel V2 server over SSL TCP/IP, authenticates, subscribes to timing channels, keeps the session alive, receives partial JSON updates, reconstructs full state locally, builds race classification rows, and displays them in a live table.

## Architecture Summary
- `src/network/AlkamelClient.*`
Raw SSL transport (`QSslSocket`), CRLF/LF line framing, send/receive logging.
- `src/network/Protocol*`
Protocol message model + parser + serializer for `command-id:[message-id[+]]:[channel]:[data]`.
- `src/network/AlkamelSession.*`
Session orchestration (`LOGIN`, `JOIN`, `PING`, `ACK/ERROR`, JSON dispatch).
- `src/state/JsonStateStore.*`
Recursive merge state store to rebuild full JSON tree from partial updates.
- `src/domain/ClassificationBuilder.*`, `src/domain/RaceState.*`
Domain extraction and normalization into classification rows.
- `src/ui/ClassificationTableModel.*`
`QAbstractTableModel` for live classification in `QTableView`.
- `src/app/MainWindow.*`
UI composition + wiring between networking, state, domain and model/view.

## Memory Ownership (Heap vs Value)
- `QObject`-derived UI/session/transport objects are created on the heap with `new` and attached to a Qt parent (`this` or a child widget). Qt parent-child ownership handles lifetime and destruction automatically.
- Raw pointer members that reference those `QObject` instances are non-owning access pointers, not raw owning pointers.
- Non-`QObject` data is mostly stored as value members (`AppConfig`, `QByteArray`, `QJsonObject`, counters, hashes, domain rows), which keeps ownership simple and deterministic.
- `std::shared_ptr` is intentionally not used because there is no genuine shared-lifetime requirement in this project.

## Build Steps
Example (Qt 6 + MinGW + Ninja on Windows):

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/mingw_64"
cmake --build build -j 8
```

Run tests:

```powershell
ctest --test-dir build --output-on-failure
```

## Run Steps
```powershell
.\build\alkamel_client.exe
```

## Assumptions
- Standings bucket preference is `overall.active`, then `overall.finishLine`, else first non-empty standings object.
- Participant mapping tries entry key first, then car-number fallback.
- Arrays/scalars/null in partial updates replace previous values at that key; objects merge recursively.

## Limitations
- Current UI refresh uses full table reset per update (acceptable for small grids, can be optimized later).
- No persistence/export; state is in-memory only.
- Logging is development-oriented and intentionally verbose.

## Self-Signed SSL Note
The assignment server uses a self-signed/mismatched certificate. SSL errors are explicitly ignored in `AlkamelClient::onSslErrors()` per test requirement.

## Partial Update Merge Note
Alkamel JSON feed sends only modified subtrees. `JsonStateStore` merges incoming object nodes recursively to maintain a full coherent state tree used by the domain layer.
