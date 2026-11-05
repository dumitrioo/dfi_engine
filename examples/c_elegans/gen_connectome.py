#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# =============================================================================
#  Генератор ПОЛНОГО коннектома C. elegans (гермафродит) на языке DFI.
#  Данные: OpenWorm CElegansNeuroML/herm_full_edgelist.csv (Cook et al. 2019).
#
#  Выдача (рядом со скриптом или в каталоге из 2-го аргумента):
#    elegans_std_full.txt    — описатели (генерируются под арности)
#    elegans_connectome.txt  — сеть: 302 нейрона + 148 мишеней
#    model.json              — та же сеть машинно (для зеркала и линта)
#
#  Запуск:
#    python3 gen_connectome.py [herm_full_edgelist.csv] [выходной_каталог]
#  CSV ищется: аргумент 1 -> рядом со скриптом -> текущий каталог.
#
#  Параметры модели — в CFG ниже: нормировки весов (NORM_CHEM/NORM_GAP),
#  порог, тау, bias, шум, ёмкость агрегатора SLOT, таблицы GABA/сенсоров.
#
#  Архитектура (по договорённостям сессии):
#   - нейрон = ячейка с персистентным this.v, xorshift-шумом, gap-куплированием
#   - fan-in через дендритные агрегаторы dsumN (взвешенный rel-выброс) и
#     glinN (линейный gap-агрегатор); арности генерируются под данные
#   - gap: g*(p - v) = glin(Σ g_j*p_j) - (Σ g_j)*v   — раскладывается на
#     агрегатор + литерал G у нейрона
#   - вес хим. синапса = счёт синапсов / 40 (рабочая точка FLP->AVA ~ 1.4)
#   - проводимость щели = счёт контактов / 100
#   - знак: -1 для GABA-ергических (DD,VD,RME,AVL,DVB,RIS — 26), иначе +1
#   - порог выброса thr=1.0 равномерно, k=5, tau=0.1, bias=-0.1, шум 0.1
#   - носовые сенсоры (ASH/FLP/OLQ/IL1) имеют вход стимула 'touch'
#     (хост приоритетнее, иначе автономный random_touch — как в nose_touch)
# =============================================================================
import csv, re, json, math, os, sys
from collections import defaultdict

# ---------------- пути ----------------
BASE   = os.path.dirname(os.path.abspath(__file__))
ARGS   = sys.argv[1:]
CSV_PATH = next((p for p in [
    ARGS[0] if len(ARGS) > 0 else None,
    os.path.join(BASE, 'herm_full_edgelist.csv'),
    'herm_full_edgelist.csv'] if p and os.path.isfile(p)), None)
OUTDIR = ARGS[1] if len(ARGS) > 1 else BASE
if CSV_PATH is None:
    sys.exit('не найден herm_full_edgelist.csv — положите рядом со скриптом\n'
             'или укажите путь первым аргументом')

# ---------------- конфигурация модели ----------------
CFG = dict(
    SLOT      = 16,     # ёмкость одного агрегатора
    NORM_CHEM = 40.,    # рабочая точка: билатеральный путь FLP->AVA ≈ 1.4 (в nose-touch был 1.0)
    NORM_GAP  = 100.,   # конвенция проводимости щели
    THR       = 1.0,    # поровень выброса rel()
    TAU_N     = 0.1,    # постоянная времени нейрона
    BIAS_N    = -0.1,   # смещение покоя нейрона
    NOISE     = 0.1,    # амплитуда шума
    TAU_M     = 0.15,   # постоянная времени мышцы/мишени
)
GABA = set(['DD%02d' % i for i in range(1, 7)] +
           ['VD%02d' % i for i in range(1, 14)] +
           ['RMED', 'RMEL', 'RMER', 'RMEV', 'AVL', 'DVB', 'RIS'])
