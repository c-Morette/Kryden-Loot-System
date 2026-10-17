"""
KrydenLootSystem - gerador de template por mapa (ferramenta INTERNA da Kryden, D-09).

Le a missao vanilla do servidor (Steam atualiza ela a cada update do DayZ, entao e a fonte mais fresca) e
gera o template que o admin so copia:

  <out>/mission/mapgroupproto.xml          lootmax de cada group <= --cap (D-10); groups de evento intocados
  <out>/mission/kls/kls_types.xml          types dos storages (tier/usage/tag) + override parcial nominal=0
  <out>/mission/cfgeconomycore.snippet.xml bloco <ce folder="kls"> para colar no cfgeconomycore.xml
  <out>/profile/Kryden/KrydenLootSystem/   Settings.json, Storages.json, Pools/*.json (D-19), Kits.json (D-29)
  <out>/REPORT.md                          capacidade, storages por usage, override, origem (RF-45)

Uso:
  python build_template.py --map chernarusplus [--cap 2] [--fill 0.42] [--mission <pasta>] [--out <pasta>]

Protecoes (a missao do servidor de teste nao e vanilla intocada):
  - so le db/ e os arquivos da raiz da missao; ignora toda pasta <ce> (o nosso kls nao volta para dentro);
  - recusa mapgroupproto com a marca do KLS e usa o backup *.bak-kls-* (nao se alimenta do proprio resultado);
  - item com nominal 0 (ex.: MBB_* locais) nao entra em nada.
"""
import argparse, glob, json, math, os, re, sys, datetime
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict

SERVER = r"C:\Program Files (x86)\Steam\steamapps\common\DayZServer"
MARK = "KrydenLootSystem template"

# Storages da V1 (D-18) e o tema padrao do template (D-19: o admin decide; isto e so o default).
STORAGES = [
    # classe,               usages,                                                                                   tags,                          min, max
    ("KLS_WoodenCrate",     ["Industrial", "Farm", "Village", "Town", "Firefighter", "Coast", "Office", "School"],     ["floor", "shelves", "ground"], 3, 6),
    # Familias do caixote (04/10/2026): com todas as usages numa classe so, o caixote em predio militar sorteava
    # pela capacidade do MAPA (militar = 3% no T4). Uma classe por familia: o CE casa a usage do ponto.
    ("KLS_WoodenCrate_Military", ["Military"],                                                                     ["floor", "shelves", "ground"], 3, 6),
    ("KLS_WoodenCrate_Police",   ["Police", "Prison"],                                                             ["floor", "shelves", "ground"], 3, 6),
    # Zona toxica fixa: usage vem do areaflags.map (AREA_USAGES), nao do predio. Sem ela M4A1/FAL/AUG sumiam.
    ("KLS_WoodenCrate_Toxic",    ["ContaminatedArea"],                                                             ["floor", "shelves", "ground"], 3, 6),
    # So caca (04/10/2026): com caca + vila ela sorteava 87% na pool de vila e roupa de cacador nao saia.
    ("KLS_HuntingBag",      ["Hunting"],                                                                              ["floor"],                     2, 4),
    ("KLS_MountainBag",     ["Town", "Village", "Coast"],                                                             ["floor"],                     2, 4),
    ("KLS_ChildBag",        ["School", "Town", "Village"],                                                            ["floor"],                     2, 3),
    ("KLS_CoyoteBag",       ["Farm", "Village", "Industrial"],                                                        ["floor"],                     2, 4),
    ("KLS_AssaultBag",      ["Military"],                                                                             ["floor"],                     2, 4),
    ("KLS_TTsKOJacket",     ["Military"],                                                                             ["floor", "shelves"],          1, 3),
    ("KLS_QuiltedJacket",   ["Town", "Village", "Office"],                                                            ["floor", "shelves"],          1, 3),
    ("KLS_ParamedicJacket", ["Medic"],                                                                                ["floor", "shelves"],          1, 3),
    ("KLS_PoliceJacket",    ["Police", "Prison"],                                                                     ["floor", "shelves"],          1, 3),
]
# Cargo de TODO storage (config.cpp): 10x10 desde 03/10/2026.
CARGO_W, CARGO_H = 10, 10
# Objetos de base que ficam no chao mesmo cabendo no cargo (decisao 03/10/2026).
FLOOR_ALWAYS = {"seachest", "powergenerator", "metalplate", "pileofwoodenplanks",
                "barrel_blue", "barrel_green", "barrel_red", "barrel_yellow"}
# Fracao dos itens de cada storage que sai da pool de ARMAS DE FOGO = fracao de arma no loot VANILLA daquela
# zona (usage, tier) x multiplicador DA ZONA = volume vanilla / volume KLS, entre 1 e --gun-boost (04/10/2026):
# cada zona fica com o mesmo numero ABSOLUTO de armas do vanilla. Com 3x fixo, a zona toxica (fill 0,80, volume
# perto do vanilla) dava ~13 FAL/AUG/M4A1 no mapa contra 4 do vanilla.
GUN_SHARE_MAX = 0.5
# Explosivo (granada, mina, C4) tem pool propria e a mesma conta da arma: fracao vanilla da zona x multiplicador
# da zona. Na pool de equipamento ele concorria com roupa/ferramenta e saia ~15% do vanilla (04/10/2026).
EXPL_SHARE_MAX = 0.3
# Calibracao do volume (04/10/2026, Morette): o template PADRAO e o vanilla, so que dentro dos storages. A
# QUANTIDADE de storages fica (fill por zona, ecossistema); sobe o numero de itens DENTRO de cada um ate o volume
# da zona bater com o loot de chao vanilla, com teto: no maximo ITEM_SCALE_MAX x o Min/Max base e MAX_ITEMS por
# storage (cargo 10x10). A maleta de armas e premium e fica fixa. Escassez e escolha do dono do servidor.
ITEM_SCALE_MAX = 3.0
# Nao reduz zona que ja passava do vanilla: com 0,5 o industrial (bau de ferramentas com 2 itens) ficou pobre
# no jogo (Morette, 04/10/2026). O tamanho base e o piso.
ITEM_SCALE_MIN = 1.0
MAX_ITEMS = 10
FIXED_SIZE = {"KLS_WeaponCase", "KLS_WeaponCase_Police", "KLS_WeaponCase_Hunting"}
# Carregador ou municao da PROPRIA arma por arma criada, em todo storage (Storages.json AmmoPerGun).
AMMO_PER_GUN = 1
# Chance por arma de vir esse carregador (04/10/2026, Morette: sempre vir o dela ficava "roubado"). O resto e o
# carregador/municao solto e aleatorio, como no vanilla. A maleta de armas e premium: 100%.
AMMO_CHANCE = 0.30
# "tools" com usage Medic que NAO e remedio (o radio saia no kit medico, 04/10/2026).
MEDICAL_EXCLUDE = {"personalradio"}
# Storages tematicos (C, 04/10/2026). Entram na divisao de slots da usage com peso THEME_WEIGHT (mais raros
# que os comuns), mas o que sai e do tema: pools da mistura, filtro de categoria, garantia e municao por arma.
#   classe, usages (onde nasce), tags, min, max, pools {usage: peso}, categorias dos sorteios livres,
#   garantia (usages ou None = as da mistura, sufixos da pool em ordem de preferencia, min, max, categorias)
#   ou None, carregadores por arma, tier minimo
# Maleta: arma LONGA garantida (pool _LongGuns, sem pistola): com a pool de armas inteira saia FNX45 em 30% das
# maletas e o T4 militar ficava com 30% de pistola (vanilla 19%) (04/10/2026). Numero de armas nao muda.
LONG_GUN = (None, ["_LongGuns", "_Guns"], 1, 1, None)
HUNTING_CASE_USAGES = ("Hunting", "Village")
# Material de construcao (pool Construction, garantia do bau de ferramentas). Pelo nominal vanilla.
CONSTRUCTION_ITEMS = {"nailbox", "metalwire", "barbedwire", "rope", "ducttape", "hammer", "handsaw", "hacksaw",
                      "hatchet", "woodaxe", "pliers", "shovel", "fieldshovel", "pickaxe", "sledgehammer",
                      "combinationlock", "combinationlock4", "netting", "camonet", "epoxyputty"}
