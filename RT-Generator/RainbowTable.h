#include <unordered_set>
#include "blake_ref.h"
class RainbowTable;
class Find_Password_Args
{
    public :
        std::string StartHashedPassword;
        int startIndex;
        int chain_steps;
        std::map<std::string,std::string>* Dictionary;
        RainbowTable* table;
};

class RainbowTable
{
public:
    RainbowTable(double StartSize, size_t Password_Length, int Chain_steps);
    ~RainbowTable();
	void Create_Table();
    void Print();
    int Get_Size(){return dictionary.size();}
    std::string Find_Password(std::string StartHashedPassword);
    void Find_Password_Parallel(std::string StartHashedPassword);
	void Save_to_file(std::string);
	void Load_from_file(std::string);
private:
    static void* task(void* argument);
    static void* Create_Rows(void* arg);

    std::string Get_Random_Password(size_t length);
    void Reduction_Function(int Salt,BitSequence* H,BitSequence* PreH);
    void Run_Chain(std::string password,int salt);
    void Step();
    std::string find_password_in_chain(std::string StartHashedPassword, std::string hashedPassword);

    std::map<std::string,std::string> dictionary;
    std::unordered_set<std::string> original_passwords;
    double vertical_size;
    int chain_steps;
    size_t password_length;
};