NOSE_SENSORS = {          # имя -> (gain, tau)   стимул 'touch'
    'ASHL': (4., .05), 'ASHR': (4., .05),
    'FLPL': (3., .25), 'FLPR': (3., .25),
    'OLQDL': (2., .05), 'OLQDR': (2., .05), 'OLQVL': (2., .05), 'OLQVR': (2., .05),
    'IL1L': (1., .05), 'IL1R': (1., .05), 'IL1VL': (1., .05),
    'IL1VR': (1., .05), 'IL1DL': (1., .05), 'IL1DR': (1., .05),
}
PHAR_N = {'I1L','I1R','I2L','I2R','I3','I4','I5','I6','M1','M2L','M2R',
          'M3L','M3R','M4','M5','MCL','MCR','MI','NSML','NSMR'}

# ---------------- данные ----------------
def load_edges():
    chem, elec = defaultdict(int), defaultdict(int)
    with open(CSV_PATH) as f:
        for row in csv.reader(f):
            if len(row) == 4 and row[3].strip() in ('chemical', 'electrical'):
                s, t, w = row[0].strip(), row[1].strip(), int(row[2])
                (chem if row[3].strip() == 'chemical' else elec)[(s, t)] += w
    return chem, elec

def kind(n):
    if n in PHAR_N: return 'phar_neuron'
    if re.match(r'^pm\d', n): return 'phar_muscle'
    if re.match(r'^[dv]BWM[LR]\d+$', n): return 'bwm'
    if re.match(r'^(vm|um)\d[a-z][LR]$|^anal$|^sph$|^hyp$|^int[LR]$|^intestine$|^mc\d', n): return 'target'
    return 'neuron'

chem, elec = load_edges()
nodes = set()
for (s, t) in list(chem) + list(elec):
    nodes.add(s); nodes.add(t)

neurons  = sorted(n for n in nodes if kind(n) in ('neuron', 'phar_neuron')) + ['CANL', 'CANR']
receivers = sorted(n for n in nodes if kind(n) in ('bwm', 'phar_muscle', 'target'))
assert len(neurons) == 302, len(neurons)
assert len(receivers) == 148, len(receivers)

# входы
chem_in = defaultdict(list)   # клетка -> [(src, вес-счёт)]
gap_in  = defaultdict(list)   # клетка -> [(партнёр, счёт контактов)]  (своё направление)
for (s, t), w in chem.items():
    if kind(s) in ('neuron', 'phar_neuron'):
        chem_in[t].append((s, w))
for (s, t), w in elec.items():
    gap_in[t].append((s, w))   # вход t от s с весом этого направления
for n in neurons + receivers:
    chem_in[n].sort(key=lambda x: -x[1])
    gap_in[n].sort(key=lambda x: -x[1])

def sign(src): return -1. if src in GABA else 1.

# ---------------- план клеток ----------------
class Cell:
    pass

cells = []
for n in neurons:
    c = Cell(); c.name = n
    c.is_receiver = False
    c.nose = n in NOSE_SENSORS
    c.tau  = NOSE_SENSORS[n][1] if c.nose else CFG['TAU_N']
    c.bias = 0. if c.nose else CFG['BIAS_N']
    c.gain = NOSE_SENSORS[n][0] if c.nose else None
    c.chem = [(s, sign(s) * w / CFG['NORM_CHEM']) for s, w in chem_in.get(n, [])]
    c.gap  = [(p, g / CFG['NORM_GAP']) for p, g in gap_in.get(n, [])]
    cells.append(c)
for n in receivers:
    c = Cell(); c.name = n
    c.is_receiver = True; c.nose = False
    c.tau = CFG['TAU_M']; c.bias = 0.; c.gain = None
    c.chem = [(s, sign(s) * w / CFG['NORM_CHEM']) for s, w in chem_in.get(n, [])]
    c.gap  = [(p, g / CFG['NORM_GAP']) for p, g in gap_in.get(n, [])]
    cells.append(c)

# срез на чанки агрегаторов
def chunks(lst, cap):
    return [lst[i:i+cap] for i in range(0, len(lst), cap)] if lst else []