# Item de loot SEM categoria no types vanilla (so 2 no Chernarus): ficavam fora das pools e ainda nasciam no chao.
# Categoria "drink" (04/10/2026): como "food", a garrafa (as vezes vazia) satisfazia a comida garantida.
NO_CATEGORY_AS = {"waterbottle": "drink", "glassbottle": "drink"}
THEMED = [
    ("KLS_WeaponCase",         ["Military"],                          ["floor", "shelves"], 1, 2, {"Military": 1}, ["ammo", "attachments"], LONG_GUN, 1, 2),
    ("KLS_WeaponCase_Police",  ["Police", "Prison"],                  ["floor", "shelves"], 1, 2, {"Police": 1},   ["ammo", "attachments"], LONG_GUN, 1, 2),
    ("KLS_WeaponCase_Hunting", ["Hunting"],                           ["floor", "shelves"], 1, 2, {"Hunting": 1},  ["ammo", "attachments"], ("pool:HuntingCase_T%d", [""], 1, 1, None), 1, 2),
    ("KLS_MedKit",             ["Medic", "Town", "Village", "Military"], ["floor", "shelves"], 2, 4, {"Medic": 1}, ["medical"], None, 0, 0),
    # 1 comida garantida: no T4 a pool militar quase nao tem comida (0,19 por vasculhada, 04/10/2026).
    # Comida da pool Food (todos os tiers): no T4 a unica comida das pools e a lata sem rotulo (04/10/2026).
    ("KLS_MilitarySupply",     ["Military"],                          ["floor"],            2, 4, {"Military": 1, "Town": 1}, ["food", "ammo"], ("pool:Food", [""], 1, 1, ["food"]), 0, 0),
    # Saco de comida um pouco mais cheio que os outros temas (04/10/2026, Morette).
    ("KLS_FoodSack",           ["Town", "Village", "Farm"],           ["floor"],            3, 6, {"Town": 1, "Village": 1, "Farm": 1}, ["food", "drink"], None, 0, 0),
    # Bau de ferramentas: 4 a 8 itens e 1 a 2 materiais de construcao garantidos (caixa de pregos, arame...).
    # Com 2 itens e so a pool de ferramentas, prego saia em ~7% dos baus (04/10/2026).
    ("KLS_ToolChest",          ["Industrial", "Farm"],                ["floor", "ground"],  4, 8, {"Industrial": 1, "Farm": 1}, ["tools"], ("pool:Construction", [""], 1, 2, None), 0, 0),
]
THEME_WEIGHT = 0.5
THEME_MIN_TIER = {t[0]: t[9] for t in THEMED}
ALL_STORAGES = STORAGES + [t[:5] for t in THEMED]

# Mistura de pools fixa por classe (peso por usage), no lugar da capacidade do mapa. Zona toxica: o predio
# la e militar, entao metade do loot e militar e metade e o exclusivo da zona (04/10/2026).
POOL_MIX = {"KLS_WoodenCrate_Toxic": {"ContaminatedArea": 1.0, "Military": 1.0}}
# Usages que o CE le do areaflags.map (plano de usage, bit = ordem do cfglimitsdefinition) e somam as do predio.
# So a zona toxica entra na conta: as outras (Coast, Hunting...) sao regioes enormes e mudariam o balanco todo.
AREA_USAGES = {"ContaminatedArea"}
# D) fill por zona (04/10/2026): militar T3/T4 e zona toxica sao regioes de risco e pagam mais. Custa pouco:
# militar e ~3% dos slots do mapa. Chave (usage, tier); tier None = todos.
FILL_ZONE = {("Military", 3): 0.40, ("Military", 4): 0.40, ("ContaminatedArea", None): 0.80}
# Ocupacao automatica das zonas de risco (opcao A, 11/10/2026): o fill acima foi afinado no Chernarus e em Livonia
# dava militar a 35% e zona toxica a 813% do vanilla. Para (usage, tier) daqui, o fill sobe ate o volume do KLS
# (itens no maximo) chegar a LOW x vanilla e desce ate o volume com o tamanho base nao passar de HIGH x vanilla,
# entre FILL_MIN e FILL_MAX. Metas do Chernarus validado (militar T1/T3/T4 72-75%, toxica 221-282%). As outras
# usages ficam no fill fixo: a quantidade de storages das cidades e a que o Morette validou.
FILL_TARGET = {"Military": (0.70, None), "ContaminatedArea": (None, 3.0)}
FILL_MIN, FILL_MAX = 0.05, 0.60
# Pool so com os itens que TEM a usage (sem os genericos sem usage, que ja estao em todas as outras pools):
# na zona toxica a lanterna e o refrigerante diluiam os exclusivos para ~10% (04/10/2026).
EXCLUSIVE_POOL_USAGES = {"ContaminatedArea"}
# Sazonais: saem do chao (override) mas nao entram nas pools (03/10/2026: pisca-pisca de Natal em outubro).
SEASONAL_ITEMS = {"xmaslights"}
SEASONAL_USAGES = {"SeasonalEvent"}
TIERS = [1, 2, 3, 4]
STORAGE_LIFETIME = 14400   # 4 h sem vasculhar; o CE rotaciona (balanceamento fino e a F4)
# Peso inteiro (D-29): o peso do template x 1000, arredondado. Escala UNICA para todas as pools: o Server compara
# peso entre pools para escolher o carregador da arma (GetItemWeight); normalizar cada pool para 10.000 levava o
# STANAG de 60 de 50% para 96% (a pool da zona toxica e pequena). Com x1000 nenhuma pool muda mais de 0,8%.
WEIGHT_SCALE = 1000


def die(msg):
    print("ERRO: " + msg)
    sys.exit(1)


