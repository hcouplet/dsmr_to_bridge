# ESPHome DSMR TCP -> SDM630 Modbus bridge

Ce composant lit un flux DSMR/P1 sur TCP (par ex. `192.168.10.20:82`) et expose les valeurs principales sous forme de getters utilisables dans un serveur Modbus RTU ESPHome pour émuler un Eastron SDM630 lu par un Solis.

## Arborescence GitHub recommandée

```text
repo/
├── components/
│   └── dsmr_tcp_bridge/
│       ├── __init__.py
│       ├── dsmr_tcp_bridge.cpp
│       └── dsmr_tcp_bridge.h
├── examples/
│   └── solis_sdm630_bridge.yaml
└── README.md
```

## Utilisation depuis le même dépôt

```yaml
external_components:
  - source:
      type: local
      path: ./components
    components: [dsmr_tcp_bridge]
```

## Utilisation depuis GitHub

```yaml
external_components:
  - source: github://VOTRE_USER/VOTRE_REPO@main
    components: [dsmr_tcp_bridge]
```

Puis dans le YAML :

```yaml
dsmr_tcp_bridge:
  id: p1bridge
  host: 192.168.10.20
  port: 82
  reconnect_interval: 3s
```

Ensuite, dans `modbus_controller.server_registers`, tu peux appeler :

```yaml
read_lambda: return id(p1bridge).get_p_total_w();
```

## Registres SDM630 couverts

- 0,2,4 : tensions L1/L2/L3
- 6,8,10 : courants L1/L2/L3
- 12,14,16 : puissance active L1/L2/L3 (W)
- 18,20,22 : puissance apparente L1/L2/L3 (VA)
- 24,26,28 : puissance réactive L1/L2/L3 (var, estimée)
- 30,32,34 : facteur de puissance L1/L2/L3 (estimé)
- 52 : puissance active totale (W)
- 70 : fréquence (fixée à 50 Hz)
- 72 : énergie importée totale (kWh)
- 74 : énergie exportée totale (kWh)

## Remarques

- La fréquence est fixée à 50 Hz.
- La puissance réactive et le facteur de puissance sont estimés à partir de P, U et I.
- Les registres 54/56/58 sont renvoyés à 0 dans l'exemple pour éviter des réponses vides si le Solis les lit.
- Le composant n'exporte pas d'entités Home Assistant ; il sert de source de données interne pour `read_lambda`.
