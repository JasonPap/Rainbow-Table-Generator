#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <map>
#include <string>
#include <time.h>
#include "RainbowTable.h"

using namespace std;

int main(void)
{
	string x;
	int startTime = time(NULL);
    RainbowTable* table = new RainbowTable(10000000,6,8000);
    cout<<"size before load: "<<table->Get_Size()<<endl;
    table->Load_from_file("testfile.txt");
    cout<<"size after load: "<<table->Get_Size()<<endl;
	cout << "give password hash to look for or exit to terminate" << endl;
	do
	{

		cin >> x;
		if(x=="exit")
            break;
		table->Find_Password_Parallel(x);
	} while (true);
	cout<<"done"<<endl;
}



