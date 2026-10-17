# Kryden Loot System

Replaces the floor loot inside buildings with **searchable storages**: abandoned backpacks, jackets, crates,
medical kits, food sacks, tool chests and weapon crates.

- The Central Economy spreads the storages **empty and closed** on the vanilla loot spots.
- A player holds **Search** (about 3 s). The server then generates the loot from JSON pools, once.
- A searched storage turns **Ruined**. Players can still take what is left. The CE removes it after
  `CleanupLifetimeRuined` (330 s in vanilla) and spawns another one somewhere else.
- A searched storage that gets emptied disappears at once. A storage nobody searches expires after 4 hours and
  respawns elsewhere.
- Storages cannot be picked up, moved, or used to store anything.

The ready-made template for **Chernarus** (DayZ 1.30) keeps the vanilla volume and mix of loot per area
(military, police, hunting, town, village, toxic zone...). It only moves that loot from the floor into the
storages. How rare loot is stays your call: edit the JSON files.

Free mod. No dependencies (no CF needed). Steam Workshop: https://steamcommunity.com/sharedfiles/filedetails/?id=3821627706
Source code, guides and the template generator for other maps: https://github.com/c-Morette/Kryden-Loot-System

---

## Install (server)

1. **Mod:** load `@KrydenLootSystem` with `-mod=` on the server and on clients. Copy `keys/Morette.bikey` to
   the server `keys` folder.
2. **Mission** (`mpmissions/dayzOffline.chernarusplus`). Back up the files you replace first, named
   `<file>.bak-kls-<date>` (for example `mapgroupproto.xml.bak-kls-20261017`): the template generator finds the
   vanilla files by that name later.
   - Copy `ServerFiles/chernarusplus/mission/mapgroupproto.xml` and `mission/cfgeventgroups.xml` over the
     mission files.
   - Copy the folder `ServerFiles/chernarusplus/mission/kls/` into the mission.
   - Open the mission `cfgeconomycore.xml` and paste the block from
     `ServerFiles/chernarusplus/mission/cfgeconomycore.snippet.xml` right before `</economycore>`.
3. **Profile:** copy `ServerFiles/chernarusplus/profile/Kryden/` into your server profile folder (the one
   passed with `-profiles=`). You end up with `<profiles>/Kryden/KrydenLootSystem/Settings.json`, `Storages.json`,
   `Kits.json` and `Pools/`.
4. Start the server. The `.RPT` should show:
   `[KrydenLootSystem] Loaded 110 pool(s), 411 kit(s) and 100 storage entry(ies)`. On DayZ 1.30 a line before it
   says the folder-listing workaround is on (DayZ bug DZEXP-134); that is expected.

A fresh economy (wiping `storage_1`) is optional. Without it, the floor loot already in persistence stays until
its lifetime ends, and the CE stops spawning new floor loot right away.

**If your mission already has changes** to `mapgroupproto.xml` or `cfgeventgroups.xml`, copying the template
files replaces those changes.

**After a DayZ update:** if the update restores the vanilla mission files, copy `mapgroupproto.xml` and
`cfgeventgroups.xml` again and check that the `kls` block is still in `cfgeconomycore.xml`.

## Uninstall

1. Restore your backups of `mapgroupproto.xml` and `cfgeventgroups.xml`.
2. Remove the `<ce folder="kls">` block from `cfgeconomycore.xml` and delete the mission `kls` folder.
3. Remove the mod. Storages left in persistence are dropped on the next start, and the vanilla floor loot
   comes back as the CE refills.

## What the template changes

- **`kls/kls_types.xml`**:
  - the storage types (where they spawn, how many, tier);
  - a partial override with `nominal` and `min` set to 0 for the vanilla loot items. That override stops the
    CE from spawning those items on the floor. `db/types.xml` is not edited.
- **`kls/kls_events.xml`**: event wrecks with common loot (crashed plane, police car) hold at most 1 storage
  each. Heli crashes and the Christmas event keep their vanilla loot.
- **`mapgroupproto.xml`**: at most 2 storages per building, and a small storage slot on wrecks.
- **`cfgeventgroups.xml`**: the same 1-storage cap on the event wreck groups.

---

## Configuration (`<profiles>/Kryden/KrydenLootSystem/`)

