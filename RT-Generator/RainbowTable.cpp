#include <iostream>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <random>
#include <algorithm>
#include <limits>
#include <ctime>
#include <cstring>
#include <functional>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include "RainbowTable.h"
//#include "blake_ref.h"


using namespace std;

static pthread_mutex_t Dictionary_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t Passwords_mutex = PTHREAD_MUTEX_INITIALIZER;

RainbowTable::RainbowTable(double StartSize, size_t Password_Length, int Chain_steps)
{
    chain_steps = Chain_steps;
    vertical_size = StartSize;
    password_length = Password_Length;
    srand(time(NULL));
}

void RainbowTable::Create_Table()
{
    cout << "creating Rainbow Table" << endl;
    int startTime = time(NULL);
    pthread_t threads[8];
    for(int i=0; i<8; i++)
    {
        pthread_create( &(threads[i]), NULL, Create_Rows, (void*) this);
    }
    for(int i=0; i<8; i++)
    {
        pthread_join(threads[i],NULL);
    }
    cout << "dictionary built with size = " << Get_Size() << endl;
    cout << "time to buid it : " << time(NULL) - startTime << " seconds" << endl;
}

void* RainbowTable::Create_Rows(void* arg)
{
    RainbowTable* rainbowTable = (RainbowTable*)arg;
    int counter;
    int limit = (rainbowTable->vertical_size)/8;
    string password;
    for (int i = 0; i<limit ; i++)
    {
        ///lock
        pthread_mutex_lock(&Passwords_mutex);
        do
        {
            password = rainbowTable->Get_Random_Password(rainbowTable->password_length);
            counter = (rainbowTable->original_passwords).count(password);

        }
        while(counter > 0);
        (rainbowTable->original_passwords).insert(password);
        ///unlock
        pthread_mutex_unlock(&Passwords_mutex);
        rainbowTable->Run_Chain(password, i);
    }
}

void RainbowTable::Load_from_file(string Filename)
{
    if (Get_Size() > 0)
    {
        string input;
        cout << "There is another Rainbow Table in memory, would you like to save it? ";
        cin >> input;
        if (input == "yes" || input == "Y" || input == "y" || input == "Yes" || input == "YES")
        {
            cout << "Enter the name of the output file to be generated : ";
            cin >> input;
            Save_to_file(input);
        }

        dictionary.clear();
    }

    string line1,line2;
    ifstream file(Filename);
    if (file.is_open())
    {
        while (getline(file, line1) && getline(file, line2))
        {
            dictionary[line1] = line2;
        }
        file.close();
    }
    else
    {
        cout << "Unable to open file, make sure it's on this folder" << endl;
        return;
    }
}

void RainbowTable::Save_to_file(string Filename)
{
    if (Get_Size() == 0)
    {
        cout << "Nothing to save to the file,"<< endl <<"first generate the rainbow table with Create_Table() function" << endl;
        return;
    }
    ofstream file;
    file.open(Filename);
    for (auto it = dictionary.begin(); it != dictionary.end(); ++it)
    {
        file << it->first << endl << it->second << endl;
    }
    file.close();
}

string RainbowTable::Find_Password(string StartHashedPassword)
{
    BitSequence LocalHashValue[64];
    BitSequence LocalPreHashValue[6];

    string hashedPassword = StartHashedPassword;
    if (hashedPassword.length() != 64)
    {
        cout << "Lookup hashed Password must be of 64 characters" << endl;
        return "error";
    }

    if (dictionary.size() == 0)
    {
        cout << "Dictionary is empty" << endl;
        string rt = "";
        return rt;
    }

    if (dictionary.count(hashedPassword) > 0)
    {
        //then the right chain is found
        //the position of the pass word is in that chain, step i
        string password = find_password_in_chain(StartHashedPassword, hashedPassword);
        cout << "password = " << password << endl ;
        return password;
    }
    else
    {
        for (int i = chain_steps - 1; i >= 0; i--)
        {
            char jasonTest[70];
            strcpy(jasonTest, StartHashedPassword.c_str());
            for (int z = 0, t = 0; z<32; ++z, t += 2)
                sscanf(jasonTest + t, "%02X", &(LocalHashValue[z]));

            for (int y = i; y < chain_steps; y++)
            {
                Reduction_Function(y,LocalHashValue,LocalPreHashValue);
                Hash(256, LocalPreHashValue, 48, LocalHashValue);
            }

            stringstream hashed_password_builder("");
            hashed_password_builder << std::hex << std::setfill('0');
            for (int i = 0; i < 32; i++)
                hashed_password_builder << std::setw(2) << static_cast<unsigned>(LocalHashValue[i]);
            hashedPassword = hashed_password_builder.str();

            if (dictionary.count(hashedPassword) > 0)
            {
                string password = find_password_in_chain(StartHashedPassword, hashedPassword);
                if(password != "password not found")
                {
                    cout << "password = " << password << endl;
                    return password;
                }
            }
        }
        string password = "password not found";
        cout << password << endl;
        return password;
    }
}


