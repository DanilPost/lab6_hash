#include <header.hpp>
#include <mutex>

int main(int argc, char *argv[]) {
  std::mutex mut;
  std::vector<struct Info_hash> for_out_json;
  int number_json = 0;

  init_log();
  unsigned thread_count;
  if (argc >= 2) {thread_count = boost::lexical_cast<unsigned>(argv[1]);}
  else thread_count = std::thread::hardware_concurrency();

  BOOST_LOG_TRIVIAL(trace) << "THREADS COUNT" << thread_count;

  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (unsigned i = 0; i < thread_count; i++) {
    threads.emplace_back(create_hash,
                         std::ref(for_out_json),std::ref(number_json),
                         std::ref(mut));
  }
  for (std::thread &thr : threads) {
    thr.join();
  }
  exit_f(std::ref(for_out_json),std::ref(number_json));
  return 0;
}