The mod never overwrites your files. A broken file is logged in the `.RPT` and ignored. Booleans are written
as `0`/`1`. Classnames that don't exist are logged once at boot and skipped.

### `Settings.json`

| Field | Default | Meaning |
|---|---|---|
| `SearchTime` | `3.0` | Seconds the player holds Search. |
| `DefaultDamage` | `[0.0, 0.82]` | Damage range (0 = pristine, 1 = ruined) for items with no `Damage` of their own. Loot never spawns ruined. |
| `DebugLog` | `0` | `1` writes one line per search in the `.RPT` with what came out. |
| `MaxGunsPerStorage` | `2` | Most firearms one storage can hold. The same gun never comes twice in one storage. |
| `MaxSameItemPerStorage` | `2` | Most copies of the same item (not guns) per storage. `0` = no cap. If a pool is too small, the item count wins over the cap. |
| `ExportItems` | `0` | `1` writes `Export/items.json` at start (every item the server knows, with size and family), for the template generator. |

### `Storages.json`

One entry per storage class. An entry for a class also covers its child classes (`KLS_WoodenCrate` covers
`KLS_WoodenCrate_T1`..`_T4`) unless the child has its own entry. The template has one entry per class and tier.

| Field | Meaning |
|---|---|
| `Storage` | Storage classname. |
| `Min`, `Max` | How many items one search creates. |
| `Pools` | `[{"Pool": "Town_T1", "Weight": 120.5}, ...]`. For each item, a pool is picked by `Weight`, then an item from it. |
| `Categories` | Optional filter: only items of these categories come from the pools. Empty = everything. |
| `Guaranteed` | Optional: `[{"Pool": "Food", "Min": 1, "Max": 2, "Categories": ["food"]}]`. These items come first and count toward `Min`/`Max`. |
| `AmmoPerGun` | Magazines or ammo of the gun's own caliber added per firearm, on top of `Max`. |
| `AmmoChance` | Chance (0 to 1) that a firearm gets that ammo. |

### `Pools/*.json`

One pool per file. The file name is the pool name. One line per item:

```json
{"Item": "AKM", "Weight": 1234, "Category": "guns"}
```

- `Weight` is relative inside the pool. The template uses the vanilla `nominal`, weighted by how much of that
  item vanilla spawns in the pool's area, x1000.
- `Category` is used by the `Categories` filters. Template values: `guns`, `ammo`, `attachments`, `medical`,
  `tools`, `food`, `drink`, `clothes`, `containers`, `explosives`.
- A line can also carry `Damage`, `Quantity`, `Attachments` or `Cargo` (same format as `Kits.json`). Whatever
  the line has wins over the item's kit, field by field.

To add an item from another mod, add a line to the pools you want. To stop that item from spawning on the
floor, add a `<type>` for it in `kls/kls_types.xml` with `nominal` and `min` set to 0.

### `Kits.json`

The default kit of each item, like the vanilla `cfgspawnabletypes.xml`. Written once, used by every pool.

```json
{
  "Item": "AKM",
  "Damage": [0.45, 0.85],
  "Attachments": [
    {"Chance": 0.25, "Items": [{"Item": "AK_PlasticBttstck", "Weight": 1.0}]},
    {"Chance": 0.5, "Items": [{"Item": "KashtanOptic", "Weight": 0.3}, {"Item": "PSO11Optic", "Weight": 0.2}]}
  ]
}
```

- `Damage`: `[min, max]` damage. `Quantity`: `[min, max]` from 0 to 1 (rounds in magazines and ammo piles,
  quantity in everything else).
- `Attachments` and `Cargo` are groups. Each group spawns with its `Chance`, and one item of the group is picked
  by `Weight`.
- Items inside a group use their own kit too, two levels deep (weapon > optic > battery).
- An item with no kit spawns bare, with `DefaultDamage`.

---

## License

[Arma and DayZ Public License Share Alike (ADPL-SA)](https://www.bohemia.net/community/licenses/arma-and-dayz-public-license-share-alike-adpl-sa):
credit with a link, no commercial use, same license for modified versions, Arma and DayZ only.

**Please do not reupload or repack the mod.** Load the original from the Steam Workshop. If you make your own
version, publish it as a separate mod that credits and links to https://github.com/c-Morette/Kryden-Loot-System.
