// CS 4365.HON HW 2: CSP Solver
// Josh Cai 

// compile with: 
//   g++ -o cspsolver cspsolver.cpp
// run with:
//   ./cspsolver [file1.var] [file2.con] [none|fc]
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <climits>

using namespace std;


struct constraint //struct to hold constraint
{
	int a, b; // index of variable in var vector (so if A is in index 0, this stores 0)
	char op;
};

// global var to avoid passing in so many times
int num_var; // total num of variabels
int c; // for printing out number of branch we're on
vector<char> var; // holds list of var characters 
vector<constraint> con; // holds all constraints
vector<vector<int> > values; // holds possibly var domain
bool fc; // forward checking flag

int find(char c) // finds index of a variable in the var vector
{
	for(int i = 0; i < var.size(); i++)
	{
		if(var[i] == c)
			return i;
	}
	return -1; // error
}

bool pass_constraints(int assign[], bool assign_flag[], int current_var, int current_val) // checks if current_var = current_val passes all constraints
{
	bool flag = true;
	for(int l = 0; l < con.size(); l++)
	{
		if(con[l].a != current_var && con[l].b != current_var) // if var isn't in this constraint, pass it
			continue;
		if(!assign_flag[con[l].a] && !assign_flag[con[l].b]) // other var isn't assigned, pass it
			continue;
		// the other var is assigned
		if(assign_flag[con[l].a]) // con[l].a is assigned
		{
			switch(con[l].op)
			{
				case '<':
					if(assign[con[l].a] >= current_val) // if constraint not met, set flag to false
						flag = false;
					// cout << var[con[l].a] << values[j][k]<< " " << con[l].op << " " << var[con[l].b] << assign[con[l].b] <<endl;
					break;
				case '!':
					if(assign[con[l].a] == current_val)
						flag = false;
					break;
				case '=':
					if(assign[con[l].a] != current_val)
						flag = false;
					break;
			}
		}
		else // con[l].b is assigned
		{
			switch(con[l].op)
			{
				case '<':
					if(current_val >= assign[con[l].b]) // if constraint not met, set flag to false
				
						flag = false;
						// cout << var[con[l].a] << values[j][k]<< " " << con[l].op << " " << var[con[l].b] << assign[con[l].b] <<endl;
					break;
				case '!':
					if(current_val == assign[con[l].b])
						flag = false;
					break;
				case '=':
					if(current_val != assign[con[l].b])
						flag = false;
					break;
			}					
		}
	}

	return flag;
}

int select_unassigned(int assign[], bool assign_flag[]) // selects next unassigned variable according to most constrained and most constraining heuristics
{
	int index = -1;
	int legal = INT_MAX; //num legal moves
	int constraining = -1; // num constraints on other variables (for tie-breaking)
	int temp, temp_con;
	bool flag;
	for(int i = 0; i < num_var; i++)
	{
		if(assign_flag[i])
			continue;
		if(fc) // if forward checking we need to count number of constraints
			temp = 0;
		else // otherwise, it's just number of values in domain
			temp = values[i].size();
		temp_con = 0;
		for(int j = 0; j < values[i].size(); j++)
		{
			flag = true;
			for(int k = 0; k < con.size(); k++)
			{
				if(con[k].a != i && con[k].b != i) // if var isn't in this constraint, pass it
					continue;
				if(!assign_flag[con[k].a] && !assign_flag[con[k].b]) // other var isn't assigned, pass it
				{
					temp_con++; // this means it constrains an unassigned variable, so add to counter in case of tie
					continue;
				}
				if(fc)
				{
					// the other var is assigned
					if(assign_flag[con[k].a]) // con[k].a is assigned
					{
						switch(con[k].op)
						{
							case '<':
								if(assign[con[k].a] >= values[i][j]) // if constraint not met, set flag to false
									flag = false;
								break;
							case '!':
								if(assign[con[k].a] == values[i][j])
									flag = false;
								break;
							case '=':
								if(assign[con[k].a] != values[i][j])
									flag = false;
								break;
						}
					}
					else // con[k].b is assigned
					{
						switch(con[k].op)
						{
							case '<':
								if(values[i][j] >= assign[con[k].b]) // if constraint not met, set flag to false
									flag = false;
								break;
							case '!':
								if(values[i][j] == assign[con[k].b])
									flag = false;
								break;
							case '=':
								if(values[i][j] != assign[con[k].b])
									flag = false;
								break;
						}					
					}
				}

			}
			if(fc && flag)
				temp++;
		}

		if(temp < legal || (temp == legal && temp_con > constraining)) // most constrained, if tie, most constraining
		{
			legal = temp;
			index = i;
			constraining = temp_con;
		}
		else if(temp == legal && temp_con == constraining && var[i] < var[index]) // if still tied, break by alphabetical
		{
			legal = temp;
			index = i;
			constraining = temp_con;
		}
	}
	return index;
}

