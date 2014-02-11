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
#include <math.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

// struct that contains the elements in each node
struct node
{
	struct node *parent;
	string state; // string representing state
	int depth, move, cost; 	// depth is depth from root, move is which position the tile moved to to get to this position
							// cost is how far that tile had to move to get to this position
	double h; // h is the heuristic evaluation function
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

// heuristic evaluation function - number of tiles out of place
double h(string state)
{
	int total = 0;
	int w = 0;
	int b = 0;
	int n = state.length() / 2;
	for(int i = 0; i < n; i++) // counts white and black tiles in the first n characters (n is half of entire string)
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

// tests if in goal position
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
	// so far has checked if first n char are 'B'
	return flag && state[n]=='x'; // so return if also the middle char is 'x' (since all other after must be 'W')

}

// prints path recursively
void print_path(node* current)
{
	// if not head, print_path of parent
	if(current->parent != NULL)
	{
		print_path(current->parent);	
		cout << "Step " << current->depth << ": move " << current->move << " " << current->state;
		if(cost)
		{
			cout << " (c=" << current->cost << ")";
		}
		cout << endl;
	}
	else // base case - initial state
	{
		cout << "Step 0: " << current->state << endl;
	}
	return;

}

// deallocates memory when not needed anymore
void dealloc_nodes(vector<node*>& V)
{
	for(int i = 0; i < V.size(); i++)
	{
		delete V[i];
	}
	V.clear();
}

int main(int argc, char* argv[])
{
	if(argc < 3 || argc > 4)
	{
		cout << "Invalid argument numbers" << endl;
	}
	else
	{
		srand(time(NULL));
		int i = 1;
		int t = 1; // counts iterations for HCS and time for SA
		double T;
		int length, strat;
		cost = false;
		if(string(argv[i]) == "-cost")
		{
			i++;
			cost = true;
		}
		string search_strat = string(argv[i++]);
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
		initial->h = h(initial->state);
		initial->cost = 0;
		
		// priority queue for UCS, GS, A-star, HCS
		priority_queue<node*, vector<node*>, compareNode> L;
		L.push(initial);

		// stack for DFS
		stack<node*> S;
		S.push(initial);

		// queue for BFS
		queue<node*> Q;
		Q.push(initial);
		
		// vector for SA
		vector<node*> V;
		// list of pointers to be deleted later
		vector<node*> delete_list;
		delete_list.push_back(initial);



		// map keeps track of visited nodes
		map<string, bool> checked;
		node* current;
		if(strat == 7)
			current = initial;

		while(!L.empty())
		{
			if(strat == 7)
			{
				// temperature function
				T = 1000 * pow(.995, t);
				if(T == 0)
					break;
			}

			if(strat == 6 && t > 10000) // if 10000 iterations and still haven't found for HCS
				break;
			t++;
			// gets current node, if SA, already has current node
			switch(strat)
			{
				case 1: // BFS
					current = Q.front();
					Q.pop();
					break;
				case 2: // DFS
					current = S.top();
					S.pop();
					break;
				case 3: // UCS
				case 4: // GS
				case 5: // A-star
					current = L.top();
					L.pop();
					break;
				case 6: // SA
					current = L.top();
					// stores current in a temp location before nodes in the queue get deallocated
					node* temp = new node;
					temp->state = current->state;
					temp->depth = current->depth;
					temp->move = current->move;
					temp->cost = current->cost;
					temp->h = current->h;
					L = priority_queue<node*, vector<node*>, compareNode>(); // clears L
					dealloc_nodes(delete_list);
					current = temp;
					break;

			}

			if(checked.count(current->state) == 0 || strat > 5) // if local search, or not expanded already (in non local)
			{
				checked[current->state] = true;
				// printing code for local search algorithms
				if(strat>5)
				{
					if(t == 2)
					{
						cout << "Step 0: " << current->state << endl;
					}
					else
					{
						cout << "Step " << t-2 << ": move " << current->move << " " << current->state;
						if(cost)
							cout << " (c=" << current->cost << ")";
						cout << endl;
					}

				}
				if(goal_test(current->state))
				{
					// if non-local then print path
					if(strat < 6)
						print_path(current);
					break;
				}
				// finds where the x is to see where to switch it
				int k = 0;
				while(current->state[k]!='x')
				{
					k++;
				}
				// starts generating successor states
				for(int j = 0; j < length; j++)
				{
					if(j == k)
						continue;
					string temp = current->state;
					node* successor = new node;
					if(strat < 6)
						successor->parent = current;
					successor->move = j;
					temp[k] = temp[j];
					temp[j] = 'x';
					successor->cost = ((j < k) ? k-j : j-k); // finds absolute value of j-k
					successor->state = temp;
					successor->depth = current->depth+1;
					delete_list.push_back(successor);
					switch(strat)
					{
						case 1: // BFS - use queue
							Q.push(successor);
							break;
						case 2: // DFS - use stack
							S.push(successor);
							break;
						case 3:  // UCS - if cost is true, use cost, else, use depth
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
						case 6: // HCS
								// for both, use h function
							successor->h = h(successor->state);
							break;
						case 5: // A-star, if cost is true, use cost + h, else, use depth + h
							if(cost)
							{
								successor->h = (double) successor->cost + h(successor->state);
							}
							else
							{
								successor->h = (double) successor->depth + h(successor->state);
							}
							break;
						case 7: // SA - use h function
							successor->h = h(successor->state);
							V.push_back(successor);
					}
					if(strat>2 && strat<7)
					{
						L.push(successor);
					}

				}
				if(strat == 6) // HCS
				{
					if((L.top())->h > current->h) // if the smallest of the sucessors is still greater than current, we are stuck
					{
						break;
					}
				}
				if(strat == 7) //SA
				{
					// choosing random successor
					int r = rand() % V.size();
					node* next;
					next = V[r];

					// can clear vector now
					V.clear();
					double change;
					change = next->h - current->h;
					if(change < 0) // got better since smaller h is better (tiles out of place)
					{
						current = next;
					}
					else // got worse
					{
						// get random value from 0 to 1
						double chance = (double) rand() / RAND_MAX;
						// if less than e^(change/T), next as current
						if(chance < exp(change/T))
							current = next;
					}
					// placing contents into temp location, since about to deallocate all nodes in delete_list
					node* temp = new node;
					temp->state = current->state;
					temp->depth = current->depth;
					temp->move = current->move;
					temp->cost = current->cost;
					temp->h = current->h;
					dealloc_nodes(delete_list);
					current = temp;

				}
			}

		}

		// free all memory
		dealloc_nodes(delete_list);

	}




	return 0;
}
