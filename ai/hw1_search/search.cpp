// CS 4365.HON HW 1: Basic Search
// Josh Cai 

// compile with: 
//   g++ -o search search.cpp
// run with:
//   ./search [-cost] <BFS|DFS|UCS|GS|A-star|HCS|SA> <inputfile>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <map>

using namespace std;

struct node
{
	struct node *parent;
	string state;
	int depth, move, cost;
	double h;
};

bool cost;

// own compare function for the node struct
class compareNode {
    public:
    bool operator()(node* x, node* y)
    {
    	return x->h > y->h;
    }
};

double h(string state)
{
	int total = 0;
	int w = 0;
	int b = 0;
	int n = state.length() / 2;
	for(int i = 0; i < n; i++)
	{
		if(state[i] == 'B')
			b++;
		else if(state[i] == 'W')
			w++;
	}
	total = w+n-b; // w white tiles out of place and (n-b) black tiles out of place
	if(state[n] == 'W') // have to consider the char in the middle - if it's white, it didn't get counted
		total++;
	return (double) total;

}

bool goal_test(string state)
{
	int n = state.length() / 2;
	bool flag = true;
	for(int i = 0; i < n; i++)
	{
		if(state[i]!='B')
		{
			flag = false;
			break;
		}
	}
	return flag && state[n]=='x';

}

void print_path(node* current)
{
	if(current->parent != NULL)
	{
		print_path(current->parent);	
		cout << "Step " << current->depth << ":   move " << current->move << " " << current->state;
		if(cost)
		{
			cout << " (c=" << current->cost << ")";
		}
		cout << endl;
	}
	else
	{
		cout << "head";
		cout << "Step 0:   " << current->state << endl;
	}
	return;

}

int main(int argc, char* argv[])
{
	if(argc < 3 || argc > 4)
	{
		cout << "Invalid argument numbers" << endl;
	}
	else
	{
		int i = 1;
		int length, strat;
		cost = false;
		if(string(argv[i]) == "-cost")
		{
			i++;
			cost = true;
		}
		string search_strat = string(argv[i++]);
		cout << search_strat << endl;
		strat = 0;
		if(search_strat == "BFS")
			strat = 1;
		else if(search_strat == "DFS")
			strat = 2;
		else if(search_strat == "UCS")
			strat = 3;
		else if(search_strat == "GS")
			strat = 4;
		else if(search_strat == "A-star")
			strat = 5;
		else if(search_strat == "HCS")
			strat = 6;
		else if(search_strat == "SA")
			strat = 7;
		ifstream fin (argv[i]);

		node* initial = new node;
		fin >> initial->state;
		length = initial->state.length();
		initial->parent = NULL;
		initial->depth = 0;
		initial->move = -1;
		initial->h = 0;
		cout << initial->state << endl;
		

		priority_queue<node*, vector<node*>, compareNode> L;
		L.push(initial);

		stack<node*> S;
		S.push(initial);

		queue<node*> Q;
		Q.push(initial);



		map<string, bool> checked;
		node* current;

		while(!L.empty())
		{
			switch(strat)
			{
				case 1:
					current = Q.front();
					Q.pop();
					break;
				case 2:
					current = S.top();
					S.pop();
					break;
				case 3:
				case 4:
				case 5:
					current = L.top();
					L.pop();
					break;
				case 6:
					current = L.top();
					L = priority_queue<node*, vector<node*>, compareNode>();
					break;

			}

			if(checked.count(current->state) == 0)
			{
				checked[current->state] = true;
				cout << current->state << "  " << current->depth << " ";
				// if(current->parent!=NULL)
				// {
				// 	cout << " parent " << current->parent->state << " ";
				// 	cout << &current << "  " << current->parent << " ";
				// }
				if(goal_test(current->state))
				{
					print_path(current);
					cout << "found goal";
					break;
				}
				int k = 0;
				while(current->state[k]!='x')
				{
					k++;
				}
				cout << k << endl;
				for(int j = 0; j < length; j++)
				{
					if(j == k)
						continue;
					string temp = current->state;
					node* successor = new node;
					successor->parent = current;
					successor->move = j;
					temp[k] = temp[j];
					temp[j] = 'x';
					successor->cost = ((j < k) ? k-j : j-k); // finds absolute value of j-k
					successor->state = temp;
					successor->depth = current->depth+1;
					// cout << temp << "   " << successor->depth << endl;
					switch(strat)
					{
						case 1:
							Q.push(successor);
							break;
						case 2:
							S.push(successor);
							break;
						case 3:  // UCS
							if(cost)
							{
								successor->h = (double) successor->cost;
							}
							else
							{
								successor->h = (double) successor->depth;
							}
							break;
						case 4: // GS
						case 6:
							successor->h = h(successor->state);
							break;
						case 5: // A-star
							if(cost)
							{
								successor->h = (double) successor->cost + h(successor->state);
							}
							else
							{
								successor->h = (double) successor->depth + h(successor->state);
							}
							break;
					}
					if(strat>2 && strat<7)
					{
						L.push(successor);
					}

				}
			}

		}
		cout << "success" << endl;



	}


	//remember to deallocate memory




	return 0;
}