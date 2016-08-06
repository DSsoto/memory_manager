#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <vector>

#define nullptr 0

class Program;
class MemoryFragment;
class MemoryManager;
class MemoryFragmentList;

class Program{
  friend class MemoryManager;
public:
  Program(std::string name, size_t size);
  ~Program(){};
  std::string getName(){ return name; }
  size_t getSize(){ return size; }

private:
  std::string name;
  size_t size;
  MemoryFragment* allocated_memory_ptr;
};

class MemoryFragment{
  friend class MemoryFragmentList;
  friend class MemoryManager;
public:
  MemoryFragment(int _first_page, int _last_page);
  ~MemoryFragment(){};
  void allocate(int irst_page, size_t pages_required){};
  void free(){};
  Program* getOwningProgramPtr(){ return owner_program; }
  size_t range(){ return last_page - first_page + 1; }
private:
  int first_page;
  int last_page;
  Program* owner_program;
  MemoryFragment *next_fragment_ptr, *previous_fragment_ptr;
};

class MemoryFragmentList{
  friend class MemoryManager;
public:
    MemoryFragmentList();
    ~MemoryFragmentList(){};
    size_t countContiguousSections();
    MemoryFragment* current(){ return current_fragment; };
    MemoryFragment* next();
    MemoryFragment* previous();
    void goToHead(){ current_fragment = head; }
      MemoryFragment* smallest_available_fragment();
    MemoryFragment* largest_available_fragment();

private:
    MemoryFragment *current_fragment, *head, *tail;
};

class MemoryManager{
public:
  MemoryManager(std::string fit_mode);
  ~MemoryManager(){};
  bool addProgram(Program* prog);
  bool killProgram(std::string name);
  size_t getNumFragments(){ 
    return allocated_memory.countContiguousSections();
  }
  void printMemory();
  static const size_t PAGE_SIZE = 4;  // KB
  static const size_t PAGES = 32;
private:
  MemoryFragmentList free_memory;
  MemoryFragmentList allocated_memory;
  std::string fit_mode;
  MemoryFragment* findProgramMemoryByName(std::string program_name);
};