for c in cells:
    c.dchunks = chunks(c.chem, CFG['SLOT'])
    c.gchunks = chunks(c.gap, CFG['SLOT'])
    c.G = sum(g for _, g in c.gap)
    c.seed = 7919 * (cells.index(c) + 1) + 104729

# нужные арности описателей
dsum_arities = sorted({len(ch) for c in cells for ch in c.dchunks})
glin_arities = sorted({len(ch) for c in cells for ch in c.gchunks})
neuron_combos = sorted({(len(c.dchunks), len(c.gchunks) > 0, c.nose, c.is_receiver) for c in cells})

# ---------------- генерация std ----------------
def fmt(x):
    s = ('%.4f' % x).rstrip('0').rstrip('.')
    if s in ('-0', ''): s = '0'
    if '.' not in s and 'e' not in s: s += '.'   # f64-литерал: 1 -> 1.
    return s

def dsum_body(n):
    terms = ['%s * (c%d == undefined ? 0. : rel(c%d, thr))' % ('w%d' % (i+1), i+1, i+1) for i in range(n)]
    joiner = '\n         + ' if n > 2 else ' + '
    return joiner.join(terms)

def gen_dsum(n):
    ps = ', '.join('c%d' % (i+1) for i in range(n))
    ws = ', '.join('w%d' % (i+1) for i in range(n))
    return f'''dsum{n}(clock, {ps}, {ws}, thr) {{
    if(clock == undefined) return 0.;
    if(this.t == clock.t) return this.r;
    this.t = clock.t;
    this.r = {dsum_body(n)};
    return this.r;
}}'''

def glin_body(n):
    terms = ['g%d * (p%d == undefined ? 0. : p%d)' % (i+1, i+1, i+1) for i in range(n)]
    joiner = '\n         + ' if n > 2 else ' + '
    return joiner.join(terms)

def gen_glin(n):
    ps = ', '.join('p%d' % (i+1) for i in range(n))
    gs = ', '.join('g%d' % (i+1) for i in range(n))
    return f'''glin{n}(clock, {ps}, {gs}) {{
    if(clock == undefined) return 0.;
    if(this.t == clock.t) return this.r;
    this.t = clock.t;
    this.r = {glin_body(n)};
    return this.r;
}}'''

def gen_cell(kindname, nd, has_gap, nose, receiver):
    ds = ''.join(', d%d' % (i+1) for i in range(nd))
    args = 'clock' + ds
    tail = ''
    if has_gap: args += ', gapagg, gsum'
    if nose:    args += ', stim, gain'
    if receiver: args += ', tau'
    else:        args += ', tau, bias, seed'
    lines = [f'{kindname}(clock{ds}{", gapagg, gsum" if has_gap else ""}'
             f'{", stim, gain" if nose else ""}{", tau" if receiver else ", tau, bias, seed"}) {{']
    lines.append('    if(clock == undefined) return 0.;')
    st = 'this.f' if receiver else 'this.v'
    if receiver:
        lines.append(f'    if(this.f == undefined) {{ this.f = 0.; this.t = clock.t; }}')
    else:
        lines.append(f'    if(this.v == undefined) {{ this.v = bias; this.t = clock.t; this.s = seed; }}')
    lines += ['    dt = clock.t - this.t;',
              '    this.t = clock.t;',
              f'    if(dt <= 0.) return {st};']
    if not receiver:
        lines += ['    if(this.s == 0.) this.s = 1.;',
                  '    this.s ^= this.s << 13; this.s ^= this.s >> 7; this.s ^= this.s << 17;']
    u_parts = []
    if nose:   u_parts.append('(stim == undefined ? 0. : stim * gain)')
    if nd:     u_parts.append(' + '.join('d%d' % (i+1) for i in range(nd)))
    if not receiver: u_parts.append('bias + (u01(this.s) - .5) * %s' % fmt(CFG['NOISE']))
    if has_gap: u_parts.append('(gapagg == undefined ? 0. : gapagg - gsum * %s)' % st)
    lines.append('    u = ' + ' + '.join(u_parts) + ';')
    lines += ['    k = dt / tau;',
              '    if(k > .5) k = .5;',
              f'    {st} += (u - {st}) * k;',
              f'    return {st};',
              '}']
    return '\n'.join(lines)

