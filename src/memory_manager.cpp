/*
  Memory Manager
  Author: David Soto
  Date: 8/5/2016
*/ 

#include <memory_manager.hpp>

using namespace std;

int main(int argc, char** argv){
  string usage = "Usage: ./memory_manager <worst|best>\n";
  stringstream menu;
  menu << "\t1. Add program\n"
       << "\t2. Kill program\n"
       << "\t3. Fragmentation\n"
       << "\t4. Print Memory\n"
       << "\t5. Exit\n\nchoice - ";
  
  if(argc != 2){
    cout << usage;
    return -1;
  }

  string fit_mode = string(argv[1]);
  if(!fit_mode.compare("worst") && !fit_mode.compare("best")){
    cout << usage;
    return -1;
  }
  Program p1("p1", 8);
  cout << p1.getName() << endl << p1.getSize() << endl;

  cout << "Using " << fit_mode << " algorithm" << endl;

  MemoryManager mem_manager(fit_mode);

  while(true){
    static string selection = "";
    cout << menu.str();
    if(!(cin >> selection)){
      cout << "Input stream error! Shutting down. \n";
      cout << "Dump:\n";
      mem_manager.printMemory();
      return -1;
    }
    if(!selection.compare("1")){
      static string name = "";
      static string size_str = "";
      static size_t size = 0;
      cout << "Program name - ";
      cin >> name;
      cout << "Program size (KB) - ";
      cin >> size_str;
      size = atoi(size_str.c_str());
      Program* prog = new Program(name, size);
      mem_manager.addProgram(prog);
    }
    else if(!selection.compare("2")){
      static string name = "";
      cout << "Program name - ";
      cin >> name;
      mem_manager.killProgram(name);
    }
    else if(!selection.compare("3")){
      cout << "There are " << mem_manager.getNumFragments()
           << " fragment(s)" << endl << endl;
    }
    else if(!selection.compare("4")){
      mem_manager.printMemory();
    }
    else if(!selection.compare("5")){
      cout << "Exiting.\n" << endl;
      break;
    }
    else cout << selection << " is not a valid option" << endl;
  }
  return 0;
}

Program::Program(string _name, size_t _size){
  name = _name;
  size = _size;
  allocated_memory_ptr = nullptr;
}

MemoryFragment::MemoryFragment(int _first_page, int _last_page){ 
  first_page = _first_page;
  last_page = _last_page;
  owner_program = nullptr;
  next_fragment_ptr = nullptr;
  previous_fragment_ptr = nullptr;
}


MemoryManager::MemoryManager(string _fit_mode){
  MemoryFragment* initial_free = new MemoryFragment(0, PAGES - 1);
  free_memory.head = free_memory.tail 
    = free_memory.current_fragment = initial_free;
  fit_mode = _fit_mode;
}

