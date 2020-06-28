import os
from pathlib import Path
from argparse import ArgumentParser

def run_tests(test_dir):
    print(f'running test in {test_dir}')
    res_file = Path('res.out')
    for in_file in test_dir.glob('*.in'):
        test_name = in_file.stem
        tests_dir = in_file.parent 

        out_file = tests_dir / (test_name + '.out')
        cmd = f'./hw5 < {in_file} > {machine_code}'
        os.system(cmd)
        cmd = f'lli {machine_code} > {res_file}'
        os.system(cmd)
        out_txt = out_file.read_text()
        res_txt = res_file.read_text()
        if out_txt != res_txt:
            print(f'failed in test {test_name}.')
            print('*** Expected ***')
            print(out_txt)
            print('*** GOT ***')
            print(res_txt)
            exit(0)
        print(f'passed test {test_name}')

parser = ArgumentParser()
parser.add_argument('--test', type=str, default='all',
                    help="enter here the name of the tests directory of you want to run only them\n\
                         for example enter --test=\"shai-omer\"\n\
                         if not mentioned, will run all tests.")
args = parser.parse_args()

tests_dir = Path('tests')
machine_code = 'code.llvm'

cmd = 'make'
os.system(cmd)

if args.test == 'all':
    for sub_tests in tests_dir.glob('*'):
        run_tests(sub_tests)
else:
    sub_tests = tests_dir / args.test
    run_tests(sub_tests)

print("finished with success.")