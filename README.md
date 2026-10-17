# Kryden Loot System for DayZ

Kryden Loot System replaces the loot lying on the floor of buildings with **searchable storages**: abandoned
backpacks, jackets, crates, medical kits, food sacks, tool chests and weapon crates.

- The Central Economy (CE) spreads the storages **empty and closed** on the normal loot spots.
- A player holds **Search** for about 3 seconds. The server then creates the loot inside the storage, once.
- A searched storage turns **Ruined**. Players can still take what is left, and the CE removes it a few
  minutes later and spawns another one somewhere else.
- Storages cannot be picked up, moved or used to store your own items.

The ready-made template for Chernarus keeps the **vanilla amount and mix of loot per area** (military, police,
hunting, town, village, toxic zone...). It only moves that loot from the floor into the storages. How rare the
loot is stays your choice: everything is in plain JSON files.

Free mod. No dependencies. Works on DayZ 1.30.

**Steam Workshop:** https://steamcommunity.com/sharedfiles/filedetails/?id=3821627706

## Where to start

| You want to... | Read |
|---|---|
| Get the mod | [Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3821627706) |
| Install it on your server | [Server guide](workshop/README.md) |
| Change what comes out of the storages | [Editing the loot](docs/EDITING-LOOT.md) |
| Use it on another map (Livonia, Sakhal, community maps) | [Map templates](docs/MAP-TEMPLATES.md) |
| Understand how it works | [How it works](docs/HOW-IT-WORKS.md) |
| Build it from source, or see how it was made | [Building](docs/BUILDING.md) |

## Maps

| Map | Template | Status |
|---|---|---|
| Chernarus | `template/chernarusplus` | Tested in-game on DayZ 1.30 |
| Livonia | `template/enoch` | Generated for DayZ 1.30, not tested in-game yet |
| Sakhal | `template/sakhal` | Generated for DayZ 1.30, not tested in-game yet |
| Any other map | made with the [generator](docs/MAP-TEMPLATES.md) | Yours to test |

## Repository layout

```
KrydenLootSystem_Client/   client PBO: storage classes, the Search action, texts (EN/PT-BR)
KrydenLootSystem_Server/   server PBO: loot generation from the JSON files
template/<map>/            ready-made template per map (mission files + profile JSON)
tools/template/            the generator that builds a template from a map's vanilla mission
tools/release/             builds the Steam Workshop folder
workshop/                  mod.cpp, logo and the server guide shipped with the mod
docs/                      the guides linked above
```

## License

[Arma and DayZ Public License Share Alike (ADPL-SA)](LICENSE.md). In short:

- give credit (name and a link to this repository);
- no commercial use;
- if you change it, publish your version under the same license;
- Arma and DayZ only.

**Please do not reupload or repack the mod.** Load the original from the Steam Workshop. If you make your own
version, publish it as a separate mod that credits and links to this one.

Made by Kryden.