bool MemoryManager::addProgram(Program* prog){
  MemoryFragment* free_fragment_for_allocation;
  // Choose a fragment of free memory based on fitting strategy
  if(fit_mode.compare("best")){
    free_fragment_for_allocation 
      = free_memory.smallest_available_fragment();
  }
  else free_fragment_for_allocation 
    = free_memory.largest_available_fragment();
  size_t pages_required 
    = size_t(ceil(double(prog->getSize()) / PAGE_SIZE)); 
  if(free_fragment_for_allocation->range() < pages_required){
    cout << "Error, not enough memory for Program " 
         << prog->getName() << endl;
    return false;
  }
  int first_page_of_new_alloc
    = free_fragment_for_allocation->first_page;
  MemoryFragment *new_fragment 
    = new MemoryFragment(first_page_of_new_alloc, 
                         first_page_of_new_alloc - 1
                         + pages_required);
  new_fragment->owner_program = prog;

  // Insert fragment to alloc_memory list

  // If alloc mem list is empty
  if(!allocated_memory.head){
    new_fragment->previous_fragment_ptr = nullptr;
    new_fragment->next_fragment_ptr = nullptr;
    allocated_memory.head = new_fragment;
    allocated_memory.tail = new_fragment;
    allocated_memory.current_fragment = new_fragment;
  }
  // If fragment should replace the head of alloc mem list
  else if(new_fragment->last_page
          < allocated_memory.head->first_page){
    new_fragment->next_fragment_ptr = allocated_memory.head;
    new_fragment->previous_fragment_ptr
      = allocated_memory.head->previous_fragment_ptr;
    allocated_memory.head = new_fragment;
    allocated_memory.current_fragment = new_fragment;
  }
  // If fragment should replace the tail of alloc mem list
  else if(new_fragment->first_page
          > allocated_memory.tail->last_page){
    new_fragment->previous_fragment_ptr = allocated_memory.tail;
    new_fragment->next_fragment_ptr = nullptr;
    allocated_memory.tail = new_fragment;
    allocated_memory.current_fragment = new_fragment;
  }
  // If fragment should be inserted into middle of alloc mem list
  else{
    allocated_memory.goToHead();
    while(true){
      allocated_memory.next();
      if(allocated_memory.current_fragment->last_page
         < first_page_of_new_alloc){
        allocated_memory.current_fragment->next_fragment_ptr
          = new_fragment;
        new_fragment->previous_fragment_ptr
          = allocated_memory.current_fragment;
        new_fragment->next_fragment_ptr = allocated_memory.next();
        new_fragment->next_fragment_ptr->previous_fragment_ptr
          = new_fragment;
        break;
      }
    }
  }

  // Remove recently allocated memory from free mem list

  // If whole fragment from free mem list must be removed
  if(pages_required == free_fragment_for_allocation->range()){
    // If last free fragment is being removed
    if(free_fragment_for_allocation == free_memory.head
       && free_fragment_for_allocation == free_memory.tail){
      free_memory.head = nullptr;
      free_memory.tail = nullptr;
      free_memory.current_fragment = nullptr;
    }
    // If head is being removed
    else if(free_fragment_for_allocation->previous_fragment_ptr){
      free_memory.head
        = free_fragment_for_allocation->next_fragment_ptr;
      free_memory.head->previous_fragment_ptr = nullptr;
    }
    // If tail is being removed
    else if(free_fragment_for_allocation->next_fragment_ptr){
      free_memory.tail
        = free_fragment_for_allocation->previous_fragment_ptr;
      free_memory.tail->next_fragment_ptr = nullptr;
    }
    // If middle fragment is being removed
    else{
      free_fragment_for_allocation->previous_fragment_ptr->next_fragment_ptr
        = free_fragment_for_allocation->next_fragment_ptr;
      free_fragment_for_allocation->next_fragment_ptr->previous_fragment_ptr
        = free_fragment_for_allocation->previous_fragment_ptr;
    }
    // delete fragment removed from free mem list
    delete free_fragment_for_allocation;

  }
  // If only part of fragment from free mem list will be allocated
  else free_fragment_for_allocation->first_page += pages_required;
  cout << "Program " << prog->getName() << " added successfully: "
       << pages_required << " page(s) used." << endl << endl;
  return true;
}


bool MemoryManager::killProgram(string name){
  allocated_memory.goToHead();
  MemoryFragment *memory_to_free
    = findProgramMemoryByName(name);
  if(memory_to_free){
    cout << "Error: program is not running\n";
    return false;
  }
  // Remove fragment from alloc mem list

  // If last allocated fragment is being removed
  if(memory_to_free == allocated_memory.head
     && memory_to_free == allocated_memory.tail){
    allocated_memory.head = nullptr;
    allocated_memory.tail = nullptr;
    allocated_memory.current_fragment = nullptr;
  }
  // If head is being removed
  else if(memory_to_free->previous_fragment_ptr){
    allocated_memory.head
      = memory_to_free->next_fragment_ptr;
    allocated_memory.head->previous_fragment_ptr = nullptr;
  }
  // If tail is being removed
  else if(memory_to_free->next_fragment_ptr){
    allocated_memory.tail
      = memory_to_free->previous_fragment_ptr;
    allocated_memory.tail->next_fragment_ptr = nullptr;
  }
  // If middle fragment is being removed
  else{
    memory_to_free->previous_fragment_ptr->next_fragment_ptr
      = memory_to_free->next_fragment_ptr;
    memory_to_free->next_fragment_ptr->previous_fragment_ptr
      = memory_to_free->previous_fragment_ptr;
  } 

  // Add recently freed fragment to free list  
  MemoryFragment* recently_freed
    = new MemoryFragment(memory_to_free->first_page,
                         memory_to_free->last_page);

  // If free mem list is empty
  if(!free_memory.head){
    recently_freed->previous_fragment_ptr = nullptr;
    recently_freed->next_fragment_ptr = nullptr;
    free_memory.head = recently_freed;
    free_memory.tail = recently_freed;
    free_memory.current_fragment = recently_freed;
  }
  // If fragment should replace the head of free mem list
  else if(recently_freed->last_page
          < free_memory.head->first_page){
    recently_freed->next_fragment_ptr = free_memory.head;
    recently_freed->previous_fragment_ptr = nullptr;
    free_memory.head = free_memory.current_fragment = recently_freed;
  }
  // If fragment should replace the tail of free mem list
  else if(recently_freed->first_page > free_memory.tail->last_page){
    recently_freed->previous_fragment_ptr = free_memory.tail;
    recently_freed->next_fragment_ptr = nullptr;
    free_memory.tail = free_memory.current_fragment = recently_freed;
  }
  // If fragment should be inserted into middle of alloc mem list
  else{
    free_memory.goToHead();
    while(true){
      free_memory.next();
      if(free_memory.current_fragment->last_page
         < memory_to_free->first_page){
        free_memory.current_fragment->next_fragment_ptr
          = recently_freed;
        recently_freed->previous_fragment_ptr
          = free_memory.current_fragment;
        recently_freed->next_fragment_ptr = free_memory.next();
        recently_freed->next_fragment_ptr->previous_fragment_ptr
          = recently_freed;
        break;
      }
    }
  }
  
  // Fuse recently freed fragment with adjacent free fragments
  
  // If previous fragment is adjacent
  if(!recently_freed->previous_fragment_ptr
     && (recently_freed->previous_fragment_ptr->last_page
         == recently_freed->first_page - 1)){
    MemoryFragment *prev = recently_freed->previous_fragment_ptr;
    recently_freed->first_page = prev->first_page;
    // If previous fragment was head
    if(prev == free_memory.head){
      free_memory.head
        = free_memory.current_fragment = recently_freed;
      recently_freed->previous_fragment_ptr = nullptr;
    }
    // previous fragment was a middle fragment
    else{
      prev->previous_fragment_ptr->next_fragment_ptr = recently_freed;
      recently_freed->previous_fragment_ptr
        = prev->previous_fragment_ptr;
    }
    delete prev;
  }
  // If next fragment is adjacent
  if(!recently_freed->next_fragment_ptr
     && (recently_freed->next_fragment_ptr->first_page
         == recently_freed->last_page + 1)){
    MemoryFragment *nxt = recently_freed->next_fragment_ptr;
    recently_freed->last_page = nxt->last_page;
    // If previous fragment was tail
    if(nxt == free_memory.tail){
      free_memory.tail
        = free_memory.current_fragment = recently_freed;
      recently_freed->next_fragment_ptr = nullptr;
    }
    // next fragment was a middle fragment
    else{
      nxt->next_fragment_ptr->previous_fragment_ptr = recently_freed;
      recently_freed->next_fragment_ptr
        = nxt->next_fragment_ptr;
    }
    delete nxt;
  }

  // delete fragment removed from free mem list
  delete memory_to_free;
  return true;
}

