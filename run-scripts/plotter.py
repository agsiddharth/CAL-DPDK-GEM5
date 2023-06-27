def read_file(stats):
    with open(stats, 'r') as f:
        lines = f.readlines()
        lines = [line.strip() for line in lines]
        return lines


def collect_experiments(path):
    import os
    experiments = []
    for root, dirs, files in os.walk(path):
        for file in files:
            if file == 'stats.txt':
                experiments.append(os.path.join(root, file))
    return experiments

def get_latencies(experiments):
    latencies = []
    for exp in experiments:
        stats = read_file(exp)
        latencies.append(get_latency(exp))
    return latencies

def get_latency(exp):
    stats = read_file(exp)
    for line in stats:
        if 'system.loadgens.LoadGeneratorPcap.latency::mean' in line:
            return float(line.split("system.loadgens.LoadGeneratorPcap.latency::mean")[1].split()[0])


path_to_stats = "/home/sa10/CAL-DPDK-GEM5/rundir/"
print('Collecting experiments...')
experiments = collect_experiments(path_to_stats)
print('Found {} experiments'.format(len(experiments)))


PACKET_RATE = 10000
L2_EXPS = [x for x in experiments if '3GHz' in x and str(PACKET_RATE) in x]
FREQ_EXPS = [x for x in experiments if '1MB' in x and str(PACKET_RATE) in x]

print('Found {} L2 experiments'.format(len(L2_EXPS)))
print('Found {} FREQ experiments'.format(len(FREQ_EXPS)))

L2_SIZES = [x.split('/')[-2].split('-')[0].split('l')[0] for x in L2_EXPS]
## insert space between number and unit
L2_SIZES = [ float(x[:-2]) / 1000 if x[-2:] == 'kB' else float(x[:-2]) for x in L2_SIZES ]

L2_LATENCIES = get_latencies(L2_EXPS)

## sort l2 and l2_latencies together
L2_SIZES, L2_LATENCIES = zip(*sorted(zip(L2_SIZES, L2_LATENCIES)))


print(L2_SIZES)
print(*L2_LATENCIES, sep='\n')



# FREQS = [x.split('/')[-2].split('-')[1].split('f')[0] for x in FREQ_EXPS]
# FREQ_LATENCIES = get_latencies(FREQ_EXPS)

# ## sort freq and freq_latencies together
# FREQS, FREQ_LATENCIES = zip(*sorted(zip(FREQS, FREQ_LATENCIES)))

# print(FREQS)
# print(*FREQ_LATENCIES, sep='\n')