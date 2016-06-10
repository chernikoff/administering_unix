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

  // Ask befor deleting
  char answer;
  while ( answer != 'y' && answer != 'n' ) {
    std::cout << "Are you shure for delete files in " << catalog_path << "\n y/n? ";
    std::cin >> answer;
  }

  // Open directory
  auto dir_deleter = [] ( DIR *dir ) { if( dir ) closedir( dir ); };
  std::unique_ptr < DIR, decltype( dir_deleter ) > dir( opendir( catalog_path.c_str() ), dir_deleter );
  if( dir ) {
    std::cout << "Start deleting files from " << catalog_path << '\n';
  } else {
    std::cout << "Error! Can't open directory, error: " << strerror( errno ) << '\n';
    return -1;
  }

  if ( answer == 'n' ) return 0;

  Queue < std::string > queue;

  std::thread reader( [ &dir, &queue] () {
    dirent *entry;
    while( ( entry = readdir( dir.get() ) ) != nullptr ) {
      std::string name( entry->d_name );
      if( name != "."  &&  name != ".." ) {
        queue.push( entry->d_name );
      }
    }
    queue.close();
  } );

  auto deleter_func =  [ &catalog_path, &queue ] () {
    size_t num_deleted = 0;
    std::cout << '\n';
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while( queue.isOpen() || queue.hasNext() ) {
      if ( queue.hasNext() ) {
        auto file_name = queue.take();
        auto full_path = catalog_path + file_name;
        remove( full_path.c_str() );
        num_deleted++;
        if ( !( num_deleted % 1000 ) ) {
          std::cout << "\r" << num_deleted / 1000 << "K";
          std::cout.flush();
        }
        queue.pop();
      } else {
        queue.wait();
      }
    }
    std::cout << '\n';
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::time_t total_seconds = std::chrono::duration_cast< std::chrono::seconds >( end - start ).count();
    std::size_t minutes = static_cast< size_t >( total_seconds ) / 60;
    std::size_t seconds = static_cast< size_t >( total_seconds ) % 60;
    std::cout << "Deleted " << num_deleted << " files,\n"
              << "time spent " << minutes << "m:" << seconds << "s,\n"
              << "avg " << num_deleted / total_seconds << " files per second\n";
  };
  std::thread deleter1( deleter_func );
  std::thread deleter2( deleter_func );

  // Wait for finish
  reader.join();
  deleter1.join();
  deleter2.join();

  return 0;
}
