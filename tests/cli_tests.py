"""End-to-end CLI checks using independent JSON/CSV parsing and expected values."""
import csv
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

cli, root = sys.argv[1], Path(sys.argv[2])
with tempfile.TemporaryDirectory(prefix='orbitwatch-cli-') as directory:
    temp = Path(directory)
    for count in (100, 500):
        source = root / f'examples/demo-{count}.json'
        report = temp / f'report-{count}.json'
        subprocess.run([cli, str(source), '10000', str(report)], check=True)
        result = json.loads(report.read_text())
        assert result['source_sha256'] == hashlib.sha256(source.read_bytes()).hexdigest()
        known = [e for e in result['events']
                 if (e['first'], e['second']) == ('OBJ-1', 'OBJ-2')]
        assert len(known) == 1
        assert known[0]['minimum_distance_m'] == 100
        assert known[0]['minimum_us'] == 50_000_000
        csv_path = temp / f'report-{count}.csv'
        subprocess.run([cli, str(source), '10000', str(csv_path)], check=True)
        with csv_path.open() as stream:
            rows = list(csv.DictReader(stream))
        assert len(rows) == len(result['events'])
        assert float(rows[0]['minimum_distance_m']) == result['events'][0]['minimum_distance_m']
    generated = temp / 'generated.json'
    subprocess.run([cli, '--generate', '500', '1000', str(generated)], check=True)
    subprocess.run([cli, str(generated), '10000', str(temp / 'generated-report.json')], check=True)
    before = generated.read_bytes()
    assert subprocess.run([cli, str(generated), '10000', str(generated)], capture_output=True).returncode == 1
    assert generated.read_bytes() == before
    assert subprocess.run([cli, str(generated), 'nan', str(temp / 'invalid.json')], capture_output=True).returncode == 1
    assert not (temp / 'invalid.json').exists()
print('CLI end-to-end checks passed')
