import os
from pathlib import Path

tests_dir = Path('tests')
n_tests = len(os.listdir(tests_dir)) // 2
res_dir = Path('results')
machine_code = 'code.llvm'
os.makedirs(res_dir, exist_ok=True)

cmd = 'make'
os.system(cmd)

for i in range(1, n_tests + 1):
    in_file = tests_dir / f'{i}.in'
    out_file = tests_dir / f'{i}.out'
    res_file = res_dir / f'{i}.res'
    cmd = f'./hw5 < {in_file} > {machine_code}'
    os.system(cmd)
    cmd = f'lli {machine_code} > {res_file}'
    os.system(cmd)
    out_txt = out_file.read_text()
    res_txt = res_file.read_text()
    if out_txt != res_txt:
        print(f'failed in test {i}.')
        print('*** Expected ***')
        print(out_txt)
        print('*** GOT ***')
        print(res_txt)
        break
