// CS 4365.HON HW 4: Bayesian Network
// Josh Cai 

// compile with: 
//   g++ -o resolution resolution.cpp
// run with:
//   ./BayesNet <bayesnet> <enum|elim> <query>


#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h> 
#include <stdio.h>
#include <iomanip>

using namespace std;

struct var
{
	char name;
	vector<char> parents;
	map<string,double> p;
	// p holds current var too
	// so if p has no parents
	// "T" will give probability of P(name)
	// and "F" will give probability of ~P(name) 
};

static const struct var emptyVar; // to reset struct
vector<var> net; // holds the bayes net
map<char, int>  ind; // map of char to index of array
char val[2];

double calc(char v, map<char,char> e)
{
	string t = ""; 
	int v2 = ind[v];
	for(int i = 0; i < net[v2].parents.size(); i++)
	{
		 t+=e[net[v2].parents[i]];
	}
	t+=e[v];
	return net[v2].p[t];
}

double enum_all(vector<char> vars, map<char,char> e)
{
	if(vars.size() == 0)
		return 1.0;
	char next; // next var to get computed
	int x; // index of next
	// pick first 
	map<char, int> avail;
	bool flag;
	for(int i = 0; i < vars.size(); i++)
	{
		flag = true;
		for(int j = 0; j < net[ind[vars[i]]].parents.size(); j++)
		{
			// if one of the parents is in the vars array
			// means parent has not been visited yet
			if(find(vars.begin(), vars.end(), net[ind[vars[i]]].parents[j]) != vars.end())
			{
				flag = false;
				break;
			}
		}

		if(flag)
		{
			avail[vars[i]] = i;
		}
	}
	// maps automatically sort by key, so we can just take the first one in the map
	map<char,int>::iterator it = avail.begin();
	x = it->second; // gets index of where it was in the vars array
	next = vars[x];
	vars.erase(vars.begin()+x, vars.begin()+x+1);


	double sum = 0.0;
	if(e.find(next) == e.end()) //not found
	{
		for(int i = 0; i < 2; i ++)
		{
			e[next] = val[i];
			sum+=calc(next, e) * enum_all(vars, e);
		}
	}
	else // found
	{
		sum = calc(next, e) * enum_all(vars, e);
	}
	printf("%.8f\n", sum);
	return sum;

}

void print_result(char a, string replace)
{
	string temp = "";
	temp+= replace.substr(0,3);
	temp+= " = "; 
	temp+= a;
	temp+= " ";
	for(int i = 3; i < replace.length(); i++)
	{
		temp+= replace[i];
		if(i < replace.length() - 1 && replace[i+1]!= ',' && replace[i+1]!=')')
			temp+= " ";
	}
	cout << temp << " = ";
}


int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		cout << "Invalid arguments" << endl;
	}
	else
	{

		bool elim = (string(argv[2]) == "elim" ? true : false); // true if elim, false otherwise

		var temp_var;

		char q; // query variable
		map<char,char> observed; // observed variables

		// sets default values in val array
		val[0] = 'f';
		val[1] = 't';


		ifstream fin (argv[1]); // bayes net file
		stringstream ss;
		string temp, temp2;
		double temp_d;


		// for(int i = 0; i < observed.size(); i ++)
		// {
		// 	cout << observed[i].first << " " << observed[i].second << endl;
		// }

		// parse file
		while(!fin.eof())
		{
			temp_var = emptyVar;
			getline(fin, temp);
			if(temp[0] == 'P' && temp[1] == '(')
			{
				temp_var.name = temp[2];
				ss.clear();
				ss.str(temp);
				ss >> temp >> temp; // throw away first two parts - P(X) and =
				ss >> temp_d;
				temp_var.p["t"] = temp_d;
				temp_var.p["f"] = 1.0 - temp_d; 
				getline(fin, temp); // removes trash newline after
			}
			else
			{
				// parse header
				ss.clear();
				ss.str(temp);
				while(ss >> temp)
				{
					if(temp == "|")
						break;
					else
						temp_var.parents.push_back(temp[0]);
				}
				ss >> temp_var.name;
				getline(fin, temp); // trash line
				while(1)
				{
					getline(fin, temp);
					if(temp == "")
						break;
					temp2 = "";
					ss.clear();
					ss.str(temp);
					while(ss >> temp)
					{
						if(temp == "|")
							break;
						else
							temp2+=temp;
					}				
					ss >> temp_d;
					temp_var.p[temp2+"t"] = temp_d;
					temp_var.p[temp2+"f"] = 1 - temp_d;
				}
			}
			ind[temp_var.name] = net.size(); // adds name / index to ind map
			net.push_back(temp_var); // adds current var to the net
		}

		// parse query
		temp = string(argv[3]);
		q = temp[2];
		int i = 4;
		while(i < temp.length())
		{
			observed[temp[i]] =  temp[i+2];
			i+=4;
		}
		// debug code
		// cout << q << endl << endl;
		// map<char,char>::iterator it2;
		// for(it2 = observed.begin(); it2 != observed.end(); ++it2)
		// 	{
		// 		cout << it2->first << "  " << it2->second << endl;
		// 	}

		double Q[2];
		double s;
		// set ups intial vars to get passed in
		vector<char> vars;
		for(int i = 0; i < net.size(); i++)
		{
			vars.push_back(net[i].name);
		}

		// enumeration-ask part of the algorithm
		for(int i = 0; i < 2; i++)
		{
			observed[q] = val[i];
			Q[i] = enum_all(vars, observed);
			s+= Q[i];
		}
		// normalization
		cout << "RESULT:" << endl;
		for(int i = 0; i < 2; i ++)
		{
			Q[i]/=s;
			// cout << "P(" << q << " = " << val[i] << " | ";
			// for(int j = 0; j < observed.)
			print_result(val[i], temp);
			printf("%.16f\n", Q[i]);
			// cout << setprecision(16) << Q[i] << endl;
		}







		// debug network
		// map<string,double>::iterator it;
		// for(int i = 0; i < net.size(); i ++)
		// {
		// 	cout << net[i].name << endl;
		// 	for(int j = 0; j < net[i].parents.size(); j++)
		// 		cout << net[i].parents[j] << " ";
		// 	cout << endl;
		// 	for(it = net[i].p.begin(); it != net[i].p.end(); ++it)
		// 	{
		// 		cout << it->first << "  " << it->second << endl;
		// 	}

		// }


	}
	return 0;
}