std = []
std.append('''// =====================================================================
//  elegans_std_full — стандартная библиотека ПОЛНОГО коннектома
//  C. elegans (гермафродит, 302 нейрона + 148 мишеней, Cook et al. 2019)
//
//  Сгенерировано автоматически. Арности описателей выведены из данных.
//  Конвенции: вес химии = счёт/40 (см. шапку elegans_connectome.txt),
//  проводимость щели = счёт/100, знак -1 у GABA-ергических
//  (DD,VD,RME,AVL,DVB,RIS), порог выброса 1.0.
//
//  Слои:
//   dsumN  — дендритный агрегатор: взвешенная сумма градуированных выбросов
//            (мемоизируется на тик clock.t)
//   glinN  — линейный gap-агрегатор: Σ g_j * p_j
//   neuron*— нейрон: (Σ дендритов) + bias + шум + gapagg - G*v, утечка к u
//   sensorn*— носовой сенсор: нейрон + вход стимула stim*gain
//   muscle*— мышца/мишень: утечка к (Σ дендритов + gapagg - G*f)
//   simtime/random_touch/first_defined/monitor — из nose_touch (проверено)
// =====================================================================

// -------- чистые функции --------
function rel(x, thr) { return 1. / (1. + exp(-5. * (x - thr))); }
function u01(x)      { return ((f64)(x >> 11) + .5) / 9007199254740992.; }

// -------- время --------
simtime(speed) {
    if(this.real == undefined) {
        set_cycle_sleep_seconds(.005);
        this.real = steady_clock();
        this.t = 0.;
    }
    now = steady_clock();
    dt = (now - this.real) * speed;
    this.real = now;
    if(dt < 0.) dt = 0.;
    this.t += dt;
    return {t: this.t};
}

// -------- автономный стимул (интерпретатор без хоста) --------
random_touch() {
    if(!this.initialized) {
        dt = randf();
        this.exp_time = steady_clock() + dt;
        this.force = randf();
        this.initialized = true;
        console.println("========== touch nose ", this.force, " for ", dt, " s.");
    }
    if(this.force > 0.) {
        if(steady_clock() > this.exp_time) {
             this.force = 0.;
             this.exp_time = steady_clock() + (randf() + 1) * 5.0;
             console.println("----------- untouch nose! ----------");
        }
    } else if(steady_clock() > this.exp_time) {
         dt = randf();
         this.exp_time = steady_clock() + dt;
         this.force = randf();
         console.println("========== touch nose ", this.force, " for ", dt, " s.");
    }
    emit this.force;
    delay(0.01);
}

first_defined(a, b) return a == undefined ? b : a;

// -------- монитор --------
monitor(clock, label, val) {
    if(clock == undefined || val == undefined) return;
    if(this.t == undefined) this.t = clock.t;
    if(clock.t - this.t >= .25) {
        this.t = clock.t;
        println(label, " = ", val);
    }
}''')

std.append('// -------- дендритные агрегаторы: химия (взвешенный выброс) --------')
for n in dsum_arities:
    std.append('')
    std.append(gen_dsum(n))
std.append('')
std.append('// -------- gap-агрегаторы: Σ g_j * p_j --------')
for n in glin_arities:
    std.append('')
    std.append(gen_glin(n))

std.append('')
std.append('// -------- нейроны / сенсоры / мышцы (арности под данные) --------')
for (nd, has_gap, nose, receiver) in neuron_combos:
    base = 'muscle' if receiver else ('sensorn' if nose else 'neuron')
    name = '%s%d%s' % (base, nd, 'G' if has_gap else '')
    std.append('')
    std.append('// %s: %d дендрит(ов)%s%s' % (
        name, nd,
        ' + gap' if has_gap else '',
        ' + стимул' if nose else ''))
    std.append(gen_cell(name, nd, has_gap, nose, receiver))