string RainbowTable::find_password_in_chain(string StartHashedPassword,string hashedPassword)
{
    string startPassword = dictionary[hashedPassword];
    BitSequence LocalPreHashValue[6];
    BitSequence LocalHashValue[64];

    for (int i = 0; i < 6; i++)
        LocalPreHashValue[i] = startPassword[i];

    for(int i = 0; i <= chain_steps; i++)
    {
        Hash(256,LocalPreHashValue,48,LocalHashValue);

        stringstream hashed_password_builder("");
        hashed_password_builder << std::hex << std::setfill('0');
        for (int z = 0; z < 32; z++)
            hashed_password_builder << std::setw(2) << static_cast<unsigned>(LocalHashValue[z]);
        string currentHashedPassword = hashed_password_builder.str();

        if (currentHashedPassword == StartHashedPassword)
        {//found password = prehashvalue
            string password;
            stringstream passwordBuilder("");

            for (int i = 0; i < 6; i++)
            {
                passwordBuilder << LocalPreHashValue[i];
            }
            password = passwordBuilder.str();
            return password;
        }
        else
        {
            Reduction_Function(i,LocalHashValue,LocalPreHashValue);
        }
    }
    return "password not found";
}

void RainbowTable::Run_Chain(std::string password,int salt)
{
    BitSequence LocalHashValue[64];
    BitSequence LocalPreHashValue[6];

    for(int y = 0; y<6; y++)
        LocalPreHashValue[y] = password[y];

    Hash(256,LocalPreHashValue,48,LocalHashValue);

    for(int i = 0; i < chain_steps; i++)
    {
        Reduction_Function(i,LocalHashValue,LocalPreHashValue);
        Hash(256,LocalPreHashValue,48,LocalHashValue);
    }

    stringstream final_hash("");
    final_hash << std::hex << std::setfill('0');
    for (int i = 0; i < 32; i++)
        final_hash << std::setw(2)  << static_cast<unsigned>(LocalHashValue[i]);
    string final_hash_str = final_hash.str();

    pthread_mutex_lock(&Dictionary_mutex);  ///lock dictionary

    if(dictionary.count(final_hash_str) == 0)
        dictionary[final_hash_str] = password;

    pthread_mutex_unlock(&Dictionary_mutex);    ///unlock dictionary
}

RainbowTable::~RainbowTable()
{
    cout<<"RainbowTable deleted"<<endl;
}

void RainbowTable::Reduction_Function(int salt,BitSequence* HashValue,BitSequence* PreHashValue)
{
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!@";

    for(int i = 0; i < 6; i++)
    {
        unsigned int index;
        index = HashValue[i] + HashValue[i+6] + HashValue[i+12] + HashValue[i+18] + HashValue[i+24] + salt;
        PreHashValue[i] = charset[index%64];
    }
}


void RainbowTable::Find_Password_Parallel(string StartHashedPassword)
{
    string hashedPassword = StartHashedPassword;
    if (hashedPassword.length() != 64)
    {
        cout << "Lookup hashed Password must be of 64 characters" << endl;
        return;
    }
    if (dictionary.size() == 0)
    {
        cout << "Dictionary is empty" << endl;
        return;
    }
    if (dictionary.count(hashedPassword) > 0)
    {
        //then the right chain is found
        //the position of the pass word is in that chain, step i
        string password = find_password_in_chain(StartHashedPassword, hashedPassword);
        cout << "password = " << password << endl ;
        return;
    }
    else
    {
        pthread_t lookupThreads[8];
        for(int i = 0 ; i < 8 ; i++)
        {
            Find_Password_Args* args = new Find_Password_Args();
            args->StartHashedPassword = StartHashedPassword;
            args->startIndex = chain_steps - 1 - i;
            args->chain_steps = chain_steps;
            args->Dictionary = &dictionary;
            args->table = this;
            pthread_create( &(lookupThreads[i]), NULL, task, (void*) args);
        }
        for(int i = 0 ; i < 8 ; i++)
        {
            pthread_join(lookupThreads[i],NULL);
        }
    }
    cout<<"search terminated"<<endl;
}


void* RainbowTable::task(void* argument)
{
    BitSequence LocalHashValue[64];
    BitSequence LocalPreHashValue[6];

    Find_Password_Args* args = (Find_Password_Args*)argument;
    string StartHashedPassword = args->StartHashedPassword;
    int startIndex = args->startIndex;
    int chain_steps = args->chain_steps;
    map<std::string,std::string>* Dictionary = args->Dictionary;
    RainbowTable* table = args->table;

    for(int i = startIndex ; i>=0 ; i-=8)
    {
        char jasonTest[70];
        strcpy(jasonTest, StartHashedPassword.c_str());
        for (int z = 0, t = 0; z<32; ++z, t += 2)
            sscanf(jasonTest + t, "%02X", &(LocalHashValue[z]));

        for (int y = i; y < chain_steps; y++)
        {
            table->Reduction_Function(y,LocalHashValue,LocalPreHashValue);
            Hash(256, LocalPreHashValue, 48, LocalHashValue);
        }

        stringstream hashed_password_builder("");
        hashed_password_builder << std::hex << std::setfill('0');
        for (int i = 0; i < 32; i++)
            hashed_password_builder << std::setw(2) << static_cast<unsigned>(LocalHashValue[i]);
        string hashedPassword = hashed_password_builder.str();

        if (Dictionary->count(hashedPassword) > 0)
        {
            string password = table->find_password_in_chain(StartHashedPassword, hashedPassword);
            if(password != "password not found")
            {
                cout << "password = " << password << endl;
                break;
            }
        }
    }
}


void RainbowTable::Print()
{
    for ( auto it = dictionary.begin(); it != dictionary.end(); ++it )
    {
        cout<<endl<<"=="<<it->second<<"  ->  "<<it->first<<endl;;
    }
}

string RainbowTable::Get_Random_Password(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!@";
        return charset[ rand()/(RAND_MAX / 64)];
    };
    string str(length,0);
    generate_n( str.begin(), length, randchar );
    return str;
}
