# Algorithms and Data Structures Final Project - a.y. 2018-2019
The aim of the project is to design and implement a system for monitoring **relationships** between **entities**. 
The application, written in standard C language, is optimized to handle a great number of relations and entities, but generally few types of relationships. 
The instance to handle is given as a text file, read from *standard input*, that contains a list of commands, each one representing a change in the entities monitored or in the relations beetween them.

The accepted commands are:
|Command|Description|
|--------------|-----------|
|__addent__|adds a new entity to the monitored entities|
|__delent__|deletes an entity (and all its relations) from the monitored entities|
|__addrel__|adds a new relation between two monitored entities|
|__delrel__|deletes a reletion between two monitored entities|
|__report__|writes on *standard output*, in lexicographical order, the types of relations and, for each one of these, the entities that receive the largest number of that relation|
|__end__|stops the sequence of commands|

## Design and Implementation
![alt text](https://github.com/LorenzoMainetti/API-2019-FinalProject/blob/master/apiProject_Diagram.png)

The data structures implemented to handle the system are:
* Linked List
* Red-Black Tree

These two basic structures are combined together in order to better describe and represent the project. The following paragraphs illustrates the use of each single structure or sub-structure.

### Data Structures
#### 1) Red-Black Tree of Entities
Each time an entity starts or ceases to be monitored, it will be added to or deleted from the RB-tree of Entities.

Each node contains:
* name (key)
* color

#### 2) Linked List of Relations Types
Each time a new type of relation starts to be monitored, a new node will be added to the list. If the type is already part of the list, only the contents of the data structures inside each node are updated.

Each node contains:
* name (key)
* maximum number of relations received
* RB-tree of Recipients (Dest)
* RB-tree for the *report*

For any operation involving a relation, the relating node is added (or deleted) to the RB-tree of Recipients and, if necessary, the counter and the relative RB-tree for the report are updated.

#### 3) Red-Black Tree of Recipients
Each node contains:
* name (key)
* color
* counter of the number of relations received
* RB-tree of Origins 

The RB-tree of Origins keeps track of the entities from which that relation is received.

N.B. the RB-tree of Origins has the same structure of the RB-tree of Entities.

#### 4) Linked List as a temporary Buffer
This structure helps handle the *delent* command, when this is called on a entity that is part of a RB-tree of Origins. It keeps trace of entities that has to be deleted from the Dest RB-tree and the Report RB-tree when an Origin has been deleted.

### Algorithms
The main algorithms used to manage the previously illustrated data structures relate to the addition and deletion of nodes in a RB-tree and the its exploration through transversal visits, as well as the addition and deletion of nodes in a linked list.

## Commands' Time Complexity
Thanks to the use of RB-trees, the commands are all pretty fast. In the worst case scenario the time complexity is:
* addent :  **O(logn)**
* delent :  **O(nlogn)**
* addrel :  **O(logn)**
* delrel :  **O(logn)**
* report :  **O(n)**

## Testing
The code has been tested and debugged through [Public Tests](https://github.com/LorenzoMainetti/API-2019-FinalProject/tree/master/test/public) and evaluated through [Private Tests](https://github.com/LorenzoMainetti/API-2019-FinalProject/tree/master/test/private) executed on a website capable of defining the occupied memory and the execution time of the program.

## Compilation and Execution
To execute the program compile the file [main.c](https://github.com/LorenzoMainetti/API-2019-FinalProject/blob/master/main.c) from command line with the following flags:

  `gcc -Wmaybe-uninitialized -Wuninitialized -Wall -pedantic -Werror -g3 main.c -o main`

Then execute the program giving one of the Public Tests as the input file and compare the output with the expected output using:

 `cat input.txt | ./main > output.txt`
 
 `diff output.txt public_output.txt `
