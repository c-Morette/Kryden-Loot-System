# Map templates (the generator)

A template is what makes the mod fit one map: the mission files for the Central Economy and the loot JSON
files. Chernarus, Livonia and Sakhal templates are ready in `template/`. For any other map, or after a DayZ
update changes the vanilla loot, build your own with the generator in `tools/template/build_template.py`.

The generator reads the **vanilla mission** of the map and calculates, for every area and tier, how much loot
vanilla puts on the floor. It then sizes the storages to deliver about the same inside them.

## What you need

- **Python 3.10 or newer** (no extra packages).
- The map's **vanilla mission folder**, for example `mpmissions/dayzOffline.chernarusplus`. If the folder
  already has a Kryden template installed, the generator uses the `*.bak-kls-*` backups you made when
  installing (the newest one).
- The **item data of your server** (`items.json`). The mod writes it:
  1. install the mod and start the server once;
  2. in `<profiles>/Kryden/KrydenLootSystem/Settings.json` set `"ExportItems": 1`;
  3. start the server again. The `.RPT` shows `Exported N item(s) to .../Export/items.json`;
  4. set `"ExportItems"` back to `0`.

  The file lists every item the server knows, including items of your other mods, with its size in the
  inventory and its family (gun, pistol, ammo...). Export it again after a DayZ update or after you add mods.

## Run it

```
python tools/template/build_template.py --map <name> --mission "<mission folder>" --items "<items.json>" --out "<output folder>"
```

Example for Livonia:

```
python tools/template/build_template.py --map enoch --mission "C:\DayZServer\mpmissions\dayzOffline.enoch" --items "C:\DayZServer\profiles\Kryden\KrydenLootSystem\Export\items.json" --out "C:\kls\enoch"
```

The output folder gets the same layout as `template/chernarusplus`: `mission/` and `profile/`. Install it with
the steps of the [server guide](../workshop/README.md#install-server). Check `REPORT.md` in the output: it lists
how many storages the map gets, per area and tier, and how close the loot volume is to vanilla (the report is
in Portuguese for now).

## Options

| Option | Default | What it does |
|---|---|---|
| `--cap` | `2` | Most storages per building (1 or 2). |
| `--fill` | `0.15` | Share of the capped loot spots that get a storage, outside military and toxic zones. |
| `--event-cap` | `1` | Most storages per event wreck with common loot (police car, crashed plane...). |
| `--min-ratio` | `0.8` | `min` of each storage type as a share of its `nominal` (below it, the CE refills). |
| `--restock` | `600` | Seconds the CE waits before refilling a storage type. |
| `--gun-boost` | `3.0` | Ceiling of the extra gun share in areas where the storages carry less than vanilla. |

Military and toxic zones size themselves: military gets storages until it reaches about 70% of the vanilla loot
volume, and the toxic zone stays under 3 times vanilla.

## Good to know

- Areas that no storage serves (castles, the amusement park, bunkers) keep their vanilla floor loot.
- Items too big for a 10x10 storage (barrels, sea chest, generator...) stay on the floor.
- Heli crashes, Christmas trees and bonfires keep their vanilla event loot.
- A community map with areas (usages) of its own: those areas get no storages, and items that only spawn there
  stay on the floor. Test the template on a local server first, with `DebugLog` on.
- Name your backups `<file>.bak-kls-<date>` when you install (for example
  `mapgroupproto.xml.bak-kls-20261017`): that is how the generator finds the vanilla files later.