void MemoryManager::printMemory(){
  cout << "legend: F: free, A: allocated" << endl;
  allocated_memory.goToHead();
  int mem_idx = 0;
  while(true){
    if(mem_idx > allocated_memory.current_fragment->first_page
       && mem_idx < allocated_memory.current_fragment->last_page){
      cout << "A,  ";
    }
    else cout << "F,  ";
    if(allocated_memory.current_fragment->next_fragment_ptr) break;
    if(mem_idx == allocated_memory.current_fragment->last_page){
      allocated_memory.next();
    }
    mem_idx++;
  }
  cout << "\b\b\b" << endl << endl;
}

MemoryFragment* 
MemoryManager::findProgramMemoryByName(string program_name){
  allocated_memory.goToHead();
  if(allocated_memory.head == nullptr) return nullptr;
  if(allocated_memory.head->owner_program->name.compare(program_name)){
    return allocated_memory.head;
  }
  while(true){
    MemoryFragment* mem_ptr = allocated_memory.next();
    if(mem_ptr == nullptr) return nullptr;
    if(!mem_ptr->owner_program->name.compare(program_name)){
      return mem_ptr;
    }
  }
}


MemoryFragmentList::MemoryFragmentList(){
  head = nullptr;
  tail = nullptr;
  current_fragment = nullptr;
}

size_t MemoryFragmentList::countContiguousSections(){
  if(head == nullptr) return 0;
  goToHead();
  size_t count = 1;
  while(next() != nullptr){
    if(current_fragment->first_page
       > current_fragment->previous_fragment_ptr->last_page + 1){
      count++;
    }
  }
  goToHead();
  return count;
}

MemoryFragment* MemoryFragmentList::next(){
  current_fragment = current_fragment->next_fragment_ptr;
  return current();
}

MemoryFragment* MemoryFragmentList::previous(){
  current_fragment = current_fragment->previous_fragment_ptr;
  return current();
}

MemoryFragment* MemoryFragmentList::smallest_available_fragment(){
  goToHead();
  vector<size_t> ranges;
  while(true){
    ranges.push_back(current_fragment->range());
    if(next() == nullptr) break;
  }
  int links_from_head = distance(ranges.begin(),
                             min_element(ranges.begin(),
                                         ranges.end()));
  goToHead();
  for(int i = 0; i < links_from_head; i++) next();
  return current_fragment;
}

MemoryFragment* MemoryFragmentList::largest_available_fragment(){
  goToHead();
  vector<size_t> ranges;
  while(true){
    ranges.push_back(current_fragment->range());
    if(next() == nullptr) break;
  }
  int links_from_head = distance(ranges.begin(),
                             max_element(ranges.begin(),
                                         ranges.end()));
  goToHead();
  for(int i = 0; i < links_from_head; i++) next();
  return current_fragment;
}