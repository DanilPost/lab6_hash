// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_

#include <iostream>
#include <vector>
#include <csignal>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <picosha2.h>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/setup.hpp>

const unsigned size_file_MB = 5 * 1024 * 1024;

struct Info_hash
{
  int time;
  std::string hash = "";
  std::string data = "";
};

bool a = true;
void sig_handler_exit(int sig)
{
  sig += 1;
  a = false;
}

void init_log()
{
  boost::log::register_simple_formatter_factory
      <boost::log::trivial::severity_level, char>("Severity");
  const std::string format =
      "[%TimeStamp%][%Severity%][%ThreadID%]: %Message%";

  auto sink_to_file = boost::log::add_file_log(
      boost::log::keywords::file_name = "logs/log_info_%N.log",
      boost::log::keywords::rotation_size = size_file_MB,
      boost::log::keywords::format = format);
  sink_to_file->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::trace);

  auto sink_to_console = boost::log::add_console_log(
      std::cout,
      boost::log::keywords::format = format);
  sink_to_console->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::info);

  boost::log::add_common_attributes();

  srand(time(nullptr));
}

void create_hash(std::vector<Info_hash> &for_out, int &number, std::mutex &mtx)
{
  signal(SIGINT, &sig_handler_exit);
  std::string is_null = "0000";
  bool k = true;
  auto start = std::chrono::high_resolution_clock::now();
  auto finish = std::chrono::high_resolution_clock::now();
  while (a)
  {
    std::string input_rand = std::to_string(std::rand());
    std::string hash_key = picosha2::hash256_hex_string(input_rand);
    if (k)
    {
      start = std::chrono::high_resolution_clock::now();
    }
    if (hash_key.substr(hash_key.size() - is_null.size()) ==
        is_null)
    {
      finish = std::chrono::high_resolution_clock::now();
      k = true;
      int r_time = std::chrono::duration_cast<std::chrono::microseconds>
          (finish - start).count();
      bool m = true;
      while (m)
      {
        if (mtx.try_lock())
        {
          m = false;
          BOOST_LOG_TRIVIAL(info) << std::endl
                                  << "!found value: [" << input_rand
                                  << "] hash is [" << hash_key << "] !"
                                  << std::endl;
          Info_hash info;
          info.time = r_time;
          info.data = input_rand;
          info.hash = hash_key;
          for_out.push_back(info);
          number += 1;
          mtx.unlock();
        }
      }
    } else
    {
      k = false;
      BOOST_LOG_TRIVIAL(trace) << std ::endl
                               << "!found value [" << input_rand
                               << "] hash is [" << hash_key << "] !"
                               << std::endl;
    }
  }
}

void exit_f(std::vector<Info_hash> &for_out, int &number)
{
  std::ofstream fout("JSON.json");
  fout << "{" << std::endl <<  "\"items:\": [" << std::endl << std::endl;
  for (int i = 0; i < number; i++)
  {
    fout << "  {" << std::endl;
    fout << "    \"time\": " <<for_out[i].time << "," << std::endl
              << "    \"hash\": \"" << for_out[i].hash << "\"," << std::endl
              << "    \"data\": \"" <<for_out[i].data << "\"," << std::endl;
    fout << "  }" << std::endl;
  }
  fout << "]," << std::endl << "  \"_meta\": {" << std::endl
       << "    \"count\": " << number << std::endl << "  }" << std::endl << "}";
  fout.close();
}

#endif // INCLUDE_HEADER_HPP_
