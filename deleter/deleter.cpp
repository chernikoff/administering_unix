#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <chrono>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "Queue.hpp"


int main( int argc, char* argv[] )
{
  // Parse CLI-string
  int opt;
  std::string catalog_path;
  while( ( opt = getopt( argc, argv, "p:" ) ) != -1 ) {
    switch( opt ) {
    case 'p':
      catalog_path = std::string( optarg );
      break;
    default:
      std::cout << "Error! Not valid parametrs\n";
      return -1;
      break;
    }
  }

  if ( catalog_path.empty() ) {
    std::cout << "Error! Path not set\n";
    return -1;
  }

  // Add slash for valid full path
  if( catalog_path.back() != '/' ) catalog_path += '/';

  // Open directory
  auto dir_deleter = [] ( DIR *dir ) { if( dir ) closedir( dir ); };
  std::unique_ptr < DIR, decltype( dir_deleter ) > dir( opendir( catalog_path.c_str() ), dir_deleter );
  if( dir ) {
    std::cout << "Start deleting files from " << catalog_path << '\n';
  } else {
    std::cout << "Error! Can't open directory, error: " << strerror( errno ) << '\n';
    return -1;
  }

  // Ask befor deleting
  char answer;
  while ( answer != 'y' && answer != 'n' ) {
    std::cout << "Are you shure for delete files in " << catalog_path << "\n y/n? ";
    std::cin >> answer;
  }

  if ( answer == 'n' ) return 0;

  Queue < std::string > queue;

  std::thread reader( [ &dir, &queue] () {
    std::cout << "Reader\n";
    dirent *entry;
    while( ( entry = readdir( dir.get() ) ) != nullptr ) {
      std::string name( entry->d_name );
      if( name != "."  &&  name != ".." ) {
        std::cout << "Push\n";
        queue.push( entry->d_name );
      }
    }
    std::cout << "End direcrory reading\n";
    queue.close();
  } );

  std::thread deleter( [ &catalog_path, &queue ] () {
    std::cout << "Deleter\n";
    size_t num_deleted = 0;
    while( queue.isOpen() || queue.hasNext() ) {
      if ( queue.hasNext() ) {
        auto file_name = queue.take();
        auto full_path = catalog_path + file_name;
        num_deleted++;
        std::cout << full_path << '\n';
        //remove( full_path.c_str() );
        queue.pop();
      } else {
        queue.wait();
      }
    }
    std::cout << "End deleting files. Deleted " << num_deleted << " files\n";
  } );

  // Wait for finish
  reader.join();
  deleter.join();

  return 0;
}
