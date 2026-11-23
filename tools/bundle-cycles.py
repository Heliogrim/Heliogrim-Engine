# file-level header graph with file moves (path=Bundle or dir/*=Bundle), dropped includes (!path), bundle merges (Sub~Root)
# prints remaining bundle SCCs and conversion layers over the unconverted bundles
# R1: library bundles (not Heliogrim*, Editor.*, *.Test, *.Main) must not include/import Heliogrim; prints violations, exit 1
import os, re, sys, collections
moves, drops, merge = {}, set(), {}
for a in sys.argv[1:]:
    if a.startswith('!'): drops.add(a[1:])
    elif '~' in a: s, r = a.split('~'); merge[s] = r
    else: k, v = a.split('='); moves[k] = v
SEP = os.sep
B = [d for d in os.listdir('src') if os.path.exists(f'src/{d}/CMakeLists.txt') and not d.endswith('.Test')]
mod = set()
for b in B:
    t = open(f'src/{b}/CMakeLists.txt', encoding='utf-8').read()
    if re.search(r'^\s*MODULE\s*$', t, re.M) or os.path.exists(f'src/{b}/CxxModule.ixx'): mod.add(b)
    m = re.search(r'SUB_OF\s+(\S+)', t)
    if m: merge.setdefault(b, m.group(1))
mod |= {b for b in B if merge.get(b) in mod}
def owner(rel):
    for k, v in moves.items():
        if rel == k or (k.endswith('*') and rel.startswith(k[:-1])): return v
    b = rel.split('/')[0]
    return b
def node(b): return merge.get(b, b)
inc = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.M)
G = collections.defaultdict(set); W = collections.Counter(); nodes = set()
for b in B:
    for dp, _, fs in os.walk('src/' + b):
        for f in fs:
            if not f.endswith(('.hpp', '.inl', '.h')): continue
            rel = os.path.join(dp, f).replace(SEP, '/')[4:]
            sb = owner(rel)
            if sb in mod or sb.endswith('.Test'): continue
            nodes.add(node(sb))
            for q, h in inc.findall(open('src/' + rel, encoding='utf-8', errors='ignore').read()):
                t = h if q == '<' else os.path.normpath(os.path.join(os.path.dirname(rel), h)).replace(SEP, '/')
                if t in drops or (rel + '>' + t) in drops: continue
                tb = owner(t)
                if tb.split('/')[0] not in B and tb not in moves.values(): continue
                if tb in mod or node(tb) == node(sb): continue
                G[node(sb)].add(node(tb)); W[(node(sb), node(tb))] += 1
idx, low, st, on, out, c = {}, {}, [], set(), [], [0]
sys.setrecursionlimit(10000)
def sc(v):
    idx[v] = low[v] = c[0]; c[0] += 1; st.append(v); on.add(v)
    for w in G[v]:
        if w not in idx: sc(w); low[v] = min(low[v], low[w])
        elif w in on: low[v] = min(low[v], idx[w])
    if low[v] == idx[v]:
        comp = []
        while True:
            w = st.pop(); on.discard(w); comp.append(w)
            if w == v: break
        out.append(sorted(comp))
for v in sorted(nodes):
    if v not in idx: sc(v)
big = [s for s in out if len(s) > 1]
print('SCCs>1:', big)
for s in big:
    S = set(s)
    for (a, b2), w in sorted(W.items()):
        if a in S and b2 in S: print(f'   {a} -> {b2}: {w}')
comp = {v: i for i, s in enumerate(out) for v in s}
dep = {}
def d(i):
    if i not in dep:
        dep[i] = 0
        dep[i] = max([d(comp[w]) + 1 for v in out[i] for w in G[v] if comp[w] != i] or [0])
    return dep[i]
L = collections.defaultdict(list)
for i, s in enumerate(out): L[d(i)].append('+'.join(s))
for k in sorted(L): print(f'L{k}:', ' '.join(sorted(L[k])))
bad = {v for s in big for v in s}
reach = {}
def r(v, seen=None):
    if v in reach: return reach[v]
    reach[v] = v in bad
    reach[v] = v in bad or any(r(w) for w in G[v])
    return reach[v]
free = sorted(v for v in nodes if not r(v))
print('FREE', len(free), ' '.join(free))
imp = re.compile(r'^\s*(?:export\s+)?import\s+(Heliogrim(?:\.[\w.]+)?)\s*;', re.M)
isH = lambda b: b == 'Heliogrim' or b.startswith('Heliogrim.')
r1 = []
for b in B:
    for dp, _, fs in os.walk('src/' + b):
        for f in fs:
            if not f.endswith(('.hpp', '.inl', '.h', '.ccm', '.ixx')): continue
            rel = os.path.join(dp, f).replace(SEP, '/')[4:]
            sb = owner(rel)
            if isH(sb) or sb.startswith('Editor.') or sb.endswith(('.Test', '.Main')): continue
            src = open('src/' + rel, encoding='utf-8', errors='ignore').read()
            for q, h in inc.findall(src):
                t = h if q == '<' else os.path.normpath(os.path.join(os.path.dirname(rel), h)).replace(SEP, '/')
                if (rel + '>' + t) in drops or t in drops: continue
                if isH(owner(t)): r1.append(f'R1: {rel} -> {t}')
            r1 += [f'R1: {rel} -> import {m}' for m in imp.findall(src)]
for l in sorted(r1): print(l)
sys.exit(1 if r1 else 0)