vector<int> value_select(int next, int assign[], bool assign_flag[]) // does ordering of values for selection purposes
{
	vector<int> new_order;
	vector<int> h; // values to order by (least constraining heuristic)
	int t, counter, index, trash;
	bool flag;
	assign_flag[next] = true; //set to true because we pretend it's assigned
	for(int i = 0; i < values[next].size(); i++)
	{
		counter = 0;
		assign[next] = values[next][i];
		for(int j = 0; j < num_var; j++)
		{
			if(assign_flag[j]) //if assigned already, skip 
				continue;
			for(int k = 0; k < values[j].size(); k++) // start checking values that pass
			{
				flag = true;
				for(int l = 0; l < con.size(); l++)
				{
					if(con[l].a != j && con[l].b != j || con[l].a != next && con[l].b != next) // if var isn't in this constraint, pass it
						continue;
					if(!assign_flag[con[l].a] && !assign_flag[con[l].b]) // other var isn't assigned, pass it
						continue;
					// the other var is assigned
					if(assign_flag[con[l].a]) // con[l].a is assigned
					{
						switch(con[l].op)
						{
							case '<':
								if(assign[con[l].a] >= values[j][k]) // if constraint not met, set flag to false
									flag = false;
								// cout << var[con[l].a] << values[j][k]<< " " << con[l].op << " " << var[con[l].b] << assign[con[l].b] <<endl;
								break;
							case '!':
								if(assign[con[l].a] == values[j][k])
									flag = false;
								break;
							case '=':
								if(assign[con[l].a] != values[j][k])
									flag = false;
								break;
						}
					}
					else // con[l].b is assigned
					{
						switch(con[l].op)
						{
							case '<':
								if(values[j][k] >= assign[con[l].b]) // if constraint not met, set flag to false
							
									flag = false;
									// cout << var[con[l].a] << values[j][k]<< " " << con[l].op << " " << var[con[l].b] << assign[con[l].b] <<endl;
								break;
							case '!':
								if(values[j][k] == assign[con[l].b])
									flag = false;
								break;
							case '=':
								if(values[j][k] != assign[con[l].b])
									flag = false;
								break;
						}					
					}
				}
				if(flag) // passed all constraints
					counter++;
			}

		}
		new_order.push_back(values[next][i]);
		h.push_back(counter);
		// cout << "hello" << values[next][i] << " " << counter << endl;
	}
	//sort correctly before returning order
	for(int i = 0; i < new_order.size(); i ++)
	{
		index = -1;
		t = -1;
		for(int j = i; j < new_order.size(); j++)
		{
			if(h[j] > t || (h[j] == t && new_order[j] < new_order[index]))
			{
				index = j;
				t = h[j];
			}
		}
		trash = new_order[i];
		new_order[i] = new_order[index];
		new_order[index] = trash;
		trash = h[i];
		h[i] = h[index];
		h[index] = trash;
	}
	// cout << var[next] << " ";
	// for(int i =0; i < new_order.size(); i ++)
	// {
	// 	cout << new_order[i] << " " << h[i] << "  ";
	// }
	// cout << endl;
	return new_order;

}

void print_path(int assign[], vector<int> order, bool success) //prints variables and values in case of failure/success
{
	cout << ++c << ". ";
	for(int i = 0; i < order.size(); i ++)
	{
		cout << var[order[i]] << "=" << assign[order[i]];
		if(i < order.size() - 1)
			cout << ",";
		cout << " ";
	}
	if(success)
	{
		cout << " solution";
	}
	else
	{
		cout << " failure";
	}
	cout << endl;
}

