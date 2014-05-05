// CS 4365.HON HW 3: Resolution Principle
// Josh Cai 

// compile with: 
//   g++ -o resolution resolution.cpp
// run with:
//   ./resolution [input_file]
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h> 

using namespace std;

struct clause
{
	int a, b; // reference to clauses if produced under resolution rule
	vector<int> list; // contains literals of the clause
};

vector<string> var; // holds list of var (literals)
vector<clause> clauses; // holds clauses
map<int, bool> checked; // for checking which clauses have been seen when finding which clauses to display 
vector<int> display; // stores which clauses to display for final output
vector<int> max_checked; // stores up to which clause a specific clause has checked against
bool found = false; // checks if False has been found or not

int get_num(string lit) // tells what num to put in the clause 
{
	// literals are stored in the var array
	// for example, the var array could look like: [a, b, c]
	// then 0 would correspond to a, 1 to ~a, 2 to b, 3 to ~b, 4 to c, and 5 to ~c
	string temp = lit;
	bool neg = false;
	int pos = -1;
	if(lit[0] == '~')
	{
		temp = lit.substr(1);
		neg = true;
	}
	// find position in var array
	for(int i = 0; i < var.size(); i++)
	{
		if(temp == var[i])
		{
			pos = i;
			break;
		}
	}
	// if not found, add to var array
	if(pos == -1)
	{
		pos = var.size();
		var.push_back(temp);
	}

	return (neg) ? (2 * pos + 1) : (2 * pos);
}

void print_real(int num) // prints what the var really looks like (for example: 1 would print ~a)
{
	if(num % 2 == 1)
		cout << "~";
	cout << var[num/2];
}

void print_clauses()
{
	for(int i = 0; i < clauses.size(); i ++)
	{
		cout << i+1 << ". ";
		if(clauses[i].list.empty())
		{
			cout << "False ";
		}
		else
		{
			for(int j = 0; j < clauses[i].list.size(); j++)
			{
				print_real(clauses[i].list[j]);
				cout << " ";
			}
		}
		cout << " {";
		if(clauses[i].a != -1)
		{
			cout << clauses[i].a+1 << "," << clauses[i].b+1;
		}
		cout << "}";
		cout << endl;			
	}
}

vector<int> new_clause(vector<int> list1, vector<int> list2, int skip1, int skip2) // generates a new list if two clauses can be resolved
{
	// also makes sure only one of each number is in the list
	vector<int> new_list;
	bool flag;
	for(int i = 0; i < list1.size(); i++)
	{
		if(i == skip1)
			continue;
		flag = true;
		for(int j = 0; j < new_list.size(); j++)
		{
			if(list1[i] == new_list[j])
			{
				flag = false;
				break;
			}
		}
		if(flag)
			new_list.push_back(list1[i]);
	}
	for(int i = 0; i < list2.size(); i++)
	{
		if(i == skip2)
			continue;
		flag = true;
		for(int j = 0; j < new_list.size(); j++)
		{
			if(list2[i] == new_list[j])
			{
				flag = false;
				break;
			}
		}
		if(flag)
			new_list.push_back(list2[i]);
	}

	return new_list;
}

bool check_if_true(vector<int> list) // checks if the clause would evaluate to true (if it has x V ~x)
{
	bool flag = false;
	for(int i = 0; i < list.size(); i++)
	{
		if(flag) break;
		for(int j = i + 1; j < list.size(); j++)
		{
			// the first part checks if they are within 1 of each other
			// the second part makes sure they are the same variable (0 and 1 returns 1 mod 4 but 1 and 2 returns 3 mod 4)
			if(abs(list[i] - list[j]) == 1 && (list[i] + list[j]) % 4 == 1)
			{
				flag = true;
				break;
			}
		}
	}
	return flag;

}

bool check_in_clauses(vector<int> list) // check if new clause is already in knowledge base, if so, do not add
{
	bool flag = false;
	bool flag2;
	for(int i = 0; i < clauses.size(); i++)
	{
		flag2 = true;
		if(clauses[i].list.size() != list.size())
			continue;
		for(int j = 0; j < clauses[i].list.size(); j++)
		{
			if(clauses[i].list[j] != list[j])
			{
				flag2 = false;
				break;
			}
		}
		if(flag2)
		{
			flag = true;
			break;
		}

	}	
	return flag;


}