def read_xml(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        txt = f.read()
    return txt, ET.fromstring(txt.encode("utf-8"))


def ival(e, tag, default=0):
    v = e.findtext(tag)
    try:
        return int(float(v)) if v is not None else default
    except ValueError:
        return default


def flag(t, name):
    f = t.find("flags")
    return f is not None and f.get(name) == "1"


# ---------------------------------------------------------------------------------------------------------
# Leitura da missao
# ---------------------------------------------------------------------------------------------------------
def load_types(mission):
    _, root = read_xml(os.path.join(mission, "db", "types.xml"))
    types = {}
    for t in root.findall("type"):
        cat = t.find("category")
        types[t.get("name")] = {
            "nominal": ival(t, "nominal"),
            "quantmin": ival(t, "quantmin", -1),
            "quantmax": ival(t, "quantmax", -1),
            "category": cat.get("name") if cat is not None else NO_CATEGORY_AS.get((t.get("name") or "").lower()),
            "usages": [u.get("name") for u in t.findall("usage") if u.get("name")],
            "tiers": [int(v.get("name")[4:]) for v in t.findall("value") if (v.get("name") or "").startswith("Tier")],
            "deloot": flag(t, "deloot"),
        }
    return types


def is_loot(t):
    """Item de chao controlado pelo KLS: nominal > 0, nao e loot de evento, nao e dispatch (D-10, D-19)."""
    return t["nominal"] > 0 and not t["deloot"] and t["category"] not in (None, "lootdispatch")


def is_seasonal(name, t):
    return name.lower() in SEASONAL_ITEMS or bool(set(t["usages"]) & SEASONAL_USAGES)


def load_spawnable(mission):
    _, presets_root = read_xml(os.path.join(mission, "cfgrandompresets.xml"))
    presets = {}
    for p in presets_root:
        presets[(p.tag, p.get("name"))] = (float(p.get("chance") or 1.0), [(i.get("name"), float(i.get("chance") or 1.0)) for i in p.findall("item")])

    _, root = read_xml(os.path.join(mission, "cfgspawnabletypes.xml"))
    spawn = {}
    for t in root.findall("type"):
        entry = {"damage": None, "attachments": [], "cargo": []}
        dmg = t.find("damage")
        if dmg is not None:
            entry["damage"] = [float(dmg.get("min") or 0), float(dmg.get("max") or 0)]
        for block in t:
            if block.tag not in ("attachments", "cargo"):
                continue
            if block.get("preset"):
                chance, items = presets.get((block.tag, block.get("preset")), (0.0, []))
            else:
                chance = float(block.get("chance") or 1.0)
                items = [(i.get("name"), float(i.get("chance") or 1.0)) for i in block.findall("item")]
            items = [(n, c) for n, c in items if n]
            if items:
                entry[block.tag].append((chance, items))
        spawn[t.get("name")] = entry
    return spawn


def proto_source(mission, filename="mapgroupproto.xml"):
    path = os.path.join(mission, filename)
    with open(path, encoding="utf-8", errors="replace") as f:
        head = f.read(4096)
    if MARK not in head:
        return path
    # Mais novo primeiro: depois de um update do DayZ o backup antigo e da versao anterior (17/10/2026, 1.29 -> 1.30).
    backups = sorted(glob.glob(path + ".bak-kls-*"), reverse=True)
    for b in backups:
        with open(b, encoding="utf-8", errors="replace") as f:
            if MARK not in f.read(4096):
                return b
    die("o %s da missao ja e um template do KLS e nao ha backup original (*.bak-kls-*)." % filename)


def event_groups(mission):
    names = set()
    for rel in ("db/events.xml", "cfgeventgroups.xml"):
        path = os.path.join(mission, rel)
        if not os.path.exists(path):
            continue
        _, root = read_xml(path)
        for e in root.iter():
            for key in ("type", "name"):
                if e.tag in ("child",) and e.get(key):
                    names.add(e.get(key).lower())
    return names


def deloot_wrecks(mission):
    """Tipos de wreck que sao filho deloot="1" no cfgeventgroups (comboio, blitz policial). O lootmax do filho
    vale para o loot raro do evento E para os storages: o gerador nao mexe nele (04/10/2026)."""
    path = os.path.join(mission, "cfgeventgroups.xml")
    if not os.path.exists(path):
        return set()
    _, root = read_xml(path)
    return {c.get("type").lower() for c in root.iter("child") if c.get("deloot") == "1" and c.get("type")}


def dayz_version():
    rpts = sorted(glob.glob(os.path.join(SERVER, "Profiles", "*.RPT")), key=os.path.getmtime, reverse=True)
    for r in rpts[:5]:
        with open(r, encoding="utf-8", errors="replace") as f:
            for line in f:
                m = re.match(r"\s*Version\s+([\d.]+)", line)
                if m:
                    return m.group(1)
    return "desconhecida"


# ---------------------------------------------------------------------------------------------------------
# Geracao
# ---------------------------------------------------------------------------------------------------------
# Eventos cujo loot e o de evento (deloot): heli crash e Natal. O group fica intocado e SEM a categoria
# "containers", para storage nunca nascer ali (D-10). Os outros eventos (comboio, viatura, trem, caixa de
# aviao) usam loot comum, que o override zerou: recebem storage com teto --event-cap (decisao 03/10/2026).
DELOOT_EVENT_GROUPS = {"wreck_uh1y", "wreck_santassleigh", "christmastree", "bonfire"}
# O NOME do evento e igual em todo mapa; o wreck muda (Livonia/Sakhal: Wreck_Mi8_Crashed e ChristmasTree_Green; o
# bonfire de Sakhal e Misc_TirePile_Burning_DE). add_deloot_event_groups poe os wrecks do mapa no set acima (11/10/2026).
DELOOT_EVENTS = {"statichelicrash", "staticchristmastree", "staticbonfire", "staticsantacrash"}


def add_deloot_event_groups(mission):
    _, root = read_xml(os.path.join(mission, "db", "events.xml"))
    for e in root.findall("event"):
        if (e.get("name") or "").lower() in DELOOT_EVENTS:
            DELOOT_EVENT_GROUPS.update(c.get("type").lower() for c in e.iter("child") if c.get("type"))


def build_proto(src_path, cap, event_cap, events, slot_groups=frozenset()):
    """Edita no texto so o lootmax de cada group e a categoria "containers" dos containers."""
    with open(src_path, encoding="utf-8", errors="replace") as f:
        txt = f.read()
    if "<defaults>" not in txt:
        die("mapgroupproto.xml sem <defaults>: formato inesperado.")
    default_group = 6
    m = re.search(r'<default\s+name="group"\s+lootmax="(\d+)"', txt)
    if m:
        default_group = int(m.group(1))
    stats = Counter()

    def add_category(block):
        if 'category name="containers"' in block:
            return block
        stats["category"] += 1
        indent = re.search(r"\n([ \t]*)<(category|tag|point)", block)
        pad = indent.group(1) if indent else "\t\t\t\t\t\t"
        first = block.index(">") + 1
        return block[:first] + "\n" + pad + '<category name="containers" />' + block[first:]

    def remove_category(block):
        if 'category name="containers"' not in block:
            return block
        stats["category_removed"] += 1
        return re.sub(r'\n[ \t]*<category name="containers"\s*/>', "", block)

    container_re = r"<container\b[^>]*>.*?</container>"

    def with_kls_container(rest):
        """Wreck deloot: o lootmax do filho no cfgeventgroups (3-4) e dividido com o loot raro do evento, entao
        o teto de storage vai num container SO de storage, com lootmax = --event-cap e os pontos/tags do
        primeiro container. Os outros containers perdem a categoria "containers" (decisao 04/10/2026, opcao C)."""
        rest = re.sub(container_re, lambda c: re.sub(r'\n[ \t]*<category name="containers"\s*/>', "", c.group(0)), rest, flags=re.S)
        first = re.search(container_re, rest, re.S)
        if not first:
            return rest
        kls = re.sub(r"<container\b[^>]*>", '<container name="lootKLS" lootmax="%d">' % event_cap, first.group(0), count=1)
        kls = re.sub(r'\n[ \t]*<category name="[^"]+"\s*/>', "", kls)
        indent = re.search(r"\n([ \t]*)<(tag|point)", kls)
        pad = indent.group(1) if indent else "\t\t\t\t\t\t"
        head_end = kls.index(">") + 1
        kls = kls[:head_end] + "\n" + pad + '<category name="containers" />' + kls[head_end:]
        line_pad = re.search(r"([ \t]*)$", rest[:first.start()]).group(1)
        stats["kls_container"] += 1
        return rest[:first.end()] + "\n" + line_pad + kls + rest[first.end():]

    def fix_group(mo):
        block = mo.group(0)
        head_end = block.index(">") + 1
        head, rest = block[:head_end], block[head_end:]
        name = re.search(r'name="([^"]+)"', head).group(1).lower()
        if name in DELOOT_EVENT_GROUPS:
            stats["deloot_event"] += 1
            return head + re.sub(r"<container\b[^>]*>.*?</container>", lambda c: remove_category(c.group(0)), rest, flags=re.S)
        limit = event_cap if name in events else cap
        lm = re.search(r'lootmax="(\d+)"', head)
        cur = int(lm.group(1)) if lm else default_group
        if cur > limit:
            stats["event_capped" if name in events else "capped"] += 1
            if lm:
                head = head.replace(lm.group(0), 'lootmax="%d"' % limit)
            else:
                head = head[:-1] + ' lootmax="%d">' % limit
        if name in slot_groups:
            return head + with_kls_container(rest)
        # Todo container aceita storage: o loot de chao esta zerado, entao so os storages sao afetados.
        return head + re.sub(r"<container\b[^>]*>.*?</container>", lambda c: add_category(c.group(0)), rest, flags=re.S)

    head_txt, body = txt.split("<defaults>", 1)
    body = re.sub(r"<group\s+name=\"[^\"]+\"[^>]*>.*?</group>", fix_group, body, flags=re.S)
    out = head_txt + "<defaults>" + body
    note = "<!-- %s (%s): group lootmax <= %d, evento <= %d; heli crash e Natal intocados e sem storage -->" % (MARK, datetime.date.today(), cap, event_cap)
    out = re.sub(r"(<\?xml[^>]*\?>)", lambda mo: mo.group(1) + "\n" + note, out, count=1)
    return out, stats


def load_item_sizes(dz_root):
    """itemSize de cada classe dos configs do jogo (o itemSize nao esta na missao). Parser leve: pilha de
    classes por chaves; heranca resolvida depois. Classe sem config conhecida = cabe (nao e excluida)."""
    sizes, parents, roots = {}, {}, {}
    if not os.path.isdir(dz_root):
        print("AVISO: %s nao existe; sem itemSize e sem pools de arma." % dz_root)
        return (lambda name: None), (lambda name: None), (lambda name, base: False)
    # Tokens: cabecalho de classe, itemSize, e chaves soltas. "};" de array (units[]={};) NAO fecha classe:
    # cada classe guarda quantas chaves de array estao abertas dentro dela.
    tok = re.compile(r"class\s+(\w+)\s*(?::\s*(\w+))?\s*\{|itemSize\[\]\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*\}|\{|\}")
    for root, _, files in os.walk(dz_root):
        for fn in files:
            if fn.lower() != "config.cpp":
                continue
            with open(os.path.join(root, fn), encoding="utf-8", errors="replace") as f:
                text = f.read()
            stack = []   # [nome, chaves de array abertas]
            for mo in tok.finditer(text):
                t = mo.group(0)
                if mo.group(1):
                    cname = mo.group(1).lower()
                    if len(stack) == 1:
                        roots.setdefault(cname, stack[0][0])
                    stack.append([cname, 0])
                    if mo.group(2):
                        parents.setdefault(cname, mo.group(2).lower())
                elif mo.group(3):
                    if stack and stack[-1][1] == 0:
                        sizes.setdefault(stack[-1][0], (int(mo.group(3)), int(mo.group(4))))
                elif t == "{":
                    if stack:
                        stack[-1][1] += 1
                elif t == "}":
                    if stack:
                        if stack[-1][1] > 0:
                            stack[-1][1] -= 1
                        else:
                            stack.pop()

    def size_of(name):
        n, seen = name.lower(), set()
        while n and n not in seen:
            if n in sizes:
                return sizes[n]
            seen.add(n)
            n = parents.get(n)
        return None

    def root_of(name):
        return roots.get(name.lower())

    def is_kind(name, base):
        n, seen, base = name.lower(), set(), base.lower()
        while n and n not in seen:
            if n == base:
                return True
            seen.add(n)
            n = parents.get(n)
        return False
    return size_of, root_of, is_kind


def load_item_export(path):
    """Export/items.json gravado pelo proprio mod (ExportItems = 1 no Settings.json, D-31): tamanho, raiz do config e
    cadeia de classes base de cada item scope 2 da config carregada no servidor, inclusive de outros mods. Substitui o
    --dz (configs extraidos do jogo), que o dono do servidor nao tem. Mesma interface do load_item_sizes."""
    with open(path, encoding="utf-8") as f:
        items = {i["Item"].lower(): i for i in json.load(f)["Items"]}

    def size_of(name):
        i = items.get(name.lower())
        return tuple(i["Size"][:2]) if i and len(i.get("Size") or []) >= 2 else None

    def root_of(name):
        i = items.get(name.lower())
        return i["Root"].lower() if i else None

    def is_kind(name, base):
        n, b = name.lower(), base.lower()
        i = items.get(n)
        return n == b or bool(i) and b in (x.lower() for x in i.get("Bases") or [])
    return size_of, root_of, is_kind


def cap_child(child, event_cap):
    """Limita lootmin/lootmax de um <child> de evento. Devolve True se mudou."""
    changed = False
    for key in ("lootmax", "lootmin"):
        v = child.get(key)
        if v is not None and v.lstrip("-").isdigit() and int(v) > event_cap:
            child.set(key, str(event_cap))
            changed = True
    return changed


def build_event_overrides(mission, event_cap):
    """O loot comum de comboio/viatura/trem/caixa de aviao vem do lootmin/lootmax do PROPRIO evento
    (events.xml e cfgeventgroups.xml), nao do mapgroupproto: com o loot de chao zerado, o CE enchia o evento
    de storages (14 na caixa de aviao de NWAF, 03/10/2026). Aqui o teto vira --event-cap por wreck.
    Heli crash e Natal (DELOOT_EVENT_GROUPS) ficam como estao.
    Saidas: kls_events.xml (so os eventos alterados, pela pasta ce) e o cfgeventgroups.xml inteiro."""
    _, ev_root = read_xml(os.path.join(mission, "db", "events.xml"))
    out_events, changed_events = [], []
    for e in ev_root.findall("event"):
        kids = e.find("children")
        if kids is None or not len(kids):
            continue
        types_ = {(c.get("type") or "").lower() for c in kids}
        if types_ & DELOOT_EVENT_GROUPS:
            continue
        # Zumbi e animal: o lootmax do filho e o loot que ele CARREGA (D-14: zumbi fica vanilla).
        if any(t.startswith(("zmb", "animal_")) for t in types_):
            continue
        # Lista (nao gerador): any() pararia no primeiro filho e so ele ficaria com teto.
        if any([cap_child(c, event_cap) for c in list(kids)]):
            out_events.append(e)
            changed_events.append(e.get("name"))
    events_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>\n'
    events_xml += "<!-- %s: eventos de loot comum com teto %d storage(s) por wreck (override pela pasta ce) -->\n<events>\n" % (MARK, event_cap)
    for e in out_events:
        events_xml += "    " + ET.tostring(e, encoding="unicode").strip() + "\n"
    events_xml += "</events>\n"

    src = proto_source(mission, "cfgeventgroups.xml")
    with open(src, encoding="utf-8", errors="replace") as f:
        txt = f.read()
    changed_groups = Counter()

    def fix_child(mo):
        tag = mo.group(0)
        if re.search(r'deloot="1"', tag):
            return tag
        new = tag
        for key in ("lootmax", "lootmin"):
            m = re.search(r'%s="(\d+)"' % key, new)
            if m and int(m.group(1)) > event_cap:
                new = new.replace(m.group(0), '%s="%d"' % (key, event_cap))
        if new != tag:
            changed_groups["child"] += 1
        return new

    groups_xml = re.sub(r"<child\b[^>]*>", fix_child, txt)
    note = "<!-- %s (%s): lootmin/lootmax de cada wreck <= %d -->" % (MARK, datetime.date.today(), event_cap)
    groups_xml = re.sub(r"(<\?xml[^>]*\?>)", lambda mo: mo.group(1) + "\n" + note, groups_xml, count=1)
    return events_xml, changed_events, groups_xml, changed_groups["child"]


def load_tiers(mission):
    """areaflags.map: cabecalho de 6 int32 (w, h, mundo_w, mundo_h, 32, 0), depois um plano uint32 de usage e um
    plano de 1 byte de value (bit 0..3 = Tier1..Tier4, bit 4 = Unique). Linha 0 = sul (conferido 03/10/2026:
    Tisy T4, NWAF/Stary Sobor T3, Chernogorsk T2, Elektro T1)."""
    import struct
    with open(os.path.join(mission, "areaflags.map"), "rb") as f:
        data = f.read()
    w, h, ww, wh, _, _ = struct.unpack("<6i", data[:24])
    values = data[24 + 4 * w * h:]
    # Livonia (11/10/2026): o plano de value vem com 4 bits por celula (2 por byte, a de x par no nibble baixo:
    # 99,75% de vizinhos iguais contra 99,69% na ordem inversa), sem o bit Unique. Chernarus e Sakhal: 1 byte.
    if len(values) == w * h:
        def value_at(i):
            return values[i]
    elif len(values) * 2 == w * h:
        def value_at(i):
            b = values[i >> 1]
            return b >> 4 if i & 1 else b & 15
    else:
        die("areaflags.map com tamanho inesperado.")

    def tiers_at(x, z):
        px = min(w - 1, max(0, int(x / ww * w)))
        pz = min(h - 1, max(0, int(z / wh * h)))
        v = value_at(pz * w + px)
        return [n for n in TIERS if v & (1 << (n - 1))]
    return tiers_at


def load_area_usages(mission):
    """Plano uint32 de usage do areaflags.map: bit i = i-esima <usage> do cfglimitsdefinition.xml (conferido
    04/10/2026: bit 15 = ContaminatedArea no navio de Rify e no sul de Pavlovo). Devolve so as de AREA_USAGES."""
    import struct
    _, lim = read_xml(os.path.join(mission, "cfglimitsdefinition.xml"))
    order = [u.get("name") for u in lim.iter("usage")]
    bits = {i: name for i, name in enumerate(order) if name in AREA_USAGES}
    with open(os.path.join(mission, "areaflags.map"), "rb") as f:
        data = f.read()
    w, h, ww, wh, _, _ = struct.unpack("<6i", data[:24])

    def usages_at(x, z):
        px = min(w - 1, max(0, int(x / ww * w)))
        pz = min(h - 1, max(0, int(z / wh * h)))
        v = struct.unpack_from("<I", data, 24 + 4 * (pz * w + px))[0]
        return [name for i, name in bits.items() if v >> i & 1]
    return usages_at


def capacity(mission, proto_path, cap):
    _, root = read_xml(proto_path)
    gdef, cdef = 6, 4
    d = root.find("defaults")
    if d is not None:
        for x in d:
            if x.get("name") == "group" and x.get("lootmax"):
                gdef = int(x.get("lootmax"))
            if x.get("name") == "container" and x.get("lootmax"):
                cdef = int(x.get("lootmax"))
    per_group = {}
    for g in root.findall("group"):
        glm = int(g.get("lootmax") or gdef)
        slots_vanilla = 0
        slots_kls = 0
        for c in g.findall("container"):
            pts = len(c.findall("point"))
            clm = min(int(c.get("lootmax") or cdef), pts)
            slots_vanilla += clm
            # O template poe a categoria "containers" em todo container (build_proto), entao todos contam.
            slots_kls += clm
        per_group[g.get("name").lower()] = {
            "usages": [u.get("name") for u in g.findall("usage")],
            "vanilla": min(glm, slots_vanilla),
            "kls": min(glm, cap, slots_kls),
        }
    _, pos = read_xml(os.path.join(mission, "mapgrouppos.xml"))
    tiers_at = load_tiers(mission)
    area_usages_at = load_area_usages(mission)
    usage_cap = defaultdict(float)
    usage_tier_cap = defaultdict(float)   # (usage, tier) -> slots; tier 0 = predio em area sem tier
    tot_vanilla = tot_kls = 0
    for g in pos.findall("group"):
        pg = per_group.get(g.get("name").lower())
        if not pg:
            continue
        tot_vanilla += pg["vanilla"]
        tot_kls += pg["kls"]
        if not pg["usages"] or not pg["kls"]:
            continue
        coords = [float(c) for c in (g.get("pos") or "0 0 0").split()]
        tiers = tiers_at(coords[0], coords[2]) or [0]
        usages = pg["usages"] + [u for u in area_usages_at(coords[0], coords[2]) if u not in pg["usages"]]
        for u in usages:
            share = pg["kls"] / len(usages)
            usage_cap[u] += share
            for t in tiers:
                usage_tier_cap[(u, t)] += share / len(tiers)
    return usage_cap, usage_tier_cap, tot_vanilla, tot_kls


def pool_item(name, types):
    """Linha da pool (formato enxuto, D-29): so o item e o peso (nominal). O kit vai para o Kits.json."""
    return {"Item": name, "Weight": float(types.get(name, {}).get("nominal", 1) or 1)}


def item_kit(name, types, spawn):
    """Kit padrao do item no Kits.json (D-29): dano e attachments/cargo do cfgspawnabletypes, quantidade do types.
    Os itens de dentro dos grupos levam so nome e peso: o kit deles e a entrada deles no Kits.json, e o Server
    abre grupos de kit ate o 2o nivel (arma > luneta > pilha). None = item sem kit."""
    t = types.get(name, {})
    s = spawn.get(name) or {"damage": None, "attachments": [], "cargo": []}
    kit = {"Item": name}
    if s["damage"]:
        kit["Damage"] = s["damage"]
    if t.get("quantmin", -1) >= 0 and t.get("quantmax", -1) >= 0:
        kit["Quantity"] = [round(t["quantmin"] / 100.0, 3), round(t["quantmax"] / 100.0, 3)]
    for key, out in (("attachments", "Attachments"), ("cargo", "Cargo")):
        groups = [{"Chance": chance, "Items": [{"Item": child, "Weight": weight} for child, weight in items]}
                  for chance, items in s[key]]
        if groups:
            kit[out] = groups
    return kit if len(kit) > 1 else None


def build_kits(pools, types, spawn):
    """Kit de todo item das pools e, em cadeia, de todo item de dentro de um kit."""
    kits, seen = {}, set()
    todo = [it["Item"] for p in pools.values() for it in p["Items"]]
    while todo:
        name = todo.pop()
        if name in seen:
            continue
        seen.add(name)
        kit = item_kit(name, types, spawn)
        if kit:
            kits[name] = kit
            todo += [i["Item"] for out in ("Attachments", "Cargo") for g in kit.get(out, []) for i in g["Items"]]
    return [kits[n] for n in sorted(kits, key=str.lower)]


def integer_weights(items):
    """Peso inteiro: x WEIGHT_SCALE e arredondado; item raro fica com 1, nunca com 0."""
    for i in items:
        i["Weight"] = max(1, int(round(i["Weight"] * WEIGHT_SCALE)))


def write_pool(path, items):
    """Uma linha por item: o admin le, busca e copia linha."""
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write('{\n  "Items": [\n    ' + ",\n    ".join(json.dumps(i) for i in items) + "\n  ]\n}\n")


def write_kits(path, kits):
    """Kit sem grupo numa linha; kit com grupo abre uma linha por grupo (como o cfgspawnabletypes)."""
    rows = []
    for kit in kits:
        head = {k: v for k, v in kit.items() if k not in ("Attachments", "Cargo")}
        if "Attachments" not in kit and "Cargo" not in kit:
            rows.append(json.dumps(head))
            continue
        parts = ['"%s": %s' % (k, json.dumps(v)) for k, v in head.items()]
        for out in ("Attachments", "Cargo"):
            if out in kit:
                parts.append('"%s": [\n        ' % out + ",\n        ".join(json.dumps(g) for g in kit[out]) + "\n      ]")
        rows.append("{\n      " + ",\n      ".join(parts) + "\n    }")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write('{\n  "Kits": [\n    ' + ",\n    ".join(rows) + "\n  ]\n}\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", default="chernarusplus")
    ap.add_argument("--mission")
    ap.add_argument("--out")
    ap.add_argument("--cap", type=int, default=2, choices=[1, 2])
    ap.add_argument("--min-ratio", type=float, default=0.8, help="min = nominal x isto; abaixo do min o CE repoe (vanilla ~0.5)")
    ap.add_argument("--restock", type=int, default=600, help="segundos que o CE espera para repor um tipo abaixo do min")
    ap.add_argument("--event-cap", type=int, default=1, help="teto de storages por wreck de evento com loot comum")
    ap.add_argument("--dz", default=r"P:\DZ", help="configs do jogo extraidos (so para o itemSize)")
    ap.add_argument("--items", help="Export/items.json do mod (ExportItems = 1); no lugar do --dz")
    ap.add_argument("--gun-boost", type=float, default=3.0, help="teto do multiplicador de arma por zona (volume vanilla / volume KLS)")
    ap.add_argument("--fill", type=float, default=0.15, help="fracao dos slots com teto ocupada por storages")
    a = ap.parse_args()

    mission = a.mission or os.path.join(SERVER, "mpmissions", "dayzOffline." + a.map)
    here = os.path.dirname(os.path.abspath(__file__))
    out = a.out or os.path.normpath(os.path.join(here, "..", "..", "template", a.map))
    if not os.path.isdir(mission):
        die("mission folder not found / missao nao encontrada: %s (use --mission <folder>)" % mission)
    # Sem dados de item o template sai sem pools de arma, calado: exige o items.json do mod (D-31) ou o P:\DZ.
    if not a.items and not os.path.isdir(a.dz):
        die("item data missing / faltam os dados de item: set ExportItems to 1 in Settings.json, start the server once "
            "and pass --items <profiles>/Kryden/KrydenLootSystem/Export/items.json")
    if a.items and not os.path.isfile(a.items):
        die("items.json not found / nao encontrado: " + a.items)

    types = load_types(mission)
    spawn = load_spawnable(mission)
    loot = {n: t for n, t in types.items() if is_loot(t)}
    # Item que nao cabe no cargo dos storages (10x10) ou que e objeto de base (FLOOR_ALWAYS) fica no chao como
    # no vanilla: fora do override e das pools (decisao 03/10/2026: barril, sea chest etc. sumiam do jogo).
    size_of, root_of, is_kind = load_item_export(a.items) if a.items else load_item_sizes(a.dz)

    def fits_cargo(name):
        sz = size_of(name)
        if not sz:
            return True
        w, h = sz
        return (w <= CARGO_W and h <= CARGO_H) or (h <= CARGO_W and w <= CARGO_H)
    floor_items = sorted(n for n in loot if not fits_cargo(n) or n.lower() in FLOOR_ALWAYS)
    for n in floor_items:
        del loot[n]
    # Item cuja usage nenhum storage atende (Historical, Lunapark, Underground, Special) tambem fica no chao: o
    # override zerava e ele nao estava em pool nenhuma, entao sumia do jogo (armadura medieval dos castelos, loot do
    # bunker de Livonia; 11/10/2026). Sazonal segue a D-22 (sai do chao e nao entra em pool).
    served_usages = {u for _, usages, _, _, _ in ALL_STORAGES for u in usages}
    unserved_items = sorted(n for n, t in loot.items()
                            if t["usages"] and not is_seasonal(n, t) and not set(t["usages"]) & served_usages)
    for n in unserved_items:
        del loot[n]
    skip = event_groups(mission)
    proto_path = proto_source(mission)
    usage_cap, usage_tier_cap, tot_vanilla, tot_kls = capacity(mission, proto_path, a.cap)
    _, vanilla_tier_cap, _, _ = capacity(mission, proto_path, 999)   # sem teto = slots vanilla

    # Fracao de arma no loot vanilla da zona: cada item cai na zona na proporcao dos slots elegiveis dele que
    # ficam nela (nominal x slots(u,n) / slots elegiveis). Mesma regra de elegibilidade das pools.
    all_usages = sorted({u for (u, _) in vanilla_tier_cap})

    def in_pool(u, n, t):
        return ((u in t["usages"] or (not t["usages"] and u not in EXCLUSIVE_POOL_USAGES))
                and (n in t["tiers"] or not t["tiers"]))

    eligible_slots = {}
    for name, t in loot.items():
        us = t["usages"] or all_usages
        ts = t["tiers"] or [0] + TIERS
        eligible_slots[name] = sum(vanilla_tier_cap.get((u, x), 0) for u in us for x in ts)

    def vanilla_total(u, n):
        """Itens que o vanilla poe no chao da zona (u, n): nominal x slots(u, n) / slots elegiveis do item."""
        return sum(t["nominal"] * vanilla_tier_cap.get((u, n), 0) / eligible_slots[name] for name, t in loot.items()
                   if not is_seasonal(name, t) and eligible_slots.get(name) and in_pool(u, n, t))

    base_size = {cls: (mn, mx) for cls, _, _, mn, mx in ALL_STORAGES}

    def avg(size):
        return (size[0] + size[1]) / 2.0

    fill_used = {}

    def zone_fill(u, n, here, weight):
        """Fill da zona: o fixo (FILL_ZONE / --fill), corrigido pelas metas do FILL_TARGET."""
        f = FILL_ZONE.get((u, n), FILL_ZONE.get((u, None), a.fill))
        low, high = FILL_TARGET.get(u, (None, None))
        slots = usage_tier_cap.get((u, n), 0)
        if (low is None and high is None) or not here or not slots:
            return f
        v = vanilla_total(u, n)
        if v <= 0:
            return f
        tw = sum(weight.values())
        base = sum(weight[c] * avg(base_size[c]) for c in here) / tw
        top = sum(weight[c] * (avg(base_size[c]) if c in FIXED_SIZE else min(MAX_ITEMS, avg(base_size[c]) * ITEM_SCALE_MAX))
                  for c in here) / tw
        if low is not None and f * slots * top < low * v:
            f = min(FILL_MAX, low * v / (slots * top))
        if high is not None and f * slots * base > high * v:
            f = max(FILL_MIN, high * v / (slots * base))
        return f

    # --- quantos storages: fill x slots, por usage, dividido entre as classes da usage e pelos tiers ------
    served = defaultdict(list)
    for cls, usages, _, _, _ in ALL_STORAGES:
        for u in usages:
            served[u].append(cls)
    # Tier pela POSICAO de cada predio (areaflags.map), nao pelo peso dos itens: no Chernarus a area T4 e
    # ~2% do mapa e dividir pelo item mandou storage T4 demais (CE: "search overtime", 03/10/2026).
    nominal = Counter()
    zone_storages = defaultdict(float)      # (cls, n, u) -> storages da classe na zona (u, n)
    for u, classes in served.items():
        for n in [0] + TIERS:
            slots = usage_tier_cap.get((u, n), 0)
            # Tema com tier minimo (maleta de armas so de T2 para cima) nao divide os slots dos tiers abaixo.
            here = [cls for cls in classes if n >= THEME_MIN_TIER.get(cls, 0)]
            weight = {cls: (THEME_WEIGHT if cls in THEME_MIN_TIER else 1.0) for cls in here}
            total_weight = sum(weight.values()) or 1
            fill = fill_used[(u, n)] = zone_fill(u, n, here, weight)
            for cls in here:
                nominal[(cls, n)] += fill * slots * weight[cls] / total_weight
                zone_storages[(cls, n, u)] += fill * slots * weight[cls] / total_weight

    # --- kls_types.xml ---------------------------------------------------------------------------------
    lines = ['<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>',
             "<!-- %s (%s, %s, DayZ %s): storages + override parcial (nominal/min = 0) do loot de chao -->" % (MARK, a.map, datetime.date.today(), dayz_version()),
             "<types>"]

    def storage_type(name, nom, usages, tags, tier):
        r = ['    <type name="%s">' % name,
             "        <nominal>%d</nominal>" % nom,
             "        <lifetime>%d</lifetime>" % STORAGE_LIFETIME,
             "        <restock>%d</restock>" % a.restock,
             "        <min>%d</min>" % (min(nom, max(1, round(nom * a.min_ratio))) if nom else 0),
             "        <quantmin>-1</quantmin>",
             "        <quantmax>-1</quantmax>",
             "        <cost>100</cost>",
             '        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" crafted="0" deloot="0"/>',
             '        <category name="containers"/>']
        r += ['        <tag name="%s"/>' % x for x in tags]
        r += ['        <usage name="%s"/>' % x for x in usages]
        if tier:
            r.append('        <value name="Tier%d"/>' % tier)
        r.append("    </type>")
        return r

    total_storages = 0
    class_rows = []
    lines.append("    <!-- ===== Storages (D-18): a classe sem sufixo e a variante 'sem tier' (predio em area sem tier) ===== -->")
    for cls, usages, tags, _, _ in ALL_STORAGES:
        base_nom = int(round(nominal[(cls, 0)]))
        total_storages += base_nom
        class_rows.append((cls, base_nom))
        lines += storage_type(cls, base_nom, usages, tags, None)
        for n in TIERS:
            nom = int(round(nominal[(cls, n)]))
            total_storages += nom
            class_rows.append((cls + "_T%d" % n, nom))
            lines += storage_type("%s_T%d" % (cls, n), nom, usages, tags, n)
    lines.append("    <!-- ===== Override parcial: o CE para de espalhar o loot de chao (D-19). deloot e lootdispatch ficam fora ===== -->")
    for name in sorted(loot):
        lines += ['    <type name="%s">' % name, "        <nominal>0</nominal>", "        <min>0</min>", "    </type>"]
    lines.append("</types>")

    add_deloot_event_groups(mission)
    proto_txt, proto_stats = build_proto(proto_path, a.cap, a.event_cap, skip, deloot_wrecks(mission) - DELOOT_EVENT_GROUPS)
    events_xml, changed_events, groups_xml, changed_children = build_event_overrides(mission, a.event_cap)

    # --- pools e Storages.json -------------------------------------------------------------------------
    def is_gun(name):
        return root_of(name) == "cfgweapons"

    def category_of(name):
        """Categoria do filtro dos storages tematicos. A "weapons" do types vira guns/ammo/attachments pela raiz
        do config; "tools" com usage Medic vira medical (o types poe remedio em tools)."""
        t = types.get(name, {})
        c = t.get("category") or ""
        if c == "weapons":
            r = root_of(name)
            return "guns" if r == "cfgweapons" else ("ammo" if r == "cfgmagazines" else "attachments")
        if c == "tools" and "Medic" in t.get("usages", []) and name.lower() not in MEDICAL_EXCLUDE:
            return "medical"
        return c

    long_gun_usages = {u for t in THEMED if t[7] and "_LongGuns" in t[7][1] and not isinstance(t[7][0], str) for u in (t[7][0] or t[5])}
    pools = {}
    pool_usages = set(served) | {u for mix in POOL_MIX.values() for u in mix}
    for u in sorted(pool_usages):
        for n in TIERS:
            names = [name for name, t in sorted(loot.items())
                     if (u in t["usages"] or (not t["usages"] and u not in EXCLUSIVE_POOL_USAGES))
                     and (n in t["tiers"] or not t["tiers"])
                     and not is_seasonal(name, t)]
            gear = [dict(pool_item(x, types), Category=category_of(x)) for x in names
                    if not is_gun(x) and category_of(x) != "explosives"]
            guns = [dict(pool_item(x, types), Category=category_of(x)) for x in names if is_gun(x)]
            expl = [dict(pool_item(x, types), Category="explosives") for x in names
                    if not is_gun(x) and category_of(x) == "explosives"]
            if gear:
                pools["%s_T%d" % (u, n)] = {"Items": gear}
            if expl:
                pools["%s_T%d_Explosives" % (u, n)] = {"Items": expl}
            if guns:
                pools["%s_T%d_Guns" % (u, n)] = {"Items": guns}
                long_guns = [g for g in guns if not is_kind(g["Item"], "Pistol_Base")]
                if long_guns and u in long_gun_usages:
                    pools["%s_T%d_LongGuns" % (u, n)] = {"Items": long_guns}
    # Maleta de caca: arma longa de caca E de vila do tier. So caca dava sempre besta no T2 (04/10/2026, opcao A).
    for n in TIERS:
        merged = {}
        for u in HUNTING_CASE_USAGES:
            for g in pools.get("%s_T%d_Guns" % (u, n), {}).get("Items", []):
                if not is_kind(g["Item"], "Pistol_Base"):
                    merged.setdefault(g["Item"], g)
        if merged:
            pools["HuntingCase_T%d" % n] = {"Items": [merged[k] for k in sorted(merged)]}
    # Peso DA ZONA (04/10/2026): o peso do item na pool <U>_T<n> e o quanto dele o vanilla poe nessa zona
    # (nominal x slots da zona / slots elegiveis do item). Com o nominal cru, item generico (nasce em todo lugar)
    # abafava o exclusivo da zona: roupa de cacador era 11% da pool de caca; no vanilla da zona e 37%.
    # Pools fora de zona (Food, Construction, HuntingCase_T<n>) ficam com o nominal.
    zone_pool = re.compile(r"^(?P<u>[A-Za-z]+)_T(?P<n>\d)(?:_(?:Guns|LongGuns|Explosives))?$")
    for pname, pdata in pools.items():
        zm = zone_pool.match(pname)
        if not zm:
            continue
        cap_un = vanilla_tier_cap.get((zm.group("u"), int(zm.group("n"))), 0)
        if not cap_un:
            continue
        for it in pdata["Items"]:
            e = eligible_slots.get(it["Item"])
            if e:
                it["Weight"] = max(0.0001, round(types[it["Item"]]["nominal"] * cap_un / e, 4))
    gun_frac_cache = {}

    def vanilla_zone(u, n):
        """(armas, explosivos, itens) que o vanilla poe na zona (u, n)."""
        if (u, n) not in gun_frac_cache:
            guns = expl = total = 0.0
            for name, t in loot.items():
                if is_seasonal(name, t) or not eligible_slots.get(name) or not in_pool(u, n, t):
                    continue
                share = t["nominal"] * vanilla_tier_cap.get((u, n), 0) / eligible_slots[name]
                total += share
                if is_gun(name):
                    guns += share
                elif category_of(name) == "explosives":
                    expl += share
            gun_frac_cache[(u, n)] = (guns, expl, total)
        return gun_frac_cache[(u, n)]

    def gun_frac(u, n):
        guns, _, total = vanilla_zone(u, n)
        return guns / total if total else 0.0

    def expl_frac(u, n):
        _, expl, total = vanilla_zone(u, n)
        return expl / total if total else 0.0

    # Fator de itens por zona: (vanilla - itens das classes fixas) / itens das classes que escalam.
    item_scale = {}
    for u in served:
        for n in [0] + TIERS:
            fixed = scalable = 0.0
            for cls in served[u]:
                st = zone_storages.get((cls, n, u), 0.0)
                if cls in FIXED_SIZE:
                    fixed += st * avg(base_size[cls])
                else:
                    scalable += st * avg(base_size[cls])
            k = (vanilla_zone(u, n)[2] - fixed) / scalable if scalable else 1.0
            item_scale[(u, n)] = max(ITEM_SCALE_MIN, min(ITEM_SCALE_MAX, k))

    def sized(cls, n):
        """Min/Max calibrado da variante: fator medio das zonas que ela cobre, pesado pelos storages."""
        mn, mx = base_size[cls]
        if cls in FIXED_SIZE:
            return mn, mx
        parts = [(zone_storages.get((cls, n, u), 0.0), item_scale.get((u, n), 1.0)) for u in served if cls in served[u]]
        tot = sum(w for w, _ in parts)
        k = sum(w * f for w, f in parts) / tot if tot else 1.0
        new_mn = min(MAX_ITEMS, max(1, int(round(mn * k))))
        new_mx = min(MAX_ITEMS, max(new_mn, int(round(mx * k))))
        return new_mn, new_mx

    def kls_zone_items(u, n):
        """Itens que o KLS poe na zona (u, n) JA calibrados: storages de cada classe x media do Min/Max dela."""
        return sum(zone_storages.get((cls, n, u), 0.0) * avg(sized(cls, n)) for cls in served.get(u, []))

    def zone_boost(u, n):
        kls = kls_zone_items(u, n)
        if kls <= 0:
            return 1.0
        return max(1.0, min(a.gun_boost, vanilla_zone(u, n)[2] / kls))

    def gun_share(u, n):
        return min(GUN_SHARE_MAX, gun_frac(u, n) * zone_boost(u, n))

    def expl_share(u, n):
        return min(EXPL_SHARE_MAX, expl_frac(u, n) * zone_boost(u, n))

    # Pool Construction: material de construcao do loot, pelo nominal (garantia do bau de ferramentas).
    construction = [dict(pool_item(x, types), Category=category_of(x)) for x, t in sorted(loot.items())
                    if x.lower() in CONSTRUCTION_ITEMS and not is_seasonal(x, t)]
    if construction:
        pools["Construction"] = {"Items": construction}

    # Pool Food: toda comida do loot, de qualquer usage e tier, pelo nominal (garantia do suprimento militar).
    food = [dict(pool_item(x, types), Category="food") for x, t in sorted(loot.items())
            if category_of(x) == "food" and not is_seasonal(x, t)]
    if food:
        pools["Food"] = {"Items": food}

    def refs_for(cls, usages, n):
        mix = POOL_MIX.get(cls)
        refs = []
        for u in (list(mix) if mix else usages):
            cap_u = (mix[u] * 100) if mix else (usage_cap.get(u, 1) or 1)
            # Mistura fixa (caixote toxico): o multiplicador e o da zona DA CLASSE, nao o da pool emprestada
            # (a metade militar do caixote toxico usava o x3 do militar e somava ~14 armas na zona).
            boost = zone_boost(usages[0], n) if mix else zone_boost(u, n)
            gear, guns, expl = "%s_T%d" % (u, n), "%s_T%d_Guns" % (u, n), "%s_T%d_Explosives" % (u, n)
            g_share = min(GUN_SHARE_MAX, gun_frac(u, n) * boost) if guns in pools else 0.0
            e_share = min(EXPL_SHARE_MAX, expl_frac(u, n) * boost) if expl in pools else 0.0
            if gear in pools:
                refs.append({"Pool": gear, "Weight": round(cap_u * (1 - g_share - e_share), 1)})
            if guns in pools:
                refs.append({"Pool": guns, "Weight": round(cap_u * g_share, 1)})
            if expl in pools and e_share > 0:
                refs.append({"Pool": expl, "Weight": round(cap_u * e_share, 2)})
        return refs

    storages_json = {"Storages": []}
    for cls, usages, _, _, _ in STORAGES:
        mn, mx = sized(cls, 0)
        storages_json["Storages"].append({"Storage": cls, "Min": mn, "Max": mx, "Pools": refs_for(cls, usages, 1), "AmmoPerGun": AMMO_PER_GUN, "AmmoChance": AMMO_CHANCE})
        for n in TIERS:
            mn, mx = sized(cls, n)
            storages_json["Storages"].append({"Storage": "%s_T%d" % (cls, n), "Min": mn, "Max": mx, "Pools": refs_for(cls, usages, n), "AmmoPerGun": AMMO_PER_GUN, "AmmoChance": AMMO_CHANCE})

    def themed_def(name, mn, mx, mix, cats, guarantee, ammo, n):
        refs = [{"Pool": "%s_T%d" % (u, n), "Weight": w} for u, w in mix.items() if "%s_T%d" % (u, n) in pools]
        guaranteed = []
        if guarantee:
            g_usages, suffixes, gmin, gmax, g_cats = guarantee
            found = None
            if isinstance(g_usages, str) and g_usages.startswith("pool:"):
                exact = g_usages[5:] % n if "%d" in g_usages else g_usages[5:]
                found = exact if exact in pools else None
                suffixes = []
            for suffix in suffixes:
                for u in (g_usages or list(mix)):
                    pool = "%s_T%d%s" % (u, n, suffix)
                    if pool in pools and (not g_cats or any(i.get("Category") in g_cats for i in pools[pool]["Items"])):
                        found = pool
                        break
                if found:
                    break
            if found:
                guaranteed.append({"Pool": found, "Min": gmin, "Max": gmax, "Categories": g_cats or []})
        return {"Storage": name, "Min": mn, "Max": mx, "Pools": refs, "Categories": cats, "Guaranteed": guaranteed,
                "AmmoPerGun": ammo, "AmmoChance": 1.0 if ammo else 0.0}

    for cls, usages, _, _, _, mix, cats, guarantee, ammo, _ in THEMED:
        storages_json["Storages"].append(themed_def(cls, *sized(cls, 0), mix, cats, guarantee, ammo, 1))
        for n in TIERS:
            storages_json["Storages"].append(themed_def("%s_T%d" % (cls, n), *sized(cls, n), mix, cats, guarantee, ammo, n))

    # globals: dano padrao
    _, glob_root = read_xml(os.path.join(mission, "db", "globals.xml"))
    gv = {v.get("name"): v.get("value") for v in glob_root.findall("var")}
    settings = {"SearchTime": 3.0,
                "DefaultDamage": [float(gv.get("LootDamageMin", 0)), float(gv.get("LootDamageMax", 0.82))],
                "DebugLog": 0,
                "MaxGunsPerStorage": 2,
                "MaxSameItemPerStorage": 2,
                "ExportItems": 0}

    # --- escrita ---------------------------------------------------------------------------------------
    mdir = os.path.join(out, "mission")
    pdir = os.path.join(out, "profile", "Kryden", "KrydenLootSystem")
    os.makedirs(os.path.join(mdir, "kls"), exist_ok=True)
    os.makedirs(os.path.join(pdir, "Pools"), exist_ok=True)
    for old in glob.glob(os.path.join(pdir, "Pools", "*.json")):
        os.remove(old)
    with open(os.path.join(mdir, "kls", "kls_types.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    with open(os.path.join(mdir, "mapgroupproto.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write(proto_txt)
    with open(os.path.join(mdir, "kls", "kls_events.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write(events_xml)
    with open(os.path.join(mdir, "cfgeventgroups.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write(groups_xml)
    with open(os.path.join(mdir, "cfgeconomycore.snippet.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write('\t<ce folder="kls">\n\t\t<file name="kls_types.xml" type="types" />\n\t\t<file name="kls_events.xml" type="events" />\n\t</ce>\n')
    for name, data in pools.items():
        # Copia: pools diferentes dividem o mesmo dict (LongGuns, HuntingCase); escalar no lugar escalaria 2 vezes.
        items = [dict(i) for i in data["Items"]]
        integer_weights(items)
        write_pool(os.path.join(pdir, "Pools", name + ".json"), items)
    kits = build_kits(pools, types, spawn)
    write_kits(os.path.join(pdir, "Kits.json"), kits)
    with open(os.path.join(pdir, "Storages.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(storages_json, f, indent=2)
    with open(os.path.join(pdir, "Settings.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(settings, f, indent=4)

    with open(proto_path, encoding="utf-8", errors="replace") as f:
        proto_lower = f.read().lower()
    touched_events = sorted(g for g in skip if ('name="%s"' % g) in proto_lower)
    rep = ["# Template KrydenLootSystem - %s" % a.map, "",
           "Gerado em %s pelo `tools/template/build_template.py` (ferramenta interna, D-09)." % datetime.datetime.now().strftime("%d/%m/%Y %H:%M"), "",
           "| | |", "|---|---|",
           "| Missao de origem | `%s` |" % mission,
           "| mapgroupproto lido | `%s` |" % os.path.basename(proto_path),
           "| Versao do DayZ (RPT) | %s |" % dayz_version(),
           "| Teto por construcao (`--cap`) | %d |" % a.cap,
           "| Ocupacao alvo (`--fill`) | %.0f%% |" % (a.fill * 100),
           "| Reposicao (`--min-ratio` / `--restock`) | min = %.0f%% do nominal / %d s |" % (a.min_ratio * 100, a.restock),
           "| Slots de loot vanilla | %d |" % tot_vanilla,
           "| Slots para storage (com teto) | %d |" % tot_kls,
           "| **Storages no mapa (soma dos nominal)** | **%d** |" % total_storages,
           "| Groups com lootmax reduzido (predios) | %d |" % proto_stats["capped"],
           "| Groups de evento com teto `--event-cap` %d | %d |" % (a.event_cap, proto_stats["event_capped"]),
           "| Groups de evento com loot proprio (heli crash, Natal): intocados e sem storage | %d |" % proto_stats["deloot_event"],
           "| Eventos com teto no `kls_events.xml` | %s |" % ", ".join(changed_events),
           "| Wrecks com teto no `cfgeventgroups.xml` | %d |" % changed_children,
           "| Containers que ganharam a categoria `containers` | %d |" % proto_stats["category"],
           "| Wrecks deloot (comboio, blitz) com container so de storage `lootKLS` lootmax %d | %d |" % (a.event_cap, proto_stats["kls_container"]),
           "| Itens que ficam no chao (nao cabem em 10x10 ou sao objeto de base) | %d: %s |" % (len(floor_items), ", ".join(floor_items)),
           "| Itens que ficam no chao (usage sem storage: Historical, Lunapark, Underground...) | %d: %s |" % (len(unserved_items), ", ".join(unserved_items)),
           "| Fill por zona (resto %.2f; militar e toxica automaticos, FILL_TARGET) | %s |" % (a.fill, ", ".join("%s T%d %.2f" % (u, n, fill_used[(u, n)]) for u in FILL_TARGET for n in TIERS if usage_tier_cap.get((u, n)))),
           "| Arma (fracao vanilla da zona x multiplicador da zona, 1 a %.1f; teto %.0f%%) | %s |" % (a.gun_boost, GUN_SHARE_MAX * 100, ", ".join("%s T%d %.1f%% (x%.1f)" % (u, n, 100 * gun_share(u, n), zone_boost(u, n)) for u in ("Military", "Police", "Hunting", "Town", "Village", "ContaminatedArea") for n in TIERS if gun_frac(u, n))),
           "| Volume calibrado (itens KLS / vanilla) | %s |" % ", ".join("%s T%d %.0f/%.0f" % (u, n, kls_zone_items(u, n), vanilla_zone(u, n)[2]) for u in ("Military", "Town", "Village", "Industrial", "ContaminatedArea") for n in TIERS if vanilla_zone(u, n)[2] >= 1),
           "| Itens por storage (Min-Max calibrado, T1..T4) | %s |" % ", ".join("%s %s" % (c.replace("KLS_", ""), "/".join("%d-%d" % sized(c, n) for n in TIERS)) for c, _, _, _, _ in ALL_STORAGES),
           "| Explosivo (fracao vanilla da zona x multiplicador; teto %.0f%%) | %s |" % (EXPL_SHARE_MAX * 100, ", ".join("%s T%d %.1f%%" % (u, n, 100 * expl_share(u, n)) for u in ("Military", "Police", "Industrial", "Town", "ContaminatedArea") for n in TIERS if expl_frac(u, n))),
           "| Carregador/municao da propria arma (`AmmoPerGun`/`AmmoChance`) | %d com %.0f%% de chance (maleta: 100%%) |" % (AMMO_PER_GUN, AMMO_CHANCE * 100),
           "| Sazonais fora das pools (continuam fora do chao) | %s |" % ", ".join(sorted(n for n, t in loot.items() if is_seasonal(n, t))),
           "| Itens no override (nominal=0) | %d |" % len(loot),
           "| Pools geradas | %d |" % len(pools),
           "| Kits (`Kits.json`) | %d |" % len(kits), "",
           "## Slots por usage (com teto)", "", "| Usage | Slots | Storages (fill) | Classes |", "|---|---|---|---|"]
    for u in sorted(usage_cap, key=lambda x: -usage_cap[x]):
        zone_storages = sum(fill_used.get((u, n), a.fill) * usage_tier_cap.get((u, n), 0) for n in [0] + TIERS)
        rep.append("| %s | %d | %d | %s |" % (u, usage_cap[u], round(zone_storages) if u in served else 0, ", ".join(served.get(u, ["(nenhuma: fica sem storage)"]))))
    rep += ["", "## Slots por tier (posicao do predio no areaflags.map)", "", "| Tier | Slots |", "|---|---|"]
    for n in [0] + TIERS:
        rep.append("| %s | %d |" % ("sem tier" if n == 0 else "T%d" % n, sum(v for (u, t), v in usage_tier_cap.items() if t == n)))
    rep += ["", "## Nominal por variante", "", "| Variante | Nominal |", "|---|---|"]
    rep += ["| %s | %d |" % r for r in class_rows]
    rep += ["", "## Instalar (servidor de teste)", "",
            "1. Missao: copiar `mission/mapgroupproto.xml` e `mission/cfgeventgroups.xml` (backup dos originais como `<arquivo>.bak-kls-<data>`) e a pasta `mission/kls/`.",
            "2. `cfgeconomycore.xml`: colar o bloco de `mission/cfgeconomycore.snippet.xml` antes de `</economycore>`.",
            "3. Profile: copiar `profile/Kryden/KrydenLootSystem/` para `<profiles>/Kryden/KrydenLootSystem/`.",
            "4. Update da Steam sobrescreve a missao: reaplicar o template depois de cada update.", ""]
    with open(os.path.join(out, "REPORT.md"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(rep))

    print("template:", out)
    print("storages no mapa:", total_storages, "| slots com teto:", tot_kls, "| slots vanilla:", tot_vanilla)
    print("override:", len(loot), "| pools:", len(pools), "| proto:", dict(proto_stats), "| itens no chao:", len(floor_items))
    print("eventos com teto:", changed_events, "| wrecks no cfgeventgroups:", changed_children)


if __name__ == "__main__":
    main()