vector<vector<int> > check_legal(int assign[], bool assign_flag[], vector<vector<int> > legal_val) // checks which values are legal for forward checking
{
	vector<vector<int> > new_legal;
	vector<int> temp_list;

	for(int i = 0; i < num_var; i++)
	{
		temp_list.clear();
		if(assign_flag[i])
		{
			temp_list.push_back(assign[i]);
		}
		else
		{
			for(int j = 0; j < legal_val[i].size(); j++)
			{
				if(pass_constraints(assign, assign_flag, i, legal_val[i][j]))
					temp_list.push_back(legal_val[i][j]);
			}
		}
		new_legal.push_back(temp_list);
	}
	return new_legal;

}

bool backtrack(int assign[], bool assign_flag[], vector<int> order, vector<vector<int> > legal_val)
{
	bool flag, result;
	vector<vector<int> > new_legal_val; // holds next set of legal values


	if(c > 29)
		return true;
	// if assignment is complete return true
	flag = true;
	for(int i = 0; i < num_var; i++)
		if(!assign_flag[i])
		{
			flag = false;
			break;
		}
	if(flag)
	{
		print_path(assign, order, true);
		return true;
	}
	
	if(fc) // check illegal, if no legal moves left for a variable, return false
	{
		new_legal_val = check_legal(assign, assign_flag, legal_val);
		flag = true;
		for(int i=0; i<num_var; i ++)
		{
			if(new_legal_val[i].size() == 0)
			{
				flag=false;
				break;
			}
		}
		if(!flag)
		{
			print_path(assign, order, false);
			return false;
		}
	}

	int next = select_unassigned(assign, assign_flag);
	order.push_back(next);
	vector<int> values_selected = value_select(next, assign, assign_flag);
	for(int i = 0; i < values_selected.size(); i ++)
	{
		assign_flag[next] = true;
		assign[next] = values_selected[i];

		flag = true;
		for(int j = 0; j < con.size(); j++)
		{
			if(!assign_flag[con[j].a] || !assign_flag[con[j].b]) // if one of the var in the constraint isn't assigned, skip
				continue;
			switch(con[j].op)
			{
				case '<':
					if(assign[con[j].a] >= assign[con[j].b]) // if constraint not met, set flag to false
						flag = false;
					break;
				case '!':
					if(assign[con[j].a] == assign[con[j].b])
						flag = false;
					break;
				case '=':
					if(assign[con[j].a] != assign[con[j].b])
						flag = false;
					break;
			}
		}
		if(flag)
		{
			result = backtrack(assign, assign_flag, order, new_legal_val); // if not fc, new_legal_val is blank, but doesn't matter because we don't use it
			if(result) 
				return true;
		}
		else
		{
			if(!fc)
				print_path(assign, order, false);
			if(c > 29)
				return true;
		}
	}
	assign_flag[next] = false;
	order.pop_back();
	return false;

}


int main(int argc, char* argv[])
{
	if(argc != 4 || (string(argv[3])!="fc" && string(argv[3]) != "none"))
	{
		cout << "Invalid arguments" << endl;
	}
	else
	{
		string temp;
		int temp_int;
		char a, b, op, t;
		vector<int> temp_list;
		constraint temp_c;

		c = 0;

		ifstream fin (argv[1]); // var_file
		ifstream fin2 (argv[2]); // con_file

		stringstream ss;
		// read in var_file
		while(!fin.eof())
		{
			temp_list.clear();
			getline(fin, temp);
			if(temp == "")
				continue;
			ss.clear();
			ss.str(temp);
			ss >> temp;
			var.push_back(temp[0]);
			while(ss >> temp_int)
			{
				temp_list.push_back(temp_int);
			}
			values.push_back(temp_list);
		}
		num_var = var.size();
		int assign [num_var];
		bool assign_flag [num_var];
		for(int i = 0; i < num_var; i ++)
		{
			assign[i] = 0;
			assign_flag[i] = false;
		}
		// read in con file
		while(!fin2.eof())
		{
			getline(fin2, temp);
			if(temp == "")
				continue;
			ss.clear();
			ss.str(temp);
			ss >> a;
			ss >> op;
			ss >> b;
			if(op == '>')
			{
				// switch order of a and b because any > constraint can be written as a < constraint
				op = '<';
				t = a;
				a = b;
				b = t;
			}
			temp_c.a = find(a);
			temp_c.b = find(b);
			temp_c.op = op;
			con.push_back(temp_c);
		}
		fc = false;
		if(string(argv[3]) == "fc")
			fc = true;
		vector<int> order; // says what order to output in
		backtrack(assign, assign_flag, order, values);
	}
	return 0;
}