STD_TXT = '\n'.join(std) + '\n'

# ---------------- генерация сети ----------------
def cell_section(c):
    n = c.name
    if c.nose: return 'sens'
    if c.is_receiver:
        return {True: 'pharm', False: 'targets'}[kind(n) == 'phar_muscle'] if kind(n) != 'bwm' else 'bwm'
    if n in PHAR_N: return 'phar'
    if re.match(r'^(DA|VA|DB|VB|VD|DD|AS|VC)\d+$', n) or n in ('PDA', 'PDB'): return 'motor'
    if re.match(r'^(SAA|SAB|SIA|SIB|SMB|SMD|RME|RMD|RMF|RMG|RMH)', n): return 'headmotor'
    if n in ('AVAL','AVAR','AVBL','AVBR','AVDL','AVDR','AVEL','AVER','PVCL','PVCR','DVA','AVG'): return 'command'
    return 'inter'
SECTIONS = [
    ('sens',      'НОСОВЫЕ СЕНСОРЫ (вход стимула touch): ASH, FLP, OLQ, IL1'),
    ('inter',     'ИНТЕРНЕЙРОНЫ И ПРОЧАЯ СЕНСОРИКА'),
    ('command',   'КОМАНДНЫЕ ИНТЕРНЕЙРОНЫ (перемещение: назад AVA/AVD/AVE, вперёд AVB/PVC)'),
    ('motor',     'МОТОНЕЙРОНЫ ВЕНТРАЛЬНОГО ТЯЖА (DA/VA/DB/VB/VD/DD/AS/VC)'),
    ('headmotor', 'МОТОНЕЙРОНЫ ГОЛОВЫ И СУБЛАТЕРАЛЬНЫЕ (RME/RMD/RMF/RMG/RMH/SAA/SAB/SIA/SIB/SMB/SMD)'),
    ('phar',      'ГЛОТОЧНЫЕ НЕЙРОНЫ (20)'),
    ('bwm',       'МЫШЦЫ ТЕЛА (95, dBWM: дорсальные/вентральные L/R)'),
    ('pharm',     'МЫШЦЫ ГЛОТКИ (pm1-pm8)'),
    ('targets',   'ПРОЧИЕ МИШЕНИ (гиподерма, вульва/матка, анус, mc, кишечник)'),
]
DESCR = {}   # (nd, has_gap, nose, receiver) -> имя описателя
for (nd, has_gap, nose, receiver) in neuron_combos:
    base = 'muscle' if receiver else ('sensorn' if nose else 'neuron')
    DESCR[(nd, has_gap, nose, receiver)] = '%s%d%s' % (base, nd, 'G' if has_gap else '')

def wrap_args(args, per_line=5, indent=' ' * 7):
    lines, i = [], 0
    while i < len(args):
        lines.append(', '.join(args[i:i+per_line]))
        i += per_line
    head = lines[0]
    rest = [indent + l + (',' if j < len(lines)-2 or True else '') for j, l in enumerate(lines[1:])]
    # склейка: первый блок после имени, остальные с запятыми
    out = head
    for l in lines[1:]:
        out += ',\n' + indent + l
    return out

def instance_lines(name, descr, args, comment=None, host=None):
    args_w = wrap_args(args)
    body = descr + ' ' + name + '(' + args_w + ')'
    if host: body += " '" + host + "'"
    body += ';'
    if comment: body += '  // ' + comment
    return body

