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

using namespace std;

struct node
{
	struct node *parent;
	string state;
	int depth;
	double h;
};

class compareNode {
    public:
    bool operator()(node& x, node& y)
    {
    	return x.h > y.h;
    }
};

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

int main(int argc, char* argv[])
{
	if(argc < 3 || argc > 4)
	{
		cout << "Invalid argument numbers" << endl;
	}
	else
	{
		int i = 1;
		int length;
		bool cost = false;
		if(string(argv[i]) == "-cost")
		{
			i++;
			cost = true;
		}
		string search_strat = string(argv[i++]);
		cout << search_strat << endl;
		ifstream fin (argv[i]);

		node initial;
		fin >> initial.state;
		length = initial.state.length();
		initial.parent = NULL;
		initial.depth = 0;
		initial.h = 0;
		cout << initial.state << endl;
		priority_queue<node, vector<node>, compareNode> L;
		L.push(initial);
		while(!L.empty())
		{
			node current = L.top();
			L.pop();
			cout << current.state << "  " << current.depth << " ";
			if(goal_test(current.state))
			{

				cout << "found goal";
				break;
			}
			int k = 0;
			while(current.state[k]!='x')
			{
				k++;
			}
			cout << k << endl;
			for(int j = 0; j < length; j++)
			{
				if(j == k)
					continue;
				string temp = current.state;
				node successor;
				successor.parent = &current;
				temp[k] = temp[j];
				temp[j] = 'x';
				successor.state = temp;
				successor.depth = current.depth+1;
				// cout << temp << "   " << successor.depth << endl;
				successor.h = (double) successor.depth;
				L.push(successor);
			}

		}
		cout << "success" << endl;



	}







	return 0;
}