void compare_clauses(clause a, clause b, int i, int j) // compares clauses to see if resolution rule can be used
{
	clause temp_clause;
	for(int k = 0; k < a.list.size(); k++)
	{
		if(found) break;
		for(int l = 0; l < b.list.size(); l++)
		{
			if(found) break;
			// same logic as line 122
			if(abs(a.list[k] - b.list[l]) == 1 && (a.list[k] + b.list[l]) % 4 == 1)
			{
				temp_clause.list = new_clause(a.list, b.list, k, l);
				sort(temp_clause.list.begin(), temp_clause.list.end());
				// if the clause doesn't have x V ~x and is not already in knowledge base, we should add to it
				if(!check_if_true(temp_clause.list) && !check_in_clauses(temp_clause.list)) 
				{
					temp_clause.a = i;
					temp_clause.b = j;
					clauses.push_back(temp_clause);
					max_checked.push_back(clauses.size() - 1);
					if(temp_clause.list.empty()) // if empty, means False was found
					{
						found = true;
					}
				}
			}
		}
	}
}

void find_display(int current) // populates the display vector with what to display
{
	if(checked.count(current) == 0)
	{
		checked[current] = true;
		display.push_back(current);
		if(clauses[current].a != -1)
		{
			find_display(clauses[current].a);
			find_display(clauses[current].b);
		}
	}
	return;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		cout << "Invalid arguments" << endl;
	}
	else
	{
		string temp;
		vector<int> temp_list;
		clause temp_clause;
		temp_clause.a = -1;
		temp_clause.b = -1;
		int trash;

		ifstream fin (argv[1]); // file with clauses

		stringstream ss;
		while(!fin.eof()) // read in file
		{
			temp_list.clear();
			getline(fin, temp);
			if(temp == "")
				continue;
			ss.clear();
			ss.str(temp);
			while(ss >> temp)
			{
				temp_list.push_back(get_num(temp));
			}
			sort(temp_list.begin(), temp_list.end()); // sort to make it easier to find duplicates later
			temp_clause.list = temp_list;
			clauses.push_back(temp_clause);
			max_checked.push_back(clauses.size() - 1);
		}

		bool maxed = false;
		while(!found && !maxed) // if not found and not maxed (maxed means all possible pairs have been compared for resolution)
		{
			// compares clauses
			for(int i = 0; i < clauses.size(); i++)
			{
				if(found) break;
				for(int j = max_checked[i]+1; j < clauses.size(); j++)
				{
					if(found) break;
					compare_clauses(clauses[i], clauses[j], i, j);
					max_checked[i] = j;
				}
			}
			if(!found)
			{
				maxed = true;
				for(int i = 0; i < clauses.size(); i++) // checks to see if all possible pairs have been compared
				{
					if(max_checked[i] < clauses.size() - 1)
					{
						maxed = false;
						break;
					}
				}
			}
		}
		if(!found)
		{
			cout << "Failure" << endl;
		}
		else // display output code
		{
			find_display(clauses.size() - 1); // finds which clauses need to be displayed
			sort(display.begin(), display.end()); // sorts to make sure it's in correct order
			for(int i = 0; i < display.size(); i ++)
			{
				cout << display[i]+1 << ". ";
				if(clauses[display[i]].list.empty())
				{
					cout << "False ";
				}
				else
				{
					for(int j = 0; j < clauses[display[i]].list.size(); j++)
					{
						print_real(clauses[display[i]].list[j]);
						cout << " ";
					}
				}
				cout << " {";
				if(clauses[display[i]].a != -1)
				{
					cout << clauses[display[i]].a+1 << "," << clauses[display[i]].b+1;
				}
				cout << "}";
				cout << endl;			
			}
		}
		cout << "Size of final clause set: " << clauses.size() << endl;
	}
	return 0;
}
