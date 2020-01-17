#include "iostream"
#include "ArgumentManager.h"
#include "string"
#include "fstream"
using namespace std;
//===================================================================================NODE========================================================================================
class Node {
public:
	int degree = 1; // degree of the tree (maximum keys inside a node
	Node *prev; // pointer pointing back to the parent (utility)
	Node **children; // children array (pointers of the data in the btree, left and right)
	int *data; // keys array
	int count = 0; // to keep track of the number of keys inserted into the keys array (data[])
	Node(int degree) // constructor of Node
	{
		this->degree = degree; // set degree
		this->prev = NULL; // point to nothing for now
		this->children = new Node*[degree + 1]; // children array will have 1 more pointers than data
		this->data = new int[degree]; // create enough slots for keys array (note the slots are equal to degree for now, but when count = degree, the node will be split
		int i = 0;
		for (0; i < degree; i++) // make both arrays "empty"
		{
			children[i] = NULL;
			data[i] = 0; // 0 is symbolic for empty
		}
		children[degree] = NULL; // since children has degree + 1 compared to keys array, we have to set the last to empty also
	}
	void insertNonFull(int item); // inserting item into the node and sort them in the correct order (full node checking is not performed here. Instead, it is done in tree class insert)
};
void Node::insertNonFull(int item)
{
	int i;
	for (i = 0; i < degree; i++)
	{
		if (data[i] == 0) // get index for insertion when we reached an empty slot in the array
			break; // we got the index, so we break the loop
		else if (data[i] > item) // if the item is less than the current key, we need to insert the item to the left
		{                        // however, we cannot add it to the left like in linked lists, so we need to move the array right
			for (int j = degree; j > i; j--) // move the array right one slot from the end to the index (insertion will be done at i)
				data[j] = data[j - 1]; 
			break; // insertion is done, so we break the loop
		}
	}
	data[i] = item; // add the item to the index
	count++; // increase the key count in the node
}
//==================================================================================B-TREE=========================================================================================
class Tree {
public:
	Node *root; // root :)
	int degree; // same as Node
	int count; // idk why there's a count here. It's not used in this Tree (count in Node is used, don't mistake one with another)
	Tree(int degree) 
	{
		this->root = NULL; // set root to empty for now until the first insertion
		this->degree = degree; // maximum keys in one node
		this->count = 0; // yea idk
	}
	//INSERT ADT
	bool searchNode(Node* cu, int searchItem, int &location); // Search the whole tree to find the data
	bool search(int searchItem); // search for duplicate data inside the node
	void insert(int data); // call insert and travel to item into data array, then splitnode is called to split the node if count = degree
	void insert_and_traverse(Node* &cu, int item, bool &done);//for inserting inside nodes inside the tree (without actual splitting) or traversing to the correct position for insertion
	void splitNode(Node* &cu, bool &done); // split the node and connect when the key count is full (count = degree)
	//PRINT ADT
	int countLevel(Node *x, int level); // count the total number of level of the tree (for printing empty if the command of printXLevel exceeds max level of the current tree
	void printXLevel(int currentLevel, int printLevel, int treeLevel, Node *x, ofstream &ofs); // Print out every keys that are on level X
	void inOrder(Node* cu, ofstream &ofs); // Inorder Traversal and print
	void inOrderPrint(ofstream& ofs); // make the function neater 
	void printAtLevel(int level, ofstream& ofs); // same as above
};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++INSERTION IMPLEMENTATION++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool Tree::searchNode(Node *cu, int searchItem, int &location) // search for duplicate inside the node
{
	location = 0; //index 1 of the array
	while (location < cu->count && searchItem > cu->data[location]) // loop and move while location (index) is less than the number of keys inside 
		location++;													// AND search item is still larger than the key at index
	if (location < cu->count && searchItem == cu->data[location])   // If the location (index) has not reached the very end and the item is found
		return true;
	else // If we reach the end (or not found in other words)
		return false;
}
bool Tree::search(int searchItem) // search for duplicate data inside the node
{
	bool found = false; // boolean to check for duplicate
	int location; // for looping and traversing in the node of the trees
	Node *cu = root;
	while (cu != NULL && !found) // loop while we're still in the tree and there's no duplicate
	{
		found = searchNode(cu, searchItem, location);
		if (!found) // if not found, travel down at the suitable location and loop until found or when we travel out of the tree
			cu = cu->children[location];
	}
	return found; //either found or not found
}
void Tree::insert_and_traverse(Node* &cu, int item, bool &done)//for inserting inside nodes inside the tree (without actual splitting) or traversing to the correct position for insertion
{
	if (cu == NULL) //if tree is empty
	{
		cu = new Node(degree); // create new root
		cu->data[cu->count] = item; // set first key as data
		cu->count = 1; // increment data count
		done = true; // insertion is made so the task is done (done = true)
	}
	else if (cu->children[0] == NULL) // tree only has a root (or possibly node and not root specifically, not sure yet) and no children
	{// insert data inside the root (or node). It doesn't matter if the key count is the same as degree because it will be split  in the loop below if key counts = degree
		cu->insertNonFull(item);
		done = true; // insertion is made so the task is done (done = true)
	}
	else if (cu->children[0] != NULL) // if the leftmost branch is not NULL (possibly tree has more nodes than just the root itself)
	{// This part is only for traversing to the correct Nodes to insert without actual insertion. The loop will make cu go down to the correct node, and the insertion will be made in the condition above
		int lastNum = cu->count; // last number will be at the key count that we keeps track
		int i = 0;
		for (i; i < cu->count; i++)
		{
			if (cu->data[i] > item)  // if data is less than key. Move down in that branch
			{
				cu = cu->children[i]; // move
				break;
			}
		}
		if (i == lastNum) // if the data is larger than all key (possibly?)
			cu = cu->children[cu->count]; // move down the last branch
	}
}
void Tree::splitNode(Node* &cu, bool &done)// split the node and connect when the key count is full (count = degree)
{
	if (cu->count == degree) // if node is full
	{
		Node *temp = new Node(degree);
		temp->prev = cu->prev;
		int tmp = 0; // index for assignment of cu's info to temp
		int cutIndex = int((degree - 1) / 2); // median (or not, because that doesn't look like the median but eh. The book uses median but any position is fine)
		int carry = cu->data[cutIndex]; // temp var for median value of the node
		cu->data[cutIndex] = 0; // delete the median value
		cu->count = cu->count - 1; // reduce key count
		int i = cutIndex + 1; // move to position next to median
		for (i; i < degree; i++)
		{
			temp->children[tmp] = cu->children[i];// assign children of cu to temp, starts with leftmost branch (children[0])
			if (temp->children[tmp] != NULL) // if child node is not empty
				temp->children[tmp]->prev = temp; // connect child node prev to temp
			cu->children[i] = NULL; // cut the connection of cu after transferring to temp
			temp->data[tmp] = cu->data[i]; //assign data from cu to temp
			cu->data[i] = 0; // delete data at i in cu after assignment
			tmp++; // increase tmp
			cu->count = cu->count - 1; // decrease cu count by 1 after transferring 1 key to temp
			temp->count = temp->count + 1; // increase temp count by 1 after getting one key from cu
		}
		temp->children[tmp] = cu->children[i]; // i reached degree (max). Assign children of cu (at the very end) to temp (I think)
		if (temp->children[tmp] != NULL) // if temp's child node at position tmp of the pointer array is not empty
			temp->children[tmp]->prev = temp; // connect child's prev to temp
		if (cu->prev == NULL) // if cu doesn't have a parent (possible root)
		{
			Node *parentNode = new Node(degree); // create a new parent
			parentNode->data[0] = carry; // assign carry data (median from above) to the first position of parent node
			parentNode->children[0] = cu; // parent's leftmost branch is cu
			parentNode->children[1] = temp; // parent's rightmost branch is temp
			parentNode->count = 1; // increase parent's key count to 1
			cu->prev = parentNode; // connect cu's prev to parent node
			temp->prev = parentNode; // same with temp. This is to make sure that these node are not mistaken as root I think
			cu = parentNode; // make old root = new root (or maybe just cu)
			done = true;
		}
		else // if cu is not the root
		{
			cu = cu->prev; // go back to cu's parent
			int tmpi = carry;
			int tmpo = 0;
			Node *nodeTemp = NULL;
			int i = 0;
			for (i; i < cu->count; i++) // move through cu's keys until a key is larger than the carry
			{
				if (cu->data[i] > carry) // find correct position and start connecting the new node to the child pointer slot, while moving the child to the next spot
				{
					nodeTemp = cu->children[i + 1]; // assign nodeTemp to the right child of the key at i in cu
					cu->children[i + 1] = temp; // connect the right child of the key at i in cu to temp
					temp = nodeTemp; //assign temp with the branch assign to nodeTemp above
					tmpo = cu->data[i]; // assign the data of the key at i in cu to tmpo
					cu->data[i] = tmpi; // assign the data at i with the carr. I think this is to move the median of a child to the parent
					tmpi = tmpo; // assign tmpi (carry) with the data from i above
				}
			} 
			cu->children[i + 1] = temp; //connect the branch in cu with tmpi_
			cu->data[i] = tmpi; // assign the data at i with tmpi (whatever it is from the loop)
			cu->count++; // increase cu key count after the assignment above
		}
	}
	else if (cu->prev != NULL) // if not parent
		cu = cu->prev;
	else
		done = true;
}
void Tree::insert(int data) // call insert and travel to item into data array, then splitnode is called to split the node if count = degree
{
	Node *cu = root; //move with cu. Classic
	bool done = false; // flag to check for the goal of the 2 loop. I'm pretty sure that the first loop done means there's an insertion, and the second one is done when there's a split
	if (search(data)) // if there's a duplicate in the tree. break the operation
		return;
	while (!done) // pretty sure this part is for inserting data inside a node
		insert_and_traverse(cu, data, done);
	done = false;
	while (!done) // split node (if applicable)
		splitNode(cu, done);
	root = cu; // assign the root to cu and finalize the new tree
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++PRINTING IMPLEMENTATION++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int Tree::countLevel(Node *cu, int level) // count the level of the tree. Make sure to pass the level as 0
{
	level++; // increment level every recursion call
	if (cu->children[0] != NULL) // if the left most branch of the node still exist, make recursion call
		return countLevel(cu->children[0], level); // move cu to the next node at the leftmost branch
	else // stop when the left most branch ends (last node pointer at position 1 - leftmost branch - points to NULL
		return level;
}
void Tree::printXLevel(int currentLevel, int printLevel, int treeLevel, Node *x, ofstream &ofs) // Print all element on that level. Make sure to pass currentLevel as 0
{
	currentLevel++; // increment currentLevel with every recursion call
	if (printLevel > treeLevel) // if the printLevel exceed the tree's maximum level treeLevel, print out empty
		ofs << "empty";
	else if (currentLevel == printLevel && x != NULL) //print out all elements on the correct level
		for (int i = 0; i < x->count; i++)
			ofs << x->data[i] << " ";
	else if (currentLevel < printLevel && x != NULL) // traverse through the level if not at the correct level
		for (int i = 0; i < x->count + 1; i++)
			printXLevel(currentLevel, printLevel, treeLevel, x->children[i], ofs);
}
void Tree::inOrder(Node* cu, ofstream &ofs) // Print the whole tree in order
{
	if (cu != NULL) // traverse through the tree until we go through every nodes
	{
		inOrder(cu->children[0], ofs); // go through only the left most branch of every node until the lowest level
		for (int i = 0; i < cu->count; i++) // traverse through every other nodes that is not on the leftmost branch
		{
			ofs << cu->data[i] << " "; // print out data. Obviously :^|
			inOrder(cu->children[i + 1], ofs); // go through every other nodes of the tree as i increments
		}
	}
}
void Tree::inOrderPrint(ofstream& ofs) // only need to pass ofs (neater than (tree->root, ofs))
{
	inOrder(root, ofs);
}
void Tree::printAtLevel(int level, ofstream& ofs) // neater than caller the function inside
{
	printXLevel(0, level, countLevel(root, 0), root, ofs);
}
//=================================================================================MAIN============================================================================================
int main(int argc, char *argv[])
{
//-----------------------------------------------------------------------ARGUMENTS AND FSTREAM STUFF------------------------------------------------------------------------------
	if (argc < 2)
	{
		cout << "Invalid arguments" << endl;
		return 1;
	}
	ArgumentManager am(argc, argv);
	string input = am.get("input");
	string output = am.get("output");
	string command = am.get("command");
	ifstream ifs(input);
	ifstream com(command);
	ofstream ofs(output);
//-------------------------------------------------------------------------GET INPUT AND MAKE OUTPUT--------------------------------------------------------------------------------
	string num, line="", level, degree; // num = input from inputXX.txt | line for getline | level for separating Level XX that can appear in commandXX.txt | degree is tree degree
	while (line == "") // Shehzad test case has a bunch of empty space before the degree. Seems unnecessary, but here it is.
	{ // carriage return here too cuz get line
		getline(com, line); // get the lines after degree (first line of command) to print out the correct format
		if (line.empty()) // ignore empty line
			continue;
		if (line[line.size() - 1] == '\r') // delete carriage return
			line = line.substr(0, line.size() - 1);
		if (line.empty()) // seems redundant but my old program broke without this due to some special cases. May not be needed in this one, but I put here just in case
			continue;
	}
	degree = line.substr(line.find('=') + 1); // assign degree with the number found after '=' in the line at the end
	Tree *tree = new Tree(stoi(degree)); // create Tree with degree got above. Convert string to int
	while (ifs >> num) // insert input to tree
		tree->insert(stoi(num));
	while (!com.eof()) // printing
	{
		getline(com, line); // get the lines after degree (first line of command) to print out the correct format
		if (line.empty()) // ignore empty line
			continue;
		if (line[line.size() - 1] == '\r') // delete carriage return
			line = line.substr(0, line.size() - 1);
		if (line.empty()) // seems redundant but my old program broke without this due to some special cases. May not be needed in this one, but I put here just in case
			continue;
		else
		{
			if (line.empty())
				continue;
			level = line.substr(line.find(' ') + 1); // assign level with the number found after the space ' ' in the line at the end
			if (line == "Inorder Traversal") // if "Inorder Traversal", call inOrder
				tree->inOrderPrint(ofs);
			else // if not Inorder Traversal, call printXLevel
				tree->printAtLevel(stoi(level), ofs);
			ofs << endl; // there's no endl in the 2 printing functions, so this is necessary (no endl because of recursion)
		}
	}
	return 0;
}