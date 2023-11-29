#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <tfem-inter.h>
using namespace std;
//#include "tfem.h"



//int P;
int kWidth = 0, nMotifs = 0;
double threshold = 0.05;
bool verbose = false;
string output = "";
string input = "";


vector<string> readFile(string filename)
{
	ifstream file;
	file.open(filename);
	if (!file)
	{
		cout << "Unable to read the file!" << endl;
		exit(0);
	}
	vector<string> seq; //=new list<string>();
	string line;
	while (getline(file, line))
	{
		seq.push_back(line);
	}
	return seq;
}

void save(string filename, vector<Motif> res)
{
	ofstream file;
	file.open(filename);
	for (int i = 0; i < res.size(); i++)
		file << res[i].text << "," << res[i].probability << endl;
	file.close();
}

void error(string msg)
{
	cout << "Fatal Error: " << msg << endl;
	cout << "Usage:" << endl;
	cout << "   tfem  InputFilePath [-n N] [-k K] [-t T] [-v]" << endl;
	cout << "Options:" << endl;
	cout << "\t-n N \tmaximum number of motifs. The default is N=0 that implies no limitation on he maximum number of motifs." << endl;
	cout << "\t-k K \tthe motif width. The default is K=0 that implies no limitation on the motif width. K>1" << endl;
	cout << "\t-t T \tthe minimum similarity threshold. The default is T=0.01 that implies no limitation on the minimum similarity. 0<T<=1" << endl;
	cout << "\t-o OutputFilePath" << endl;
	cout << "\t-v \tShows the output on the command line." << endl;
	//cout << "\t-p \tthe number of threads." << endl;
	exit(0);
}

void parse(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		string s = string(argv[i]);
		if (s == "-k" || s == "-n" || s == "-t" || s == "-o" || s == "-v" || s == "-p")
		{
			if (i == argc - 1 && s != "-v")
				error("not enough arguments!");
			if (s == "-k")
				kWidth = stoi(argv[i + 1]);
			//if (s == "-p")
			//	P = stoi(argv[i + 1]);
			else if (s == "-n")
				nMotifs = stoi(argv[i + 1]);
			else if (s == "-t")
				threshold = stod(argv[i + 1]);
			else if (s == "-o")
				output = string(argv[i + 1]);
			else if (s == "-v")
			{
				verbose = true;
				i--;
			}
			i++;
		}
		else
		{
			//inputfile
			if (input != "")
				error("bad arguments list!");
			input = string(argv[i]);
		}
	}
	if (input == "")
		error("input file is required among the arguments!");
}

int main(int argc, char **argv)
{
	parse(argc, argv);
	vector<string> seq = readFile(input);
	tfem p;
	vector<Motif> res = p.findMotifs(seq, nMotifs, kWidth, threshold);
	if (output != "")
		save(output, res);
	if (output == "" || verbose == true)
	{
		for (int i = 0; i < res.size(); i++)
			cout << res[i].text << " " << res[i].probability << endl;
	}
	return 0;
}
