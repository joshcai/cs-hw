// CS 4365.HON HW 4: Bayesian Network
// Josh Cai 

// compile with: 
//   g++ -o BayesNet BayesNet.cpp
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

struct factor
{
	vector<char> vars; // holds vars in this factor
	map<string, double> p; // same as var.p but for factor
};

static const struct var emptyVar; // to reset struct
static const struct factor emptyFactor; // to reset struct
vector<var> net; // holds the bayes net
map<char, int>  ind; // map of char to index of array
char val[2]; // holds 'f' and 't'

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

void print_step(vector<char> vars, map<char,char> e, double value)
{
	string temp;

	for(int i = 0; i < vars.size(); i ++)
	{
		temp+= vars[i];
		temp+= " ";
	}
	cout << setw(15) << left << temp << "| ";
	map<char,char>::iterator it;
	temp="";
	for(it = e.begin(); it != e.end(); ++it)
	{
		temp+= it->first;
		temp+= "=";
		temp+= it->second;
		temp+= " ";
	}
	cout << setw(25) << left << temp << " = ";
	printf("%.8f\n", value);
}

double enum_all(vector<char> vars, map<char,char> e)
{
	vector<char> vars_old = vars;
	map<char,char> e_old = e;
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
	if(e.find(next) == e.end()) //not found in e (hidden var)
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
	print_step(vars_old, e_old, sum);
	return sum;

}

void print_result(char a, string replace)
{
	string temp = "";
	temp+= replace.substr(0,3);
	temp+= " = "; 
	temp+= a;
	if(replace.length() != 4)
	{
		temp+= " ";
		for(int i = 3; i < replace.length(); i++)
		{
			temp+= replace[i];
			if(i < replace.length() - 1 && replace[i+1]!= ',' && replace[i+1]!=')')
				temp+= " ";
		}
	}
	else
	{
		temp+=" | )";
	}
	cout << temp << " = ";
}

void recurse(map<string, double> & p, string assignment, vector<char> v, map<char,int> num_a, map<char,int> num_b, map<string, double> a_p, map<string, double> b_p)
{
	if(assignment.length() == v.size())
	{
		string a_q, b_q;
		a_q.resize(num_a.size());
		b_q.resize(num_b.size());
		double prob = 1.0;
		for(int i = 0; i < v.size(); i ++)
		{
			if(num_a.find(v[i]) != num_a.end()) // inside of num_a map
			{
				a_q[num_a[v[i]]] = assignment[i];
			}
			if(num_b.find(v[i]) != num_b.end()) // inside of num_a map
			{
				b_q[num_b[v[i]]] = assignment[i];
			}
		}
		prob *= a_p[a_q];
		prob *= b_p[b_q];
		p[assignment] = prob;

	}
	else
	{
		recurse(p, assignment+"t", v, num_a, num_b, a_p, b_p);
		recurse(p, assignment+"f", v, num_a, num_b, a_p, b_p);
	}
}

factor multiply(factor a, factor b) // generates new factor that is the pointwise product of two factors
{
	if(b.vars.size() == 0)
		return a;
	if(a.vars.size() == 0)
		return b;
	factor c;
	vector<char> v;
	map<char,int> num_a; // indicates which index it came from
	map<char,int> num_b; // indicates which index it came from
	for(int i = 0; i < a.vars.size(); i ++)
	{
		v.push_back(a.vars[i]);
		num_a[a.vars[i]] = i;
	}
	for(int i = 0; i < b.vars.size(); i ++)
	{
		if(find(v.begin(), v.end(), b.vars[i]) == v.end()) // not in set
		{
			v.push_back(b.vars[i]);
		}
		num_b[b.vars[i]] = i;
	}
	string a_query, b_query;
	recurse(c.p, "", v, num_a, num_b, a.p, b.p);
	c.vars = v;
	return c;

}

factor real_sum_out(factor a, char next)
{
	factor b;
	int x;
	for(int i = 0; i < a.vars.size(); i ++)
	{
		if(a.vars[i] != next)
		{
			b.vars.push_back(a.vars[i]);
		}
		else
		{
			x = i;
		}
	}
	map<string,double> ::iterator it;
	string temp, q1, q2;
	for(it = a.p.begin(); it != a.p.end(); ++it)
	{
		temp = it->first;
		temp.erase(x,1);
		if(b.p.find(temp) == b.p.end()) // prob not in b.p yet
		{
			q1 = it->first;
			q2 = it->first;
			q1[x] = 't';
			q2[x] = 'f';
			b.p[temp] = a.p[q1] + a.p[q2]; // sums the probabilities
		}
	}

	return b;
}

vector<factor> sum_out(char next, vector<factor> factors)
{
	factor product;
	vector<factor> new_factors;
	for(int i = 0; i < factors.size(); i++)
	{
		if(find(factors[i].vars.begin(), factors[i].vars.end(), next) != factors[i].vars.end()) // var is in this factor
		{
			product = multiply(product, factors[i]);
		}
		else
		{
			new_factors.push_back(factors[i]);
		}
	}
	factor temp_f;
	temp_f = real_sum_out(product, next);
	if(temp_f.vars.size() > 0)
	{
		new_factors.push_back(temp_f);
	}
	return new_factors;
}