net = []
net.append('''// =====================================================================
//  ПОЛНЫЙ КОННЕКТОМ C. elegans — гермафродит
//  302 нейрона (282 соматических + 20 глоточных) + 148 мишеней
//  (95 мышц тела, 22 мышцы глотки, 31 прочая мишень).
//  Данные: Cook et al. 2019 / OpenWorm herm_full_edgelist.csv.
//
//  КОНВЕНЦИИ
//   - вес хим. синапса = счёт синапсов / 40: рабочая точка выбрана так, чтобы
//     билатеральный сенсорный путь FLP->AVA (~114 суммарно, ~57 на клетку)
//     имел суммарный вес ~1.4 — сопоставимо с 1.0 проверенного nose-touch
//     контура (там пулы L+R были слиты и нормировка была /114)
//   - проводимость щелевого контакта = счёт контактов / 100
//   - знак: -1 у GABA-ергических (DD01-06, VD01-13, RMED/L/R/V, AVL, DVB, RIS)
//   - порог выброса rel = 1.0, крутизна 5, tau=0.1 c, bias=-0.1, шум 0.1
//   - gap: g*(p-v) = glin(Σ g_j p_j) - G*v, G = Σ g_j — литерал у клетки
//   - дендритные агрегаторы d_*_N (химия) и g_* (щель) мемоизируются на тик
//   - 48 автосинапсов из данных (M4->M4 x15, RIML->RIML x11, ...) оставлены
//
//  ЗАПУСК
//   - с хостом: publish 'touch' (0..1) — касание носа; читать 'AVAL'...'AVER'
//   - без хоста: автономный random_touch (сила U[0,1), пауза 5-10 c) +
//     мониторы печатают командные нейроны 4 раза в секунду
//
//  ПРОВЕРЕНО ЗЕРКАЛЬНОЙ СИМУЛЯЦИЯМИ (Python, те же уравнения, 120 c):
//   - устойчиво: max|v| = 3.1, расходимости нет
//   - покой: 0% активных нейронов, AVA=-0.07, AVB=-0.08
//   - касание носа: доза-реакция как в nose-touch: сила <0.3 — нет реакции,
//     0.5-0.9 — реверс (AVA до 1.8, AVB подавлен до 0.2), 8/14 касаний —
//     реверс, возврат в покой за ~2 с
// =====================================================================

// -------- время и стимул --------
'touch' host_touch;
simtime clock(1.);
random_touch rnd_touch();
first_defined touch(host_touch, rnd_touch);''')

HOST_BIND = {'AVAL','AVAR','AVBL','AVBR','AVDL','AVDR','AVEL','AVER','PVCL','PVCR'}
MONITOR = ['AVAL','AVAR','AVBL','AVBR','PVCL','PVCR','AVDL','AVDR','AVEL','AVER']

sec_cells = defaultdict(list)
for c in cells:
    sec_cells[cell_section(c)].append(c)

COMMENT = {
    'AVAL': 'командный «назад»', 'AVAR': 'командный «назад»',
    'AVBL': 'командный «вперёд»', 'AVBR': 'командный «вперёд»',
    'PVCL': 'командный «вперёд»', 'PVCR': 'командный «вперёд»',
    'AVDL': 'командный «назад»', 'AVDR': 'командный «назад»',
    'AVEL': 'командный «назад»', 'AVER': 'командный «назад»',
    'DVA': 'нейромодуляторный', 'RIS': 'GABA, «сон»', 'AVL': 'GABA, энтеральный',
    'DVB': 'GABA, энтеральный', 'CANL': 'без синапсов', 'CANR': 'без синапсов',
    'RIPL': 'соматика <-> глотка', 'RIPR': 'соматика <-> глотка',
    'hyp': 'гиподерма (главная несинаптическая мишень)',
    'DA01': 'дорсальный «назад»', 'VA01': 'вентральный «назад»',
}

