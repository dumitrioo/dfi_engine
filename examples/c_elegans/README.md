# C. elegans - models of the nervous system in DFI language

## Full connectome

| file | what is this |
| --- | --- |
| `elegans_connectome.dfi` | Network: **302 neurons** (282 somatic, incl. CANL/R without synapses, + 20 pharyngeal) **+ 148 targets** (95 dBWM body muscles, 22 pm pharyngeal muscles, 31 other: hypodermis, vm/um, anal, mc, intestine). Total ~1455 instances. |
| `elegans_std_full.dfi` | The standard library for this connect: 49 descriptors with arities inferred from the data (dsum1-16, glin1-16, neuron/sensorn/muscle variants) + verified simtime/random_touch/first_defined. |

Data: Cook et al. 2019 (OpenWorm `herm_full_edgelist.csv`), all 4681 chemical
edges and 2698 gap junction directions, including 48 autosynapses from the data
and the somatic↔pharyngeal RIP bridge.

Conventions: chemistry weight = count/40 (bilateral FLP→AVA ≈ 1.4 - operating point
of the validated nose-touch circuit), gap = count/100, sign -1 for GABAergic
(DD, VD, RME, AVL, DVB, RIS - 26 neurons), spike threshold rel = 1.0, k = 5,
tau = 0.1 s, bias = -0.1, noise 0.1 (xorshift, seed unique per instance).

Architecture: fan-in via dendritic aggregators `dsumN` (weighted
graded spike, memoized per tick) and `glinN` (linear Σ g p);
The gap term `g·(p−v)` is decomposed into `glin(Σ g_j·p_j) − G·v` with the cell literal G.
Hubs: AVAL - 63 chemical inputs (4 aggregators), AVAR - 48 gap partners.

Run: with host - publish `'touch'` (0..1), read `'AVAL'…'AVER'`;
without host - standalone random_touch + monitors (4 prints/sec).

**Verified by mirror simulation** (same equations, 120 sec, 24,000 ticks):
stable (max|v| = 3.1); quiet dormancy (0% active, AVA -0.07 / AVB -0.08);
Nasal touch - dose-response as for the 15-node circuit: strength < 0.3 - no response, 0.5–0.9 - reversal (AVA up to 1.8 with suppressed AVB), 8/14 touches - reversal, return to rest ~2 s. VC → vm (vulva), DVB → int (enterica), etc.
included as in the data.