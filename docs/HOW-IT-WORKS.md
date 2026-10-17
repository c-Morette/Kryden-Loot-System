# How it works

## Two PBOs

| PBO | Runs on | What it has |
|---|---|---|
| `KrydenLootSystem_Client` | clients and server | the storage classes, the Search action, the texts (English and Brazilian Portuguese), the textures and logo |
| `KrydenLootSystem_Server` | server (the client loads it but never runs its logic) | reading the JSON files, generating the loot, the item export |

Everything that decides something runs on the server. The client only shows the action and the storage.

## The storage

Every storage is a class that inherits from the vanilla `Container_Base` and reuses the model of a vanilla item
(backpacks, jackets, the wooden crate, the first aid kit, the canvas bag, the sea chest...). No vanilla class
is changed. The classes come in five variants, one per tier and one without tier (`KLS_WoodenCrate`,
`KLS_WoodenCrate_T1` ... `_T4`), because the CE places items by tier and no script API returns the tier of a
position.

What a storage refuses:

- being picked up, moved, worn or swapped;
- anything put inside it, and anything put inside the items inside it (a medical kit in the crate, an ammo
  pile to merge). The server opens a short window only while it creates the loot.

## The life of a storage

1. **Spawn.** The CE places it on a normal loot spot, empty and full health. Its inventory is hidden.
2. **Search.** A player holds the action for `SearchTime` seconds (3 by default). When two players search at
   the same time, the first one to finish generates the loot and the bar of the other one stops.
3. **Loot.** The server picks the items from the JSON files (see [Editing the loot](EDITING-LOOT.md)) and
   creates them in the cargo, with temperature, damage and quantity set like vanilla loot.
4. **Ruined.** The storage turns Ruined: it cannot be searched again, and the inventory opens.
5. **Cleanup.** If someone takes the last item, the storage is deleted at once. Otherwise the CE deletes it
   after `CleanupLifetimeRuined` (330 seconds in vanilla), with whatever is left, and spawns a new one somewhere
   else. A storage that nobody searches expires after 4 hours.

Server restarts keep closed storages closed. A Ruined storage is removed when the server loads.

## The template

The template changes the Central Economy without touching `db/types.xml`:

- `kls/kls_types.xml` (a `ce` folder registered in `cfgeconomycore.xml`) declares the storage types (where
  they spawn, how many, which tier) and sets `nominal` and `min` to 0 for the vanilla floor loot. The CE stops
  spawning that loot on the floor; lifetime and everything else stay as in vanilla.
- `mapgroupproto.xml` limits each building to 2 storages and lets the storages use the loot spots.
- `kls/kls_events.xml` and `cfgeventgroups.xml` limit event wrecks with common loot to 1 storage each. Heli
  crashes, Christmas trees and bonfires keep their own vanilla loot.

## How the loot was calibrated

The goal is the **vanilla loot, inside the storages**. The generator measures vanilla from the mission files:

- For each item, the share of it that falls in an area is its `nominal` times the loot spots of that area,
  divided by all the spots where the item can spawn. That gives the vanilla mix and volume of every area and
  tier.
- The pool of each area and tier holds the items of that area with that share as their weight, so the
  hunting pool is mostly hunting gear and the military pool has ammo and magazines on top.
- Guns and explosives have pools of their own and keep the vanilla share of the area.
- The number of storages per area is a share of the loot spots. The items per storage grow, up to 3 times the
  base size and 10 items, until the area comes close to the vanilla volume. Military zones get extra storages
  until they reach about 70% of vanilla; toxic zones are kept under 3 times vanilla.
- A gun comes with its own magazine or ammo 30% of the time (the weapon crates always). At most 2 guns and 2
  copies of the same item per storage.

On Chernarus the result is about 3,400 storages carrying close to 90% of the vanilla volume, instead of about
18,000 items lying on the floors.

## DayZ 1.30 note

DayZ 1.30 has a bug where listing a folder through `$profile:` returns nothing (DZEXP-134). The mod detects it
at start and lists the pool folder with the same workaround as Community Framework 1.5.9. The `.RPT` says so
in one line.
