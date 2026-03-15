# ESPHome DSMR TCP -> SDM630 Modbus bridge

Ce composant lit un flux DSMR/P1 sur TCP (par ex. `192.168.10.20:82`) et expose les valeurs principales via des getters utilisables dans un serveur Modbus RTU ESPHome pour émuler un Eastron SDM630 lu par un onduleur Solis.

## Nouveautés de cette version

- gestion du **stale timeout** quand le flux DSMR TCP se coupe
- stratégie configurable : `hold_last`, `nan`, `zero`
- méthodes d'état :
  - `is_tcp_connected()`
  - `is_data_fresh()`
  - `get_age_s()`
- exemple mis à jour pour **ESP32 Mini D1 + module RS485 auto-direction**

## Recommandation Solis

Pour un Solis en compteur externe, la stratégie recommandée est :

```yaml
dsmr_tcp_bridge:
  stale_timeout: 10s
  stale_strategy: hold_last
```

Cela évite de renvoyer `0 W` si le Wi‑Fi ou le TCP tombe, ce qui peut sinon pousser l'onduleur à injecter trop.

## Arborescence GitHub recommandée

```text
repo/
├── components/
│   └── dsmr_tcp_bridge/
│       ├── __init__.py
│       ├── dsmr_tcp_bridge.cpp
│       └── dsmr_tcp_bridge.h
├── examples/
│   └── solis_sdm630_bridge_mini_d1.yaml
└── README.md
```

## Utilisation depuis GitHub

```yaml
external_components:
  - source: github://hcouplet/dsmr_to_bridge@main
    components: [dsmr_tcp_bridge]
```

Puis dans le YAML :

```yaml
dsmr_tcp_bridge:
  id: p1bridge
  host: 192.168.10.20
  port: 82
  reconnect_interval: 3s
  stale_timeout: 10s
  stale_strategy: hold_last
```

## Getters disponibles

```yaml
read_lambda: return id(p1bridge).get_p_total_w();
```

Autres méthodes utiles :

```cpp
id(p1bridge).is_tcp_connected();
id(p1bridge).is_data_fresh();
id(p1bridge).get_age_s();
```

## Registres SDM630 couverts

- `0,2,4` : tensions L1/L2/L3
- `6,8,10` : courants L1/L2/L3
- `12,14,16` : puissance active L1/L2/L3 (W)
- `18,20,22` : puissance apparente L1/L2/L3 (VA)
- `24,26,28` : puissance réactive L1/L2/L3 (var, estimée)
- `30,32,34` : facteur de puissance L1/L2/L3 (estimé)
- `52` : puissance active totale (W)
- `70` : fréquence (fixée à 50 Hz)
- `72` : énergie importée totale (kWh)
- `74` : énergie exportée totale (kWh)

## Câblage Mini D1 ESP32

### ESP32 -> module RS485 TTL

- `GPIO17` -> `TXD`
- `GPIO16` -> `RXD`
- `3.3V` -> `VCC`
- `GND` -> `GND`

### RS485 -> Solis

- `A+` -> `A`
- `B-` -> `B`

Si aucune communication ne passe, inverser `A/B`.

## Remarques

- La fréquence est fixée à `50.0 Hz`.
- La puissance réactive et le facteur de puissance sont estimés à partir de `P`, `U` et `I`.
- Les registres `54/56/58` sont laissés à `0` dans l'exemple.
- Le composant n'exporte pas d'entités Home Assistant nativement ; les capteurs d'état dans l'exemple sont des `template` qui appellent les méthodes du composant.
