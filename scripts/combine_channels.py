import argparse

parser = argparse.ArgumentParser(description='Pack texture')
parser.add_argument('file1', metavar='file1', type=str, nargs='+',
                    help='First file')
parser.add_argument('file2', metavar='file2', type=str, nargs='+',
                    help='First file')



def main():
	args = parser.parse_args()
	file1 = args.file1
	file1 = args.file2

if __name__ == '__main__':
	main()