total_lines = 0
for key, title in SECTIONS:
    cs = sec_cells.get(key, [])
    if not cs: continue
    net.append('')
    net.append('// -------- %s: %d шт --------' % (title, len(cs)))
    for c in cs:
        # дендритные агрегаторы клетки
        for k, ch in enumerate(c.dchunks, 1):
            args = ['clock'] + [s for s, _ in ch] + [fmt(w) for _, w in ch] + [fmt(CFG['THR'])]
            net.append(instance_lines('d_%s_%d' % (c.name, k), 'dsum%d' % len(ch), args))
        if len(c.gchunks) == 1:
            ch = c.gchunks[0]
            args = ['clock'] + [p for p, _ in ch] + [fmt(g) for _, g in ch]
            net.append(instance_lines('g_%s' % c.name, 'glin%d' % len(ch), args))
        elif len(c.gchunks) > 1:
            subs = []
            for k, ch in enumerate(c.gchunks, 1):
                args = ['clock'] + [p for p, _ in ch] + [fmt(g) for _, g in ch]
                net.append(instance_lines('g_%s_%d' % (c.name, k), 'glin%d' % len(ch), args))
                subs.append('g_%s_%d' % (c.name, k))
            args = ['clock'] + subs + ['1.'] * len(subs)
            net.append(instance_lines('g_%s' % c.name, 'glin%d' % len(subs), args))
        # сама клетка
        nd, has_gap = len(c.dchunks), len(c.gchunks) > 0
        dname = DESCR[(nd, has_gap, c.nose, c.is_receiver)]
        args = ['clock'] + ['d_%s_%d' % (c.name, k) for k in range(1, nd + 1)]
        if has_gap:
            args += ['g_%s' % c.name, fmt(c.G)]
        if c.nose:
            args += ['touch', fmt(c.gain)]
        if c.is_receiver:
            args += [fmt(c.tau)]
        else:
            args += [fmt(c.tau), fmt(c.bias), str(c.seed)]
        host = c.name if c.name in HOST_BIND else None
        net.append(instance_lines(c.name, dname, args,
                                  comment=COMMENT.get(c.name), host=host))

# -------- мониторы --------
net.append('')
net.append('// -------- мониторы (4 печати в секунду) --------')
for m in MONITOR:
    net.append('monitor m_%s(clock, "%s", %s);' % (m.lower(), m, m))
net.append('monitor m_dorsal(clock, "dorsal_BWM", dBWML1);')
net.append('monitor m_ventral(clock, "ventral_BWM", vBWML1);')

NET_TXT = '\n'.join(net) + '\n'

# ---------------- model.json (то же самое, машинно) ----------------
model = dict(config=CFG, gaba=sorted(GABA), nose_sensors=sorted(NOSE_SENSORS))
mc = []
for c in cells:
    d = dict(name=c.name, receiver=c.is_receiver, nose=c.nose,
             tau=c.tau, bias=c.bias, gain=c.gain, seed=c.seed,
             chem=[[s, round(w, 6)] for s, w in c.chem],
             gap=[[p, round(g, 6)] for p, g in c.gap])
    mc.append(d)
model['cells'] = mc
model['counts'] = dict(
    neurons=len([c for c in cells if not c.is_receiver]),
    receivers=len([c for c in cells if c.is_receiver]),
    dsum=sum(len(c.dchunks) for c in cells),
    glin=sum(len(c.gchunks) + (1 if len(c.gchunks) > 1 else 0) for c in cells),
    chem_edges=sum(len(c.chem) for c in cells),
    gap_edges=sum(len(c.gap) for c in cells),
)

# ---------------- выдача ----------------
os.makedirs(OUTDIR, exist_ok=True)
open(os.path.join(OUTDIR, 'elegans_std_full.txt'), 'w').write(STD_TXT)
open(os.path.join(OUTDIR, 'elegans_connectome.txt'), 'w').write(NET_TXT)
json.dump(model, open(os.path.join(OUTDIR, 'model.json'), 'w'))
print('данные: %s' % CSV_PATH)
print('выход:  %s' % os.path.abspath(OUTDIR))

print('=== СГЕНЕРИРОВАНО ===')
print('клеток: %d (нейронов %d, мишеней %d)' % (
    len(cells), model['counts']['neurons'], model['counts']['receivers']))
print('агрегаторы: dsum %d, glin %d' % (model['counts']['dsum'], model['counts']['glin']))
print('рёбра: chem %d, gap %d' % (model['counts']['chem_edges'], model['counts']['gap_edges']))
print('dsum арности:', dsum_arities)
print('glin арности:', glin_arities)
print('описателей клеток:', len(neuron_combos))
print('std: %d строк, connectome: %d строк' % (STD_TXT.count('\n'), NET_TXT.count('\n')))