void print_factor(factor a) // prints out factor in pretty form
{
	map<string,double>::iterator it;
	for(it = a.p.begin(); it != a.p.end(); ++it)
	{
		for(int i = 0; i < it->first.length(); i ++)
		{
			cout << a.vars[i] << "=" << it->first[i];
			if(i < it->first.length() - 1)
				cout << " ";
		}

		cout << ":  " << it->second << endl;
	}
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
		string query;
		query = string(argv[3]);
		q = query[2];
		int i = 4;
		while(i < query.length())
		{
			observed[query[i]] =  query[i+2];
			i+=4;
		}

		// set ups intial vars to get passed in
		vector<char> vars;
		for(int i = 0; i < net.size(); i++)
		{
			vars.push_back(net[i].name);
		}
		double Q[2];
		double s;

		if(elim) //elimination-ask algorithm
		{	
			map<char,int> avail;
			bool flag;
			char next;
			int min = -1;
			int size, x;
			vector<factor> factors;
			factor temp_factor;
			while(vars.size()>0) // since variables are getting eliminated, it will eventually be zero
			{
				// code to find the next variable to be eliminated
				min = -1;
				for(int i = 0; i < vars.size(); i++)
				{
					flag = true;
					for(int j = 0; j < vars.size(); j++)
					{
						if(i==j)
							continue;
						// if one of the other ndoes has this current node as a parent
						// means that it can't be eliminated yet
						if(find(net[ind[vars[j]]].parents.begin(), net[ind[vars[j]]].parents.end(), vars[i]) != net[ind[vars[j]]].parents.end())
						{
							flag = false;
							break;
						}
					}
					if(flag)
					{
						size = net[ind[vars[i]]].parents.size();
						if(observed.find(vars[i]) == observed.end()) // not found in observed, so add 1
							size++;
						if(min == -1 || size < min)
						{
							avail.clear();
							avail[vars[i]] = i;
							min = size;
						}
						else if(size == min)
						{
							avail[vars[i]] = i;
						}
					}
				}

				map<char,int>::iterator it2 = avail.begin();
				x = it2->second; // gets index of where it was in the vars array
				next = vars[x]; // gets actual character
				vars.erase(vars.begin()+x, vars.begin()+x+1); // deletes next var to be eliminated from vars vector

				// creating new factor
				temp_factor = emptyFactor;

				// finding which vars belong in the factor
				// over the parents of next
				for(int i = 0; i < net[ind[next]].parents.size(); i++)
				{
					if(observed.find(net[ind[next]].parents[i]) == observed.end())
					{
						temp_factor.vars.push_back(net[ind[next]].parents[i]);
					}
				}
				if(observed.find(next) == observed.end()) // not observed
				{
					temp_factor.vars.push_back(next);
				}

				// finding which probabilities to put in the factor

				map<string,double>::iterator temp_it;
				// over the query values of next
				for(temp_it = net[ind[next]].p.begin(); temp_it!= net[ind[next]].p.end(); ++temp_it)
				{
					flag = true;
					temp = "";
					// over the parents 
					for(int i = 0; i < temp_it->first.length() - 1; i ++)
					{
						if(observed.find(net[ind[next]].parents[i]) == observed.end())
						{
							temp+=temp_it->first[i];
							continue;
						}
						if(temp_it->first[i] != observed[net[ind[next]].parents[i]])
						{
							flag = false;
							break;
						}
					}
					if(observed.find(next) == observed.end()) // not observed
					{
						temp+=temp_it->first[temp_it->first.length() - 1];
					}
					else if(temp_it->first[temp_it->first.length() - 1] != observed[next])
					{
						flag = false;
					}
					if(flag)
					{
						temp_factor.p[temp] = temp_it->second;
					}
				}
				if(temp_factor.vars.size() > 0)
				{
					factors.push_back(temp_factor);
				}
				if(next!=q && observed.find(next) == observed.end()) // hidden variable
				{
					factors = sum_out(next, factors);
				}


				cout << "----- Variable: " << next << " -----" << endl << "Factors: " << endl;
				for(int i = 0; i < factors.size(); i ++)
				{
					print_factor(factors[i]);
					cout << endl;
				}
			}

			// compute final factor
			temp_factor = factors[0];
			for(int i = 1; i < factors.size(); i ++)
			{
				temp_factor = multiply(temp_factor, factors[i]);
			}

			for(int i = 0; i < 2; i++)
			{
				temp = "";
				temp+= val[i];
				Q[i] = temp_factor.p[temp];
				s+= Q[i];
			}

		}
		else //enumeration-ask algorithm
		{
			for(int i = 0; i < 2; i++)
			{
				observed[q] = val[i];
				Q[i] = enum_all(vars, observed);
				s+= Q[i];
			}
			cout << endl;
		}

		// normalization (same for both algorithms)
		cout << "RESULT:" << endl;
		for(int i = 0; i < 2; i ++)
		{
			Q[i]/=s;
			print_result(val[i], query);
			printf("%.16f\n", Q[i]);
		}

	}
	return 0;
}
