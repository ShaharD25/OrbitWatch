#!/usr/bin/env python3
"""Generate deterministic synthetic fixtures. No orbital propagation is performed."""
import argparse
import json
import math
from pathlib import Path


def generate(count, samples):
    objects = []
    for i in range(count):
        positions = []
        for k in range(samples):
            angle = 2 * math.pi * k / (samples - 1) + 2 * math.pi * i / count
            radius = 7_000_000 + 20_000 * (i % 7)
            positions.append([radius * math.cos(angle), radius * math.sin(angle),
                              100_000 * math.sin(angle + i)])
        objects.append({'id': f'OBJ-{i+1}', 'positions': positions})
    for k, position in enumerate(objects[0]['positions']):
        objects[1]['positions'][k] = [position[0] + 2000 * (k - samples // 2),
                                       position[1], position[2] + 100]
    return {'schema_version': 1, 'name': f'Synthetic demonstration: {count} objects',
            'units': 'm', 'time_unit': 'us', 'frame': 'SCENARIO_CARTESIAN',
            'times_us': [k * 1_000_000 for k in range(samples)], 'objects': objects}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--objects', type=int, choices=range(2, 501), default=100, metavar='2..500')
    parser.add_argument('--samples', type=int, choices=range(2, 10001), default=100, metavar='2..10000')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    with args.output.open('w') as stream:
        json.dump(generate(args.objects, args.samples), stream, separators=(',', ':'), allow_nan=False)
        stream.write('\n')
