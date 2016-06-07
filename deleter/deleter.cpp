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
      std::cout << "Error! Not valid parametrs";
      return -1;
      break;
    }
  }

  // Add slash for valid full path
  if( catalog_path.back() != '/' ) catalog_path += '/';

  // Open directory
  auto dir_deleter = [] ( DIR *dir ) { if( dir ) closedir( dir ); };
  std::unique_ptr < DIR, decltype( dir_deleter ) > dir( opendir( catalog_path.c_str() ), dir_deleter );
  if( dir ) {
    std::cout << "Start deleting files from " << catalog_path << std::endl;
  } else {
    std::cout << "Error! Can't open directory, error: " << strerror( errno ) << std::endl;
    return -1;
  }

  Queue < std::string > queue;

  std::thread reader( [ &dir, &queue] () {
      std::cout << "Reader" << std::endl;
      dirent *entry;
      while( ( entry = readdir( dir.get() ) ) != nullptr ) {
        std::string name( entry->d_name );
        if( name != "."  &&  name != ".." )
        {
          std::cout << "Push" << std::endl;
          queue.push( entry->d_name );
          usleep( 100000 );
        }
      }
      std::cout << "End direcrory reading" << std::endl;
      queue.close();
  } );

  std::thread deleter( [ &catalog_path, &queue ] () {
      std::cout << "Deleter" << std::endl;
      size_t num_deleted = 0;
      while( queue.isOpen() ) {
        auto file_name = queue.pop();
        auto full_path = catalog_path + file_name;
        num_deleted++;
        std::cout << full_path << std::endl;
      }
      std::cout << "End deleting files. Deleted " << num_deleted << " files" << std::endl;

  } );

  // Wait for finish
  reader.join();
  deleter.join();

  return 0;